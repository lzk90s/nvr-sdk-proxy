#pragma once

#include <string>
#include <vector>

#include "3rdsdk/stub/sdk_stub.h"

#include "3rdsdk/stub/empty_sdk_stub.h"
#include "3rdsdk/vendor/dahua/sdk_stub_impl.h"
#include "3rdsdk/vendor/hikvision/sdk_stub_impl.h"
#include "3rdsdk/vendor/dahuanvr/sdk_stub_impl.h"
#include "3rdsdk/vendor/hikvisionnvr/sdk_stub_impl.h"

namespace sdkproxy {
namespace sdk {

class SdkStubFactory {
public:
    static std::shared_ptr<SdkStub> Create(const std::string &vendor) {
        if ("dahuanvr" == vendor) {
            return std::make_shared<dahuanvr::SdkStubImpl>();
        } else if ("hikvisionnvr" == vendor) {
            return std::make_shared<hikvisionnvr::SdkStubImpl>();
        } else {
            return std::make_shared<EmptySdkStub>();
        }
    }

    static std::vector<std::string> GetVendors() {
        std::string strVendors = std::getenv("SDK_VENDORS") ? std::getenv("SDK_VENDORS") : "dahuanvr,hikvisionnvr";
        std::vector<std::string> vendors;
        const char *d = ",";
        char *p       = strtok((char *)strVendors.c_str(), d);
        while (p) {
            vendors.push_back(p);
            p = strtok(nullptr, d);
        }
        return vendors;
    }
};

} // namespace sdk
} // namespace sdkproxy