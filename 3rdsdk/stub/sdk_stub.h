#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <functional>

#include "common/helper/logger.h"
#include "3rdsdk/stub/po_type.h"
#include "3rdsdk/stub/cache.h"

namespace sdkproxy {
namespace sdk {

class CallbackClosure {
public:
    void *thisClass;
};

class SdkStub {
public:
    using OnDownloadData = std::function<void(intptr_t id, const uint8_t *buffer, int32_t bufferLen)>;
    using OnRealPlayData = std::function<void(intptr_t id, const uint8_t *buffer, int32_t bufferLen)>;
    using OnAnalyzeData = std::function<void(intptr_t id, int type, const std::string &jsonData, const uint8_t *imgBuffer,
                          int32_t imgBufferLen, void *userData)>;

public:
    SdkStub(const std::string &vendor, const std::string &description, int port)
        : vendor_(vendor), description_(description), port_(port) {
        LOG_INFO("\033[0;33mInitialize sdk for {}, {}\033[0m", vendor, description);
    }

    virtual ~SdkStub() {
    }

    std::string GetVendor() const {
        return vendor_;
    }

    std::string GetDescription() const {
        return description_;
    }

    int GetPort() const {
        return port_;
    }

    std::string ChannelIp2Id(const std::string &channelIp) {
        std::string id = "";
        std::vector<Device> devices;
        int ret = QueryDeviceCache(devices);
        if (ret == 0) {
            for (auto &d : devices) {
                if (d.ip == channelIp) {
                    id = d.id;
                }
            }
        }
        LOG_INFO("ChannelIp2Id: {} -> {}", channelIp, id);
        return id;
    }

    int32_t QueryDeviceCache(std::vector<Device> &devices) {
        deviceCache_.setQueryFunc([&](const std::string & k, Cache<std::vector<Device>>::Value & v) {
            return QueryDevice(v.value);
        });

        Cache<std::vector<Device>>::Value v;
        int ret = deviceCache_.Query("device", v);
        if (0 != ret) {
            return ret;
        }

        devices = v.value;

        return 0;
    }

    virtual int32_t Login(const std::string &ip, const std::string &user, const std::string &password) {
        return -1;
    };

    virtual int32_t Logout() {
        return -1;
    };

    virtual int32_t QueryDevice(std::vector<Device> &devices) {
        return -1;
    }

    virtual int32_t StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) {
        return -1;
    }

    virtual int32_t StopRealStream(intptr_t &jobId) {
        return -1;
    }

    virtual int32_t QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                                std::vector<RecordInfo> &records) {
        return -1;
    }

    virtual int32_t DownloadRecordByTime(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                                         OnDownloadData onData, intptr_t &jobId) {
        return -1;
    }

    virtual int32_t StopDownloadRecord(intptr_t &jobId) {
        return -1;
    }

    virtual int32_t StartEventAnalyze(const std::string &devId, OnAnalyzeData onData, void *userData, intptr_t &jobId) {
        return -1;
    }

    virtual int32_t StopEventAnalyze(intptr_t &jobId) {
        return -1;
    }

    virtual int32_t GetFtp(const std::string &devId, FtpInfo &ftpInfo) {
        return -1;
    }

    virtual int32_t SetFtp(const std::string &devId, const FtpInfo &ftpInfo) {
        return -1;
    }

    virtual int32_t QueryVisitorsFlowRateHistory(const std::string &devId, int32_t granularity,
            const TimePoint &startTime, const TimePoint &endTime, std::vector<VisitorsFlowRateHistory> &histories) {
        return -1;
    }

    virtual int32_t SnapPicture(const std::string &devId, uint8_t *imgBuf, uint32_t &imgBufSize) {
        return -1;
    }

protected:
    std::string ip_;

private:
    std::string vendor_;
    std::string description_;
    int port_;
    Cache<std::vector<Device>> deviceCache_;
};

}
}