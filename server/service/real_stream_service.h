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
#include "server/util/pipe_guard.h"

namespace sdkproxy {

class RealStreamServiceImpl final : public RealStreamService {
public:
    ~RealStreamServiceImpl() {}

    virtual void Start(::google::protobuf::RpcController *controller, const ::sdkproxy::HttpRequest *request, ::sdkproxy::HttpResponse *response,
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
            LOG_ERROR("Invalid arguments, devId {}", devId);
            parser.SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return;
        }

        std::shared_ptr<PipeGuard> dataPipe(new PipeGuard());
        if (0 != dataPipe->Init()) {
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Open pipe failed");
            return;
        }

        intptr_t jobId = 0;
        int ret        = sdk->StartRealStream(
            devId,
            [=](intptr_t id, const uint8_t *buffer, int32_t bufferLen) {
                if (nullptr != buffer) {
                    IoUtil::writen(dataPipe->GetWriteFd(), (const char *)buffer, bufferLen);
                } else {
                    //通知写完毕
                    dataPipe->NotifyWriteCompleted();
                }
            },
            jobId);

        if (0 != ret) {
            LOG_ERROR("Failed to start real stream");
            parser.SetResponseError(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to start real stream");
            return;
        }

        butil::intrusive_ptr<brpc::ProgressiveAttachment> pa(cntl->CreateProgressiveAttachment());

        // start thread for transport
        std::thread t([=]() {
            // non block I/O
            fcntl(dataPipe->GetReadFd(), F_SETFL, O_NONBLOCK);
            fd_set rfdset;

            while (true) {
                FD_ZERO(&rfdset);
                FD_SET(dataPipe->GetReadFd(), &rfdset);

                struct timeval tv;
                tv.tv_sec  = 30;
                tv.tv_usec = 0;
                int r      = select(dataPipe->GetReadFd() + 1, &rfdset, nullptr, nullptr, &tv);
                if (-1 == r && errno == EINTR) {
                    continue;
                } else if (-1 == r) {
                    // error
                    LOG_ERROR("Wait for data error");
                    break;
                } else if (0 == r) {
                    // timeout, no sdk data
                    LOG_ERROR("Wait for data timeout");
                    break;
                } else {
                    // read data from pipe
                    char buf[4096];
                    int len = read(dataPipe->GetReadFd(), buf, sizeof(buf));
                    if ((len < 0 && errno != EAGAIN) || (len == 0) || (len > 0 && ProgressiveAttachmentUtil::writen(pa.get(), buf, len) < 0)) {
                        break;
                    }
                }
            }

            //通知读取完毕，当客户端主动关闭时，需要通知pipe的写端停止写
            dataPipe->NotifyReadCompleted();

            // finished
            intptr_t jid = jobId;
            sdk->StopRealStream(jid);
            LOG_INFO("The real stream is completed, dev {}", devId);
        });
        t.detach();
    }
};

} // namespace sdkproxy