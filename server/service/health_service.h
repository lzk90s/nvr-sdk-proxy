#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>

#include "common/helper/logger.h"

#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_manager.h"

#include "server/rpc/service.pb.h"
#include "server/util/io_util.h"
#include "server/util/progressive_attachment_util.h"
#include "server/service/http_request_parser.h"

namespace sdkproxy {

class HealthServiceImpl : public HealthService {
public:
    void Health(::google::protobuf::RpcController *controller,
                const ::sdkproxy::HttpRequest *request,
                ::sdkproxy::HttpResponse *response,
                ::google::protobuf::Closure *done) override {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);

        cntl->http_response().set_content_type("application/json");
        cntl->response_attachment().append("{\"status\": \"UP\"}");
    }
};

}