#pragma once

#include <map>
#include <cstdint>

#include <arpa/inet.h>

#include "common/helper/singleton.h"
#include "common/helper/logger.h"
#include "common/helper/threadpool.h"

#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_stub_factory.h"
#include "3rdsdk/stub/metadata_store.h"

namespace sdkproxy {
namespace sdk {

class NetworkException : public std::exception {
public:
    const char* what()const throw() override {
        return "Network exception";
    }
};

class LoginException : public std::exception {
public:
    const char* what()const throw() override {
        return "Login failed";
    }
};

class SdkManager {
public:
    SdkManager() : probeWorker_(4), metaStore_("/mnt/sdk_meta") {}

    std::shared_ptr<SdkStub> TryLoginAndGet(const std::string &ip, const std::string &user, const std::string &password) {
        if (ip.empty() || user.empty() || password.empty()) {
            LOG_ERROR("Invalid param, ip={}, user={}, password={}", ip, user, password);
            return nullptr;
        }

        std::shared_ptr<SdkStub> stub = nullptr;
        std::string key = buildSdkKey(ip);

        // 1. find from cache
        {
            std::unique_lock<std::mutex> lck(mutex_);
            if (sdks_.find(key) != sdks_.end()) {
                return sdks_[key];
            }
        }

        // 2. probe with vendor
        std::string vendor;
        try {
            if (metaStore_.Get(key, vendor)) {
                LOG_INFO("Get vendor from meta storage succeed, address {}, vendor {}", ip, vendor);
                stub = probe(vendor, ip, user, password);
            }
        } catch (...) {
            // continue probe
        }

        // 3. probe without vendor
        try {
            if (nullptr == stub) {
                stub = probe("", ip, user, password);
            }
        } catch (std::exception &e) {
            LOG_ERROR("Probe address {} failed with user {} and password {}, {}", ip, user, password, e.what());
            throw;
        }

        // 4. store the data
        {
            std::unique_lock<std::mutex> lck(mutex_);
            sdks_[key] = stub;
            if (vendor != stub->GetVendor()) {
                metaStore_.Put(key, stub->GetVendor());
            }
        }

        LOG_INFO("Probe address {} succeed, {}", ip, stub->GetDescription());

        return stub;
    }

    ~SdkManager() {
        std::unique_lock<std::mutex> lck(mutex_);
        for (auto &p : sdks_) {
            p.second->Logout();
        }
        sdks_.clear();
    }

private:
    std::string buildSdkKey(const std::string &ip) {
        return ip;
    }

    std::shared_ptr<SdkStub> probe(const std::string &vendor, const std::string &ip, const  std::string &user,
                                   const std::string &password) throw (NetworkException, LoginException) {
        std::shared_ptr<SdkStub> stub = nullptr;
        std::vector<std::future<int>> results;
        auto probeVendors = vendor.empty() ? SdkStubFactory::GetVendors() : std::vector<std::string> { vendor };

        //start asynchronous probe
        for (auto &v : probeVendors) {
            results.push_back(probeWorker_.commit([this, v, ip, user, password, &stub]() {
                std::shared_ptr<SdkStub> s = SdkStubFactory::Create(v);
                //quick check the address
                if (!isHostReachable(ip, s->GetPort())) {
                    return -1;
                }
                //try login
                if (0 != s->Login(ip, user, password)) {
                    return -2;
                }
                stub = s;
                return 0;
            }));
        }

        //wait for probe result
        int loginErrorTimes = 0;
        for (auto &r : results) {
            int status = r.get();
            if (status == 0) {
                break;
            } else if (status == -2) {
                loginErrorTimes++;
            }
        }

        //check result
        if (nullptr == stub) {
            if (loginErrorTimes > 0) {
                throw LoginException();
            } else {
                throw NetworkException();
            }
        } else {
            return stub;
        }
    }

    bool isHostReachable(const std::string &ip, int port) {
        int fd;
        struct sockaddr_in addr;
        struct timeval timeo = { 4, 0 };
        socklen_t len = sizeof(timeo);
        bool reachable = true;

        fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);

        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            reachable = false;
//             if (errno == EINPROGRESS) {
//                 //timeout? return true
//                 reachable = true;
//             }
        }
        close(fd);
        return reachable;
    }

private:
    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<SdkStub>> sdks_;
    std::threadpool probeWorker_;
    MetaDataStore metaStore_;
};

SdkManager& SDK_MNG() {
    return Singleton<SdkManager>::getInstance();
}

}
}