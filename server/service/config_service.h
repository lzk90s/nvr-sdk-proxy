#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <sstream>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>

#include <brpc/progressive_attachment.h>

#include "common/helper/logger.h"

#include "3rdsdk/stub/po_type_serialization.h"
#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_manager.h"

#include "server/rpc/service.pb.h"
#include "server/service/http_request_parser.h"

namespace sdkproxy {
using json = nlohmann::json;

class ConfigServiceImpl : public ConfigService {
public:
    void GetFtp(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
                ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string devId = sdk->ChannelIp2Id(parser.GetChannelIp());

        if (devId.empty()) {
            LOG_ERROR("Invalid arguments");
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        sdk::FtpInfo ftpInfo;
        int ret = sdk->GetFtp(devId, ftpInfo);
        if (0 != ret) {
            LOG_INFO("Failed to get ftp");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to get ftp");
            return;
        }

        json j = ftpInfo;
        cntl->http_response().set_content_type("application/json");
        cntl->response_attachment().append(j.dump());
    }

    void SetFtp(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
                ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string devId    = sdk->ChannelIp2Id(parser.GetChannelIp());
        std::string jsonBody = cntl->request_attachment().to_string();

        if (devId.empty() || jsonBody.empty()) {
            LOG_ERROR("Invalid arguments");
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        auto j               = json::parse(jsonBody.c_str());
        sdk::FtpInfo ftpInfo = j.get<sdk::FtpInfo>();
        int ret              = sdk->SetFtp(devId, ftpInfo);
        if (0 != ret) {
            LOG_INFO("Failed to config ftp, ip {}", parser.GetIp());
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to config ftp");
            return;
        }

        LOG_INFO("Succeed to config ftp, {}", j.dump());
    }
};
} // namespace sdkproxy