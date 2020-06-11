#include <memory>
#include <string>

#include <gflags/gflags.h>
//#include <butil/logging.h>
#include <brpc/server.h>

#include "server/service/real_stream_service.h"
#include "server/service/vod_service.h"
#include "server/service/device_query_service.h"
#include "server/service/event_analyze_service.h"
#include "server/service/health_service.h"
#include "server/service/config_service.h"
#include "server/service/visitors_flowrate_service.h"

//----------------server params----------------
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 7011, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");


int main(int argc, char *argv[]) {
    GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);

    // create rpc server
    brpc::Server server;

    // Instance of your service.
    sdkproxy::RealStreamServiceImpl realStreamServiceImpl;
    sdkproxy::VodServiceImpl vodServiceImpl;
    sdkproxy::DeviceQueryServiceImpl deviceQueryServiceImpl;
    sdkproxy::EventAnalyzeServiceImpl eventAnalyzeServiceImpl;
    sdkproxy::HealthServiceImpl healthServiceImpl;
    sdkproxy::ConfigServiceImpl configServiceImpl;
    sdkproxy::VisitorsFlowRateServiceImpl visitorsFlowRateServiceImpl;

    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&realStreamServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&vodServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&deviceQueryServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&eventAnalyzeServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&healthServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&configServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0 ||
            server.AddService(&visitorsFlowRateServiceImpl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        std::cout << "Fail to add service" << std::endl;
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
        std::cout << "Fail to start EchoServer" << std::endl;
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    std::cout << "Quit" << std::endl;

    return 0;
}
