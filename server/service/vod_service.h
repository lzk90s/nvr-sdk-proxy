#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <sstream>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>

#include <brpc/progressive_attachment.h>
#include <brpc/errno.pb.h>

#include "common/helper/logger.h"
#include "common/helper/counttimer.h"

#include "3rdsdk/stub/po_type_serialization.h"
#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_manager.h"

#include "server/rpc/service.pb.h"
#include "server/util/io_util.h"
#include "server/util/progressive_attachment_util.h"
#include "server/service/http_request_parser.h"

namespace sdkproxy {

using json = nlohmann::json;

class VodServiceImpl : public VodService {
public:

    void Query(::google::protobuf::RpcController *controller,
               const ::sdkproxy::HttpRequest *request,
               ::sdkproxy::HttpResponse *response,
               ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        CountTimer timer("VodServiceImpl::Query");

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string devId = sdk->ChannelIp2Id(parser.GetChannelIp());
        std::string startTime = parser.GetQueryByKey("startTime");
        std::string endTime = parser.GetQueryByKey("endTime");

        if (devId.empty() || startTime.empty() || endTime.empty()) {
            LOG_ERROR("Invalid arguments, devId {}, startTime {}, endTime {}", devId, startTime, endTime);
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        std::vector<sdk::RecordInfo> records;
        int ret = sdk->QueryRecord(devId, sdk::TimePoint().FromString(startTime), sdk::TimePoint().FromString(endTime),
                                   records);
        if (0 != ret) {
            LOG_INFO("Failed to query record");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to query record");
            return;
        }

        json j = records;
        cntl->http_response().set_content_type("application/json");
        cntl->response_attachment().append(j.dump());
    }

    void DownloadByTime(::google::protobuf::RpcController *controller,
                        const ::sdkproxy::HttpRequest *request,
                        ::sdkproxy::HttpResponse *response,
                        ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        CountTimer timer("VodServiceImpl::DownloadByTime");

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string devId = sdk->ChannelIp2Id(parser.GetChannelIp());
        std::string startTime = parser.GetQueryByKey("startTime");
        std::string endTime = parser.GetQueryByKey("endTime");

        if (devId.empty() || startTime.empty() || endTime.empty()) {
            LOG_ERROR("Invalid arguments, devId {}, startTime {}, endTime {}", devId, startTime, endTime);
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        int fd[2];
        if (0 != pipe(fd)) {
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Open pipe failed");
            return;
        }

        LOG_INFO("Start to download record, dev {}, from {} to {}", devId, startTime, endTime);

        intptr_t jobId = 0;
        int ret = sdk->DownloadRecordByTime(devId, sdk::TimePoint().FromString(startTime), sdk::TimePoint().FromString(endTime),
        [ = ](intptr_t id, const uint8_t *buffer, int32_t bufferLen) {
            if (nullptr != buffer) {
                IoUtil::writen(fd[1], (const char *)buffer, bufferLen);
            } else {
                //关闭写端，会触发read返回0，可以正常退出
                close(fd[1]);
            }
        }, jobId);

        if (0 != ret) {
            LOG_ERROR("Failed to download file");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to download file");
            return;
        }

        butil::intrusive_ptr<brpc::ProgressiveAttachment> pa(cntl->CreateProgressiveAttachment());

        //start thread for transport
        std::thread t([ = ]() {
            //non block I/O
            fcntl(fd[0], F_SETFL, O_NONBLOCK);
            fd_set rfdset;

            while (true) {
                FD_ZERO(&rfdset);
                FD_SET(fd[0], &rfdset);

                struct timeval tv;
                tv.tv_sec = 30;
                tv.tv_usec = 0;
                int r = select(fd[0] + 1, &rfdset, nullptr, nullptr, &tv);
                if (-1 == r && errno == EINTR) {
                    continue;
                } else if (-1 == r) {
                    //error
                    LOG_ERROR("Wait for data error");
                    break;
                } else if (0 == r) {
                    //timeout, no sdk data
                    LOG_ERROR("Wait for data timeout");
                    break;
                }  else {
                    //read data from pipe
                    char buf[4096];
                    int len = read(fd[0], buf, sizeof(buf));
                    if ((len < 0 && errno != EAGAIN) ||
                            (len == 0) ||
                            (len > 0 && ProgressiveAttachmentUtil::writen(pa.get(), buf, len) < 0)) {
                        break;
                    }
                }
            }

            //finished
            close(fd[1]);
            close(fd[0]);
            intptr_t jid = jobId;
            sdk->StopDownloadRecord(jid);
            LOG_INFO("The download is completed, dev {}, from {} to {}", devId, startTime, endTime);
        });
        t.detach();
    }
};

}