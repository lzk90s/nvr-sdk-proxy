#pragma once

#include "3rdsdk/stub/sdk_stub.h"

namespace sdkproxy {
namespace sdk {

class EmptySdkStub final : public SdkStub {
public:
    EmptySdkStub() : SdkStub("dummy", "Invalid sdk", 0) {}
};
}
}