#pragma once

#include <mutex>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "3rdsdk/stub/sdk_stub.h"

namespace sdkproxy {
namespace sdk {
namespace hikvisionnvr {

class SdkStubImpl final : public SdkStub {
public:
    SdkStubImpl();

    ~SdkStubImpl();

    int32_t Login(const std::string &ip, const std::string &user, const std::string &password) override;

    int32_t Logout() override;

    int32_t QueryDevice(std::vector<Device> &devices) override;

    int32_t StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) override;

    int32_t StopRealStream(intptr_t &jobId) override;

    int32_t QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                        std::vector<RecordInfo> &records) override;

    int32_t DownloadRecordByTime(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                                 OnDownloadData onData, intptr_t &jobId) override;

    int32_t StopDownloadRecord(intptr_t &jobId) override;

    int32_t StartEventAnalyze(const std::string &devId, OnAnalyzeData onData, void *userData, intptr_t &jobId) override;

    int32_t StopEventAnalyze(intptr_t &jobId) override;

    int32_t GetFtp(const std::string &devId, FtpInfo &ftpInfo) override;

    int32_t SetFtp(const std::string &devId, const FtpInfo &ftpInfo) override;

public:
    /**-------------------------------- callback --------------------------------**/
    bool AlarmMsgCallback(int64_t cmd, char *buffer, int64_t bufferLen, intptr_t userData);

private:
    std::string  getChannelName(int channelIdx);

private:
    std::mutex mutex_;
    intptr_t handle_;
    int32_t channelNum_;
    int32_t startChan_;    //模拟通道开始号
};

}
}
}