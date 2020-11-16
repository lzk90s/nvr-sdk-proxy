#pragma once

#include <mutex>
#include <string>
#include <memory>
#include <map>

#include "3rdsdk/stub/sdk_stub.h"

namespace sdkproxy {
namespace sdk {
namespace dahua {

class SdkStubImpl final : public SdkStub {
public:
    SdkStubImpl();

    ~SdkStubImpl();

    int32_t Login(const std::string &ip, const std::string &user, const std::string &password) override;

    int32_t Logout() override;

    int32_t QueryDevice(std::vector<Device> &devices) override;

    int32_t StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) override;

    int32_t StopRealStream(intptr_t &jobId) override;

    int32_t QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime, std::vector<RecordInfo> &records) override;

    int32_t DownloadRecordByTime(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime, OnDownloadData onData,
                                 intptr_t &jobId) override;

    int32_t StopDownloadRecord(intptr_t &jobId) override;

private:
    int32_t createNodeDep(const std::string &depId, int nType, std::function<void(Device &c)> onData);

private:
    std::mutex mutex_;
    int32_t handle_;
};

} // namespace dahua
} // namespace sdk
} // namespace sdkproxy