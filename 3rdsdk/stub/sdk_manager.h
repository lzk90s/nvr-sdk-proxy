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
    const char *what() const throw() override { return "Network exception"; }
};

class LoginException : public std::exception {
public:
    const char *what() const throw() override { return "Login failed"; }
};

class SdkManager {
public:
    SdkManager() : probeWorker_(4), metaStore_("/mnt/sdk_meta") {}

    std::shared_ptr<SdkStub> TryLoginAndGet(const std::string &ip, const std::string &user, const std::string &password) {
        if (ip.empty() || user.empty() || password.empty()) {
            LOG_ERROR("Invalid param, ip={}, user={}, password={}", ip, user, password);
            return nullptr;
        }

        std::string key = buildSdkKey(ip);

        // find from cache
        {
            std::unique_lock<std::mutex> lck(mutex_);
            if (sdks_.find(key) != sdks_.end()) {
                return sdks_[key];
            }
        }

        try {
            auto stub = tryProbe(ip, user, password);

            // save to cache
            std::unique_lock<std::mutex> lck(mutex_);
            sdks_[key] = stub;

            LOG_INFO("Probe address {} succeed, {}", ip, stub->GetDescription());

            return stub;
        } catch (std::exception &e) {
            LOG_ERROR("Probe address {} failed, {}", ip, e.what());
            throw;
        }
    }

    ~SdkManager() {
        std::unique_lock<std::mutex> lck(mutex_);
        for (auto &p : sdks_) {
            p.second->Logout();
        }
        sdks_.clear();
    }

private:
    std::shared_ptr<SdkStub> tryProbe(const std::string &ip, const std::string &user, const std::string &password) {
        std::shared_ptr<SdkStub> stub = nullptr;
        std::string key               = buildSdkKey(ip);

        // 1. get vendor from storage
        std::string vendor;
        if (metaStore_.Get(key, vendor)) {
            LOG_INFO("Found vendor {} for address {} in storage", vendor, ip);
        }

        // 2. probe with specific vendor
        if (!vendor.empty()) {
            try {
                stub = probe(std::vector<std::string>{vendor}, ip, user, password);
            } catch (LoginException &e) {
                // login failed? return
                throw;
            } catch (...) {
                // other exception, continue other probe progress
            }
        }

        // 3. probe with all vendor
        if (nullptr == stub) {
            auto allVendors = SdkStubFactory::GetVendors();
            std::vector<std::string> vendorList;
            std::copy_if(allVendors.begin(), allVendors.end(), std::back_inserter(vendorList), [vendor](const std::string &v) { return vendor != v; });

            try {
                stub = probe(vendorList, ip, user, password);
            } catch (std::exception &e) {
                throw;
            }
        }

        // 4. store the data
        if (nullptr != stub && vendor != stub->GetVendor()) {
            metaStore_.Put(key, stub->GetVendor());
        }

        return stub;
    }

    std::shared_ptr<SdkStub> probe(const std::vector<std::string> &vendorList, const std::string &ip, const std::string &user,
                                   const std::string &password) throw(NetworkException, LoginException) {
        if (vendorList.empty()) {
            return nullptr;
        }

        std::shared_ptr<SdkStub> stub = nullptr;
        std::vector<std::future<int>> results;

        // start asynchronous probe
        for (auto &v : vendorList) {
            results.push_back(probeWorker_.commit([this, v, ip, user, password, &stub]() {
                std::shared_ptr<SdkStub> s = SdkStubFactory::Create(v);
                // quick check the address
                if (!isHostReachable(ip, s->GetPort())) {
                    return -1;
                }
                // try login
                if (0 != s->Login(ip, user, password)) {
                    return -2;
                }
                stub = s;
                return 0;
            }));
        }

        // wait for probe result
        int loginErrorTimes = 0;
        for (auto &r : results) {
            int status = r.get();
            if (status == 0) {
                break;
            } else if (status == -2) {
                loginErrorTimes++;
            }
        }

        // check result
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
        struct timeval timeo = {4, 0};
        socklen_t len        = sizeof(timeo);
        bool reachable       = true;

        fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port        = htons(port);

        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            reachable = false;
            //             if (errno == EINPROGRESS) {
            //                 //timeout? return true
            //                 reachable = true;
            //             }
        }
        close(fd);
        return reachable;
    }

    std::string buildSdkKey(const std::string &ip) { return ip; }

private:
    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<SdkStub>> sdks_;
    std::threadpool probeWorker_;
    MetaDataStore metaStore_;
};

SdkManager &SDK_MNG() {
    return Singleton<SdkManager>::getInstance();
}

} // namespace sdk
} // namespace sdkproxy