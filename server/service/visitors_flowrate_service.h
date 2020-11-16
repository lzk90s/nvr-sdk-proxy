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

class VisitorsFlowRateServiceImpl : public VisitorsFlowRateService {
public:
    void QueryHistory(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
                      ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        HttpRequestParser parser(cntl);

        auto sdk = parser.GetSdkStubByRequest();
        if (nullptr == sdk) {
            return;
        }

        std::string devId       = sdk->ChannelIp2Id(parser.GetChannelIp());
        std::string granularity = parser.GetQueryByKey("granularity");
        std::string startTime   = parser.GetQueryByKey("startTime");
        std::string endTime     = parser.GetQueryByKey("endTime");

        if (devId.empty() || granularity.empty() || startTime.empty() || endTime.empty()) {
            LOG_ERROR("Invalid arguments");
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        std::vector<sdk::VisitorsFlowRateHistory> histories;
        int ret = sdk->QueryVisitorsFlowRateHistory(devId, atoi(granularity.c_str()), sdk::TimePoint().FromString(startTime),
                                                    sdk::TimePoint().FromString(endTime), histories);
        if (0 != ret) {
            LOG_INFO("Failed to query visitors flow rate history");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to query visitors flow rate history");
            return;
        }

        json j = histories;
        cntl->http_response().set_content_type("application/json");
        cntl->response_attachment().append(j.dump());
    }
};

} // namespace sdkproxy