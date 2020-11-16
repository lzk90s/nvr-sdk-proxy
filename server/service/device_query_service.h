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

class DeviceQueryServiceImpl : public DeviceQueryService {
    void Discovery(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
                   ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }
    }

    void Query(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
               ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::vector<sdk::Device> devices;
        int ret = sdk->QueryDevice(devices);
        if (0 != ret) {
            LOG_INFO("Failed to query device");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to query device");
            return;
        }

        std::string pretty = parser.GetQueryByKey("pretty");
        if (pretty == "json") {
            cntl->http_response().set_content_type("application/json");
            cntl->response_attachment().append(buildJsonResponseMsg(devices));
        } else {
            cntl->http_response().set_content_type("text/html");
            cntl->response_attachment().append(buildHtmlResponseMsg(devices));
        }
    }

private:
    std::string buildJsonResponseMsg(std::vector<sdk::Device> &devices) {
        json j = devices;
        return j.dump();
    }

    std::string buildHtmlResponseMsg(std::vector<sdk::Device> &devices) {
        std::stringstream html;

        html << "<!DOCTYPE html>";
        html << "<html xmlns=\"http://www.w3.org/1999/xhtml\">";
        html << "<head>";
        html << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
        html << "<title>设备信息</title>";
        html << "</head>";
        html << "<body>";
        html << "<div id=\"tabdiv\">";
        html << "<table id=\"tabinfo\" border=\"1\" cellpadding=\"10\" style=\"margin:auto\">";
        html << "<tr>";
        html << "<th>通道ID</th>";
        html << "<th>通道名</th>";
        html << "<th>通道IP</th>";
        html << "</tr>";
        for (auto &d : devices) {
            html << "<tr>";
            html << "<td>" << d.id << "</td>";
            html << "<td>" << d.name << "</td>";
            html << "<td>" << d.ip << "</td>";
            html << "</tr>";
        }
        html << "</table>";
        html << "</div>";
        html << "</body>";
        html << "</html>";

        return html.str();
    }
};

} // namespace sdkproxy