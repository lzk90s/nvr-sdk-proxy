#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <sstream>
#include <chrono>
#include <list>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>

#include "brpc/channel.h"

#include "server/util/httplib.h"
#include "common/helper/logger.h"
#include "common/helper/timer.h"

#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_manager.h"

#include "server/rpc/service.pb.h"
#include "server/service/http_request_parser.h"
#include "server/util/io_util.h"
#include "server/util/progressive_attachment_util.h"

namespace sdkproxy {

using json = nlohmann::json;
using namespace sdkproxy::sdk;

class EventAnalyzeServiceImpl : public EventAnalyzeService {
public:
    EventAnalyzeServiceImpl() {
        char *eventUploadHost = std::getenv("EVENT_UPLOAD_HOST");
        if (nullptr == eventUploadHost) {
            eventUploadHost = (char *)"capture-structuration";
        }
        char *eventUploadPort = std::getenv("EVENT_UPLOAD_PORT");
        if (nullptr == eventUploadPort) {
            eventUploadPort = (char *)"12500";
        }

        LOG_INFO("Event upload destination is {}:{}", eventUploadHost, eventUploadPort);

        client_.reset(new httplib::Client(eventUploadHost, atoi(eventUploadPort)));
    }

    void Reset(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
               ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        std::unique_lock<std::mutex> lck(mutex_);

        LOG_INFO("Reset event analyze, total {}", jobCache_.size());

        for (auto &p : jobCache_) {
            p.second->sdkStub->StopEventAnalyze(p.second->jobId);
        }

        jobCache_.clear();
    }

    void Query(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
               ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        std::unique_lock<std::mutex> lck(mutex_);

        std::vector<std::string> devList;
        for (auto &p : jobCache_) {
            devList.push_back(p.second->devCode);
        }

        json j = devList;
        cntl->http_response().set_content_type("application/json");
        cntl->response_attachment().append(j.dump());
    }

    void Start(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
               ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string ip        = parser.GetIp();
        std::string channelIp = parser.GetChannelIp();
        std::string devId     = sdk->ChannelIp2Id(channelIp);
        std::string devCode   = parser.GetQueryByKey("devCode");
        std::string key       = buildKey(ip, devId);

        if (devId.empty() || devCode.empty()) {
            LOG_ERROR("Invalid arguments, devId {}, devCode {}", devId, devCode);
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        {
            std::unique_lock<std::mutex> lck(mutex_);
            if (jobCache_.find(key) != jobCache_.end()) {
                LOG_INFO("Task exist for {}-{}", ip, devId);
                jobCache_[key]->time = std::chrono::system_clock::now();
                return;
            }
        }

        std::shared_ptr<JobInfo> jobInfo(new JobInfo());
        jobInfo->sdkStub   = sdk;
        jobInfo->time      = std::chrono::system_clock::now();
        jobInfo->devCode   = devCode;
        jobInfo->channelIp = channelIp;

        intptr_t jobId = 0;
        int32_t ret    = sdk->StartEventAnalyze(
            devId,
            [this](intptr_t id, int type, const std::string &jsonData, const uint8_t *imgBuffer, int32_t imgBufferLen, void *userData) {
                uploadEvent(((JobInfo *)userData)->devCode, ((JobInfo *)userData)->channelIp, type, jsonData, imgBuffer, imgBufferLen);
            },
            (void *)jobInfo.get(), jobId);

        if (0 != ret) {
            LOG_ERROR("Failed to start event analyze");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to start event analyze");
            return;
        }

        // 保存记录
        {
            std::unique_lock<std::mutex> lck(mutex_);
            jobInfo->jobId = jobId;
            jobCache_[key] = jobInfo;
        }
    }

    void Stop(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
              ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        std::string devCode = parser.GetQueryByKey("devCode");
        if (devCode.empty()) {
            LOG_ERROR("Invalid arguments, devCode {}", devCode);
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        std::unique_lock<std::mutex> lck(mutex_);

        std::vector<std::string> devList;
        for (auto &p : jobCache_) {
            if (devCode == p.second->devCode) {
                LOG_INFO("Stop event analyze job for device {}", devCode);
                p.second->sdkStub->StopEventAnalyze(p.second->jobId);
                break;
            }
        }
    }

private:
    void uploadEvent(const std::string &devCode, const std::string &realIp, int alarmType, const std::string &alarmData, const uint8_t *imgBuffer,
                     int32_t bufferLen) {
        std::string path = buildImagePath(devCode);
        auto t           = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), "%F %T");
        std::string nowDateTime = ss.str();

        httplib::MultipartFormDataItems items = {
            {"path", path, "", ""},
            {"realIp", realIp, "", ""},
            {"dateTime", nowDateTime, "", ""},
            {"alarmType", std::to_string(alarmType), "", ""},
            {"alarmData", alarmData, "", "application/json"},
            {"image", std::string((char *)imgBuffer, bufferLen), devCode + ".jpg", "image/jpeg"},
        };

        LOG_INFO("Upload alarm event, devCode = {}, realIp = {}, alarmType = {}, alarmData = {}", devCode, realIp, alarmType, alarmData);

        client_->Post("/v3/upload", items);
    }

    std::string buildKey(const std::string &ip, const std::string &devId) { return ip + "_" + devId; }

    std::string buildImagePath(const std::string &devCode) {
        std::stringstream stm;
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        stm << "/" << devCode << "/" << std::put_time(std::localtime(&t), "%Y/%m/%d/%Y_%m_%d_%H_%M_%S") << "_0.jpg";
        return stm.str();
    }

private:
    typedef struct tagJobInfo {
        intptr_t jobId;
        std::chrono::time_point<std::chrono::system_clock> time;
        std::shared_ptr<sdkproxy::sdk::SdkStub> sdkStub;
        std::string devCode;
        std::string channelIp;
    } JobInfo;

    std::map<std::string, std::shared_ptr<JobInfo>> jobCache_;
    CppTime::Timer checkTimer_;
    std::mutex mutex_;
    std::string httpCallbackUrl_;
    std::shared_ptr<httplib::Client> client_;
};
} // namespace sdkproxy