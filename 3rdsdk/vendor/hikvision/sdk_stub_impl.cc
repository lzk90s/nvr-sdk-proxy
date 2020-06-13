#include "3rdsdk/vendor/hikvision/sdk_stub_impl.h"

namespace sdkproxy {
namespace sdk {
namespace hikvision {
using json = nlohmann::json;

#define SUFFIX(msg) std::string("[{}] ").append(msg)
#define STUB_LLOG_DEBUG(fmt, ...) LLOG_DEBUG(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_INFO(fmt, ...) LLOG_INFO(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_WARN(fmt, ...) LLOG_WARN(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)
#define STUB_LLOG_ERROR(fmt, ...) LLOG_ERROR(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)


SdkStubImpl::SdkStubImpl() : SdkStub("hikvision", "Hikvision platform sdk", 81) {
}

}
}
}