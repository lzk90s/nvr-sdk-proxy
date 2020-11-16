#pragma once

#include <mutex>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "3rdsdk/stub/sdk_stub.h"

namespace sdkproxy {
namespace sdk {
namespace dahuanvr {

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

    int32_t StartEventAnalyze(const std::string &devId, OnAnalyzeData onData, void *userData, intptr_t &jobId) override;

    int32_t StopEventAnalyze(intptr_t &jobId) override;

    int32_t GetFtp(const std::string &devId, FtpInfo &ftpInfo) override;

    int32_t SetFtp(const std::string &devId, const FtpInfo &ftpInfo) override;

    int32_t QueryVisitorsFlowRateHistory(const std::string &devId, int32_t granularity, const TimePoint &startTime, const TimePoint &endTime,
                                         std::vector<VisitorsFlowRateHistory> &histories) override;

    int32_t SnapPicture(const std::string &devId, uint8_t *imgBuf, uint32_t &imgBufSize) override;

public:
    /**-------------------------------- callback --------------------------------**/
    bool AnalyzerDataCallBack(intptr_t handle, int64_t alarmType, void *alarmInfo, uint8_t *buffer, int64_t bufSize, intptr_t userData);

    void VideoStatSumCallBack(uintptr_t handle, void *buffer, uint64_t bufSize, uintptr_t userData);

    bool MessageCallBack(uint64_t cmd, char *buffer, uint64_t bufSize, uint64_t eventId, uintptr_t userData);

    void TimeDownLoadPosCallback(intptr_t handle, int64_t totalSize, int64_t downLoadSize, uintptr_t userData);

private:
    std::mutex mutex_;
    intptr_t handle_;
    int32_t channelNum_;
};

} // namespace dahuanvr
} // namespace sdk
} // namespace sdkproxy