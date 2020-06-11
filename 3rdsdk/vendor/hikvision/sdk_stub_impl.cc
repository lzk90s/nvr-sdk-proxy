#include "3rdsdk/vendor/hikvision/sdk_stub_impl.h"

namespace sdkproxy {
namespace sdk {
namespace hikvision {
using json = nlohmann::json;

SdkStubImpl::SdkStubImpl() : SdkStub("hikvision", "Hikvision platform sdk", 81) {
}

}
}
}