#include <iomanip>
#include <ctime>
#include <chrono>

#include "3rdsdk/stub/po_type_serialization.h"
#include "3rdsdk/vendor/dahuanvr/sdk_stub_impl.h"

#include "common/helper/logger.h"
#include "common/helper/singleton.h"

#include "dhnetsdk.h"


namespace sdkproxy {
namespace sdk {
namespace dahuanvr {
using json = nlohmann::json;

static Logger logger("dahua_nvr");
const static int TIMEOUT = 30000;

#define SUFFIX(msg) std::string("[{}] ").append(msg)
#define STUB_LLOG_DEBUG(fmt, ...) LLOG_DEBUG(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_INFO(fmt, ...) LLOG_INFO(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_WARN(fmt, ...) LLOG_WARN(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)
#define STUB_LLOG_ERROR(fmt, ...) LLOG_ERROR(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)

const inline DWORD lastError() {
    return CLIENT_GetLastError() & 0x7fffffff;
}

#define CHECK(method_call, msg)                                                         \
do {                                                                                    \
    BOOL __result = FALSE;                                                              \
    if ((__result= (method_call)) != TRUE) {                                            \
        DWORD err = lastError();                                                        \
        STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", err, msg); return err;      \
    }                                                                                   \
} while (0)

#define CHECK_EX(method_call, msg, defer)                                               \
do {                                                                                    \
    BOOL __result = FALSE;                                                              \
    if ((__result= (method_call)) != TRUE) {                                            \
        DWORD err = lastError();                                                        \
        STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", err, msg); return err;      \
        (defer)();                                                                      \
    }                                                                                   \
} while (0)


class SdkHolder {
public:
    SdkHolder() {
        if (FALSE == CLIENT_Init(NULL, 0)) {
            throw std::runtime_error("Init sdk error");
        }

        DWORD sdkVersion = CLIENT_GetSDKVersion();
        LLOG_INFO(logger, "Succeed to initialize dahua nvr sdk, the sdk version is {}", sdkVersion);

        // 设置断线重连
        CLIENT_SetAutoReconnect(SdkHolder::pfHaveReConnect, 0);

        // 设置响应超时时间5s，尝试连接次数3次
        int nWaitTime = 5000;
        int nTryTime = 3;
        CLIENT_SetConnectTime(nWaitTime, nTryTime);

        NET_PARAM param;
        param.nWaittime = TIMEOUT;
        param.nGetDevInfoTime = TIMEOUT;
        param.nSearchRecordTime = TIMEOUT;
        CLIENT_SetNetworkParam(&param);
    }

    ~SdkHolder() {
        CLIENT_Cleanup();
    }

    static void CALLBACK pfHaveReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser) {
        LLOG_INFO(logger, "Reconnect device {}:{}", pchDVRIP, nDVRPort);
    }
};

static void toTimePoint(TimePoint &tp, const NET_TIME &tm) {
    tp.year = tm.dwYear;
    tp.month = tm.dwMonth;
    tp.day = tm.dwDay;
    tp.hour = tm.dwHour;
    tp.minute = tm.dwMinute;
    tp.second = tm.dwSecond;
}

static void fromTimePoint(const TimePoint &tp, NET_TIME &tm) {
    tm.dwYear = tp.year;
    tm.dwMonth = tp.month;
    tm.dwDay = tp.day;
    tm.dwHour = tp.hour;
    tm.dwMinute = tp.minute;
    tm.dwSecond = tp.second;
}

static std::string getCurrentDateTime(const std::string &format = "%F %T") {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), format.c_str());
    return ss.str();
}

static std::string netTimeToString(const NET_TIME_EX &tm) {
    char t[128] = { 0 };
    snprintf(t, sizeof(t) - 1, "%04d-%02d-%02d %02d:%02d:%02d", tm.dwYear, tm.dwMonth, tm.dwDay, tm.dwHour, tm.dwMinute,
             tm.dwSecond);
    return std::string(t);
}

static std::string netTimeToString(const NET_TIME &tm) {
    char t[128] = { 0 };
    snprintf(t, sizeof(t) - 1, "%04d-%02d-%02d %02d:%02d:%02d", tm.dwYear, tm.dwMonth, tm.dwDay, tm.dwHour, tm.dwMinute,
             tm.dwSecond);
    return std::string(t);
}

static std::vector<int32_t> parseBoundingBox(const DH_RECT &box, int32_t width, int32_t height) {
    DH_RECT dstRect = { 0 };

    dstRect.left = ceil((double)(width  * box.left) / 8192);
    dstRect.right = ceil((double)(width  * box.right) / 8192);
    dstRect.bottom = ceil((double)(height * box.bottom) / 8192);
    dstRect.top = ceil((double)(height * box.top) / 8192);

    int x = dstRect.left;
    int y = dstRect.top;
    int w = dstRect.right - dstRect.left;
    int h = dstRect.bottom - dstRect.top;

    return std::vector<int32_t> {x, y, w, h};
}

SdkStubImpl::SdkStubImpl() : SdkStub("dahuanvr", "Dahua net sdk", 37777) {
    //init singleton
    channelNum_ = 0;
    Singleton<SdkHolder>::getInstance();
}

SdkStubImpl::~SdkStubImpl() {
    //退出登录
    Logout();
}

int32_t SdkStubImpl::Login(const std::string &ip, const std::string &user, const std::string &password) {
    ip_ = ip;

    int ret = 0;
    NET_DEVICEINFO_Ex devInfo = { 0 };
    handle_ = CLIENT_LoginEx2(ip.c_str(), GetPort(), user.c_str(), password.c_str(), EM_LOGIN_SPEC_CAP_TCP, NULL, &devInfo,
                              &ret);
    if (0 == handle_) {
        STUB_LLOG_ERROR("Failed to login {}, ret {}", ip, ret);
        return ret;
    }

    STUB_LLOG_INFO("Succeed to login {}, handle {}", ip, handle_);
    STUB_LLOG_INFO("SerialNumber={}, DVRType={}, ChannelNum={}", devInfo.sSerialNumber, devInfo.nDVRType, devInfo.nChanNum);

    channelNum_ = devInfo.nChanNum;

    return 0;
}

int32_t SdkStubImpl::Logout() {
    CLIENT_Logout(handle_);
    return 0;
}

int32_t SdkStubImpl::QueryDevice(std::vector<Device> &devices) {
    if (channelNum_ <= 0) {
        return 0;
    }

    STUB_LLOG_INFO("Start query device");

    std::unique_ptr<DH_IN_MATRIX_GET_CAMERAS> pInParam(new DH_IN_MATRIX_GET_CAMERAS());
    pInParam->dwSize = sizeof(DH_IN_MATRIX_GET_CAMERAS);

    std::unique_ptr<DH_OUT_MATRIX_GET_CAMERAS> pOutParam(new DH_OUT_MATRIX_GET_CAMERAS());
    pOutParam->dwSize = sizeof(DH_OUT_MATRIX_GET_CAMERAS);
    pOutParam->nMaxCameraCount = channelNum_;
    std::unique_ptr<DH_MATRIX_CAMERA_INFO[]> cameras(new DH_MATRIX_CAMERA_INFO[channelNum_]);

    if (pOutParam->nMaxCameraCount > 0) {
        pOutParam->pstuCameras = cameras.get();
        memset(pOutParam->pstuCameras, 0, sizeof(DH_MATRIX_CAMERA_INFO) * channelNum_);
    }

    for (int i = 0; i < channelNum_; i++) {
        pOutParam->pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
        pOutParam->pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
    }

    if (TRUE == CLIENT_MatrixGetCameras(handle_, pInParam.get(), pOutParam.get(), TIMEOUT)) {
        for (int i = 0; i < pOutParam->nMaxCameraCount; i++) {
            std::unique_ptr<NET_IN_GET_CAMERA_STATEINFO> pCameraStateInBuf(new NET_IN_GET_CAMERA_STATEINFO());
            pCameraStateInBuf->dwSize = sizeof(NET_IN_GET_CAMERA_STATEINFO);
            pCameraStateInBuf->bGetAllFlag = FALSE;
            pCameraStateInBuf->nValidNum = 1;
            pCameraStateInBuf->nChannels[0] = i;

            std::unique_ptr<NET_OUT_GET_CAMERA_STATEINFO> pCameraStateOutBuf(new NET_OUT_GET_CAMERA_STATEINFO());
            pCameraStateOutBuf->dwSize = sizeof(NET_OUT_GET_CAMERA_STATEINFO);
            pCameraStateOutBuf->nMaxNum = 1;
            NET_CAMERA_STATE_INFO cameraStateInfo = { 0 };
            pCameraStateOutBuf->pCameraStateInfo = &cameraStateInfo;
            memset(pCameraStateOutBuf->pCameraStateInfo, 0, sizeof(NET_CAMERA_STATE_INFO));

            CHECK(CLIENT_QueryDevInfo(handle_, NET_QUERY_GET_CAMERA_STATE, pCameraStateInBuf.get(), pCameraStateOutBuf.get(),
                                      nullptr, TIMEOUT),
                  "Query device info");
            if (EM_CAMERA_STATE_TYPE_CONNECTED != pCameraStateOutBuf->pCameraStateInfo->emConnectionState) {
                LLOG_WARN(logger, "The channel {} has not connected, ignore it", i);
                continue;
            }

            std::unique_ptr<NET_ENCODE_CHANNELTITLE_INFO> pChlInfo(new NET_ENCODE_CHANNELTITLE_INFO());
            memset(pChlInfo.get(), 0, sizeof(NET_ENCODE_CHANNELTITLE_INFO));
            pChlInfo->dwSize = sizeof(NET_ENCODE_CHANNELTITLE_INFO);
            CHECK(CLIENT_GetConfig(handle_, NET_EM_CFG_ENCODE_CHANNELTITLE, pOutParam->pstuCameras[i].nUniqueChannel,
                                   pChlInfo.get(),
                                   sizeof(NET_ENCODE_CHANNELTITLE_INFO)), "CLIENT_GetConfig NET_EM_CFG_ENCODE_CHANNELTITLE");

            Device dev;
            dev.id = std::to_string(pOutParam->pstuCameras[i].nUniqueChannel);
            dev.name = pChlInfo->szChannelName;
            dev.ip = pOutParam->pstuCameras[i].stuRemoteDevice.szIp;
            STUB_LLOG_INFO("Found channel, id={}, name={}, ip={}", dev.id, dev.name, dev.ip);

            devices.push_back(dev);
        }
    } else {
        for (int i = 0; i < channelNum_; i++) {
            Device dev;
            dev.id = std::to_string(i);
            dev.name = "Channel_" + dev.id;
            dev.ip = ip_;
            STUB_LLOG_INFO("Found channel, id={}, name={}, ip={}", dev.id, dev.name, dev.ip);

            devices.push_back(dev);
        }
    }

    STUB_LLOG_INFO("Succeed to query device, num {}", devices.size());

    return 0;
}

typedef struct tagRealPlayInfo : public CallbackClosure {
    LLONG playHandle;
    SdkStub::OnRealPlayData fn;

    tagRealPlayInfo() {
        playHandle = -1;
        fn = nullptr;
    }
} RealPlayContext;

void CALLBACK previewDataCallback(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param,
                                  LDWORD dwUser) {
    RealPlayContext *context = (RealPlayContext *)dwUser;
    if (nullptr == context || nullptr == context->fn) {
        return;
    }
    context->fn(lRealHandle, pBuffer, dwBufSize);
}

int32_t SdkStubImpl::StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context(new RealPlayContext());

    LLONG playHandle = CLIENT_StartRealPlay(handle_, atoi(devId.c_str()), 0, DH_RType_Realplay, previewDataCallback,
                                            nullptr, (LDWORD)context.get());
    if (playHandle < 0) {
        int ret = lastError();
        STUB_LLOG_ERROR("NET_DVR_RealPlay_V40 error, ret = {}", ret);
        return ret;
    }

    context->playHandle = playHandle;
    context->thisClass = this;
    jobId = (intptr_t)context.get();
    context.release();

    return 0;
}

int32_t SdkStubImpl::StopRealStream(intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context((RealPlayContext *)jobId); //auto delete

    if (context->playHandle >= 0) {
        CLIENT_StopRealPlay(context->playHandle);
    }

    LLOG_INFO(logger, "Stop real play succeed, playId {}", context->playHandle);

    return 0;
}

int32_t SdkStubImpl::QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                                 std::vector<RecordInfo> &records) {
    const uint32_t recordMaxCount = 1000;
    int32_t recordCount = 0;
    std::unique_ptr<NET_RECORDFILE_INFO[]> recordInfos(new NET_RECORDFILE_INFO[recordMaxCount]);

    LLOG_INFO(logger, "Start query record for device {}, {}-{}", devId, startTime.ToString(), endTime.ToString());

    NET_TIME tmStart, tmEnd;
    fromTimePoint(startTime, tmStart);
    fromTimePoint(endTime, tmEnd);
    CHECK(CLIENT_QueryRecordFile(handle_, atoi(devId.c_str()), 0, &tmStart, &tmEnd, nullptr, recordInfos.get(),
                                 recordMaxCount * sizeof(NET_RECORDFILE_INFO), &recordCount, TIMEOUT), "Query record");
    for (int32_t i = 0; i < recordCount; i++) {
        sdkproxy::sdk::RecordInfo r;
        r.fileName = recordInfos[i].filename;
        r.fileSize = recordInfos[i].size;
        toTimePoint(r.startTime, recordInfos[i].starttime);
        toTimePoint(r.endTime, recordInfos[i].endtime);
        records.push_back(r);
    }

    STUB_LLOG_INFO("Succeed to query record, record count {}", recordCount);

    return 0;
}

//playback closure
typedef struct tagPlaybackInfo : public CallbackClosure {
    intptr_t downloadId;
    SdkStub::OnDownloadData fn;

    tagPlaybackInfo() {
        downloadId = -1;
        fn = nullptr;
        thisClass = nullptr;
    }
} PlaybackContext;

static void timeDownLoadPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index,
                            NET_RECORDFILE_INFO recordfileinfo,
                            LDWORD dwUser) {
    SdkStubImpl *thisClass = (SdkStubImpl *)((CallbackClosure *)dwUser)->thisClass;
    thisClass->TimeDownLoadPosCallback(lPlayHandle, dwTotalSize, dwDownLoadSize, index, (void *)&recordfileinfo, dwUser);
}

void SdkStubImpl::TimeDownLoadPosCallback(intptr_t handle, int64_t totalSize, int64_t downLoadSize, int32_t index,
        void *recordfileinfo,
        uintptr_t userData) {
    PlaybackContext *context = (PlaybackContext *)userData;
    if (nullptr == context) {
        return;
    }

    if (((int32_t)downLoadSize) <= 0) {
        STUB_LLOG_INFO("Record download 100%, downloadId {}", handle);
        context->fn(handle, nullptr, -1);
    }
}

static int downloadDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser) {
    PlaybackContext *context = (PlaybackContext *)dwUser;
    if (nullptr == context) {
        return 0;
    }

    switch (dwDataType) {
    case 0:
        context->fn(lRealHandle, pBuffer, dwBufSize);
        break;
    default:
        break;
    }
    return 0;
}

int32_t SdkStubImpl::DownloadRecordByTime(const std::string &devId, const TimePoint &startTime,
        const TimePoint &endTime, OnDownloadData onData, intptr_t &jobId) {
    //new context
    std::unique_ptr<PlaybackContext> context(new PlaybackContext());
    context->fn = onData;
    context->thisClass = this;

    // download by time
    NET_TIME tmStart, tmEnd;
    fromTimePoint(startTime, tmStart);
    fromTimePoint(endTime, tmEnd);
    context->downloadId = CLIENT_DownloadByTimeEx(handle_, atoi(devId.c_str()), 0,
                          &tmStart, &tmEnd, nullptr,
                          timeDownLoadPos, (LDWORD)context.get(),
                          downloadDataCallBack, (LDWORD)context.get());
    if (0 == context->downloadId) {
        DWORD err = lastError();
        STUB_LLOG_ERROR("CLIENT_DownloadByTimeEx error {}", err);
        return err;
    }

    STUB_LLOG_INFO("Downloading file for dev {} between {} and {}, downloadId {}", devId, startTime.ToString(),
                   endTime.ToString(), context->downloadId);

    jobId = (intptr_t)context.get();
    context.release();  // 释放控制权

    return 0;
}

int32_t SdkStubImpl::StopDownloadRecord(intptr_t &jobId) {
    std::unique_ptr<PlaybackContext> context((PlaybackContext *)jobId); // auto delete
    if (nullptr != context) {
        STUB_LLOG_INFO("Stop download file, downloadId {}", context->downloadId);
        CHECK(CLIENT_StopDownload(context->downloadId), "CLIENT_StopDownload");
        jobId = 0;
    }
    return 0;
}

//playback closure
typedef struct tagEventAnalyzeContext : public CallbackClosure {
    intptr_t analyzeId;
    intptr_t videoStatHandle;
    intptr_t startServiceHandle;
    SdkStub::OnAnalyzeData fn;
    void *userData;

    tagEventAnalyzeContext() {
        analyzeId = -1;
        videoStatHandle = -1;
        startServiceHandle = -1;
        fn = nullptr;
        thisClass = nullptr;
    }
} EventAnalyzeContext;

inline static std::string vehicleSizeToString(const int v) {
    std::string strType = "Unknown";
    if (v & 0x1) {
        strType = "小型车";
    }
    if (v & 0x2) {
        strType = "中型车";
    }
    if (v & 0x4) {
        strType = "大型车";
    }
    if (v & 0x8) {
        strType = "微型车";
    }
    if (v & 0x16) {
        strType = "长车";
    }
    return strType;
}


typedef std::map<std::string, int32_t> TypeMapping;

static const TypeMapping plateColorMapping = {
    {"Unknown", 999},
    {"Yellow", 1},
    {"Blue", 2},
    {"Black", 3},
    {"White", 4},
};

static const TypeMapping platTypeMapping = {
    {"Unknown", 99},
    {"Normal", 2},
    {"YellowPlate", 1},
    {"DoubleYellow", 1},
    {"Police", 23},
    {"Armed", 32},
    {"Military", 32},
    {"DoubleMilitary", 32},
    {"SpecialAdministrativeRegion", 26},
    {"Trainning", 16},
    {"Personal", 99},
    {"Agri", 99},
    {"Embassy", 3},
    {"Moto", 99},
    {"Tractor", 99},
    {"OfficialCar", 99},
    {"PersonalCar", 99},
    {"WarCar", 32},
    {"Other", 99},
    {"CivilAviation", 99},
    {"Black", 99},
    {"PureNewEnergyMicroCar", 99},
    {"MixedNewEnergyMicroCar", 99},
    {"PureNewEnergyLargeCar", 99},
    {"MixedNewEnergyLargeCar", 99}
};

static const TypeMapping vehicleColorMapping = {
    {"Unknown", 999},
    {"Yellow", 8},
    {"Blue", 2},
    {"Black", 1},
    {"White", 6},
    {"Red", 7},
    {"Gray", 12},
    {"Green", 4},
};

static const TypeMapping vehicleTypeMapping = {
    {"Unknown", 0},
    {"Motor", 3},
    {"Non-Motor", 3},
    {"Bus", 6},
    {"Bicycle", 2},
    {"Motorcycle", 3},
    {"UnlicensedMotor", 3},
    {"LargeCar", 8},
    {"MicroCar", 4},
    {"EmbassyCar", 4},
    {"MarginalCar", 4},
    {"AreaoutCar", 4},
    {"ForeignCar", 4},
    {"DualTriWheelMotorcycle", 5},
    {"LightMotorcycle", 3},
    {"EmbassyMotorcycle", 3},
    {"MarginalMotorcycle", 3},
    {"AreaoutMotorcycle", 3},
    {"ForeignMotorcycle", 3},
    {"FarmTransmitCar", 4},
    {"Tractor", 0 },
    {"Trailer", 8},
    {"CoachCar", 4},
    {"CoachMotorcycle", 3},
    {"TrialCar", 4},
    {"TrialMotorcycle", 3},
    {"TemporaryEntryCar", 4},
    {"TemporaryEntryMotorcycle", 3},
    {"TemporarySteerCar", 4},
    {"PassengerCar", 4},
    {"LargeTruck", 8},
    {"MidTruck", 8},
    {"SaloonCar", 4},
    {"Microbus", 7},
    {"MicroTruck", 8},
    {"Tricycle", 5},
    {"Passerby", 1}
};

static int32_t mappingConvert(const TypeMapping &m, const std::string &key)  {
    int32_t v = 0;
    if (m.find(key) != m.end()) {
        v = m.at(key);
    } else {
        v = m.at("Unknown");
    }
    return v;
}

static BOOL analyzerDataCallBack(LLONG lAnalyzerHandle, DWORD dwAlarmType, void *pAlarmInfo, BYTE *pBuffer,
                                 DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved) {
    SdkStubImpl *thisClass = (SdkStubImpl *)((CallbackClosure *)dwUser)->thisClass;
    return thisClass->AnalyzerDataCallBack(lAnalyzerHandle, dwAlarmType, pAlarmInfo, pBuffer, dwBufSize, dwUser);
}

bool SdkStubImpl::AnalyzerDataCallBack(intptr_t handle, int64_t alarmType, void *alarmInfo, uint8_t *buffer,
                                       int64_t bufSize, intptr_t userData) {
    EventAnalyzeContext *context = (EventAnalyzeContext *)userData;
    if (nullptr == context || nullptr == context->fn || nullptr == alarmInfo || nullptr == buffer) {
        return false;
    }

    //排除移动侦测告警
    if (alarmType == EVENT_ALARM_MOTIONDETECT) {
        return true;
    }

    STUB_LLOG_INFO("[analyzerDataCallBack] Received alarm message, type {}", alarmType);

    switch (alarmType) {
    //违停
    case EVENT_IVS_TRAFFIC_PARKING: {
        DEV_EVENT_TRAFFIC_PARKING_INFO &data = *((DEV_EVENT_TRAFFIC_PARKING_INFO *)alarmInfo);

        DH_RECT &rectBoundingBox = data.stuVehicle.BoundingBox;
        long nWidth = data.stuResolution.snWidth;
        long nHeight = data.stuResolution.snHight;
        if ((nWidth <= 0) || (nHeight <= 0)) {
            return false;
        }

        IllegalParkingEvent info;
        info.rect = parseBoundingBox(rectBoundingBox, nWidth, nHeight);
        info.id = std::to_string(data.nEventID);
        info.dateTime = netTimeToString(data.UTC);
        info.plateNumber = data.stTrafficCar.szPlateNumber;
        info.plateColor = std::to_string(mappingConvert(plateColorMapping, data.stTrafficCar.szPlateColor));
        info.plateType = std::to_string(mappingConvert(platTypeMapping, data.stTrafficCar.szPlateType));
        info.vehicleColor = std::to_string(mappingConvert(vehicleColorMapping, data.stTrafficCar.szVehicleColor));
        info.vehicleType = std::to_string(mappingConvert(vehicleTypeMapping, data.stuVehicle.szObjectSubType));
        info.vehicleSize = vehicleSizeToString(data.stTrafficCar.nVehicleSize);
        info.lane = std::to_string(data.stTrafficCar.nLane);
        info.imageGroupId = std::to_string(data.stuFileInfo.nGroupId);
        info.imageIndex = data.stuFileInfo.bIndex;
        info.imageCount = data.stuFileInfo.bCount;
        info.violationCode = data.stTrafficCar.szViolationCode;
        info.violationDesc = data.stTrafficCar.szViolationDesc;

        //没有车牌号的，直接跳过
        if (info.plateNumber == "") {
            break;
        }

        context->fn(handle, AlarmEventType::ILLEGAL_PARKING, ((json)info).dump(), buffer, bufSize,
                    context->userData);
        break;
    }
    //人数量统计
    case EVENT_IVS_NUMBERSTAT: {
        DEV_EVENT_NUMBERSTAT_INFO &data = *((DEV_EVENT_NUMBERSTAT_INFO *)alarmInfo);

        PersonNumEvent info;
        info.ruleName = data.szName;

        break;
    }
    //
    case EVENT_IVS_MAN_NUM_DETECTION: {
        DEV_EVENT_MANNUM_DETECTION_INFO &data = *((DEV_EVENT_MANNUM_DETECTION_INFO *)alarmInfo);
        break;
    }
    case EVENT_IVS_STAYDETECTION: {
        DEV_EVENT_STAY_INFO &data = *((DEV_EVENT_STAY_INFO *)alarmInfo);
        break;
    }
    //交通抓拍
    case EVENT_IVS_TRAFFICJUNCTION: {
        DEV_EVENT_TRAFFICJUNCTION_INFO &data = *((DEV_EVENT_TRAFFICJUNCTION_INFO *)alarmInfo);

        DH_RECT &rectBoundingBox = data.stuVehicle.BoundingBox;
        long nWidth = data.stuResolution.snWidth;
        long nHeight = data.stuResolution.snHight;
        if ((nWidth <= 0) || (nHeight <= 0)) {
            return false;
        }

        VehicleCaptureEvent info;
        info.rect = parseBoundingBox(rectBoundingBox, nWidth, nHeight);
        info.plateNumber = data.stTrafficCar.szPlateNumber;
        info.plateType = std::to_string(mappingConvert(platTypeMapping, data.stTrafficCar.szPlateType));
        info.plateColor = std::to_string(mappingConvert(plateColorMapping, data.stTrafficCar.szPlateColor));
        info.vehicleColor = std::to_string(mappingConvert(vehicleColorMapping, data.stTrafficCar.szVehicleColor));
        info.vehicleType = std::to_string(mappingConvert(vehicleTypeMapping, data.stuVehicle.szObjectSubType));

        //没有车牌号的，直接跳过
        if (info.plateNumber == "") {
            break;
        }

        context->fn(handle, AlarmEventType::VEHICLE_CAPTURE, ((json)info).dump(), buffer, bufSize,
                    context->userData);

        break;
    }
    case EVENT_IVS_TRAFFICGATE: {
        DEV_EVENT_TRAFFICGATE_INFO &data = *((DEV_EVENT_TRAFFICGATE_INFO *)alarmInfo);

        DH_RECT &rectBoundingBox = data.stuVehicle.BoundingBox;
        long nWidth = data.stuResolution.snWidth;
        long nHeight = data.stuResolution.snHight;
        if ((nWidth <= 0) || (nHeight <= 0)) {
            return false;
        }

//         VehicleCaptureEvent info;
//         info.rect = parseBoundingBox(rectBoundingBox, nWidth, nHeight);
//         info.plateNumber = data.stTrafficCar.szPlateNumber;
//         info.plateType = std::to_string(mappingConvert(platTypeMapping, data.stuVehicle.ty));
//         info.plateColor = std::to_string(mappingConvert(plateColorMapping, data.stTrafficCar.szPlateColor));
//         info.vehicleColor = std::to_string(mappingConvert(vehicleColorMapping, data.stTrafficCar.szVehicleColor));
//         info.vehicleType = std::to_string(mappingConvert(vehicleTypeMapping, data.stuVehicle.szObjectSubType));

//         //没有车牌号的，直接跳过
//         if (info.plateNumber == "") {
//             break;
//         }
//
//         context->fn(lAnalyzerHandle, AlarmEventType::VEHICLE_CAPTURE, ((json)info).dump(), pBuffer, dwBufSize, context->userData);
        break;
    }
    case EVENT_IVS_TRAFFIC_TOLLGATE: {
        break;
    }
    case EVENT_IVS_ACCESS_CTL: {
        DEV_EVENT_ACCESS_CTL_INFO &data = *((DEV_EVENT_ACCESS_CTL_INFO *)alarmInfo);
        AcsEvent info;
        info.dateTime = netTimeToString(data.UTC);
        info.type = (int32_t)data.emEventType;
        info.openMethod = data.emOpenMethod;
        break;
    }
    default: {
        break;
    }
    }

    return true;
}

static void videoStatSumCallBack(LLONG lAttachHandle, NET_VIDEOSTAT_SUMMARY *pBuf, DWORD dwBufLen, LDWORD dwUser) {
    SdkStubImpl *thisClass = (SdkStubImpl *)((CallbackClosure *)dwUser)->thisClass;
    thisClass->VideoStatSumCallBack(lAttachHandle, (void *)pBuf, dwBufLen, dwUser);
}

void SdkStubImpl::VideoStatSumCallBack(uintptr_t handle, void *buffer, uint64_t bufSize, uintptr_t userData) {
    EventAnalyzeContext *context = (EventAnalyzeContext *)userData;
    if (nullptr == context || nullptr == context->fn || nullptr == buffer) {
        return;
    }

    NET_VIDEOSTAT_SUMMARY *pBuf = (NET_VIDEOSTAT_SUMMARY *)buffer;

    STUB_LLOG_INFO("[videoStatSumCallBack] Received video stat summary event");

    VisitorsFlowRateEvent info;
    info.dateTime = netTimeToString(pBuf->stuTime);
    info.ruleName = pBuf->szRuleName;
    info.inCount.total = pBuf->stuEnteredSubtotal.nTotal;
    info.inCount.hour = pBuf->stuEnteredSubtotal.nHour;
    info.inCount.today = pBuf->stuEnteredSubtotal.nToday;
    info.inCount.osd = pBuf->stuEnteredSubtotal.nOSD;

    info.outCount.total = pBuf->stuExitedSubtotal.nTotal;
    info.outCount.hour = pBuf->stuExitedSubtotal.nHour;
    info.outCount.today = pBuf->stuExitedSubtotal.nToday;
    info.outCount.osd = pBuf->stuExitedSubtotal.nOSD;

    context->fn(handle, AlarmEventType::VISITORS_FLOWRATE, ((json)info).dump(), nullptr, 0, context->userData);
}


BOOL messageCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort,
                     BOOL bAlarmAckFlag, LONG nEventID, LDWORD dwUser) {
    SdkStubImpl *thisClass = (SdkStubImpl *)((CallbackClosure *)dwUser)->thisClass;
    return thisClass->MessageCallBack(lCommand, pBuf, dwBufLen, nEventID, dwUser);
}

bool SdkStubImpl::MessageCallBack(uint64_t cmd, char *buffer, uint64_t bufSize,  uint64_t eventId, uintptr_t userData) {
    EventAnalyzeContext *context = (EventAnalyzeContext *)userData;
    if (nullptr == context || nullptr == context->fn || nullptr == buffer) {
        return true;
    }

    //排除new_file事件
    if (cmd == DH_ALARM_NEW_FILE) {
        return true;
    }

    STUB_LLOG_INFO("[messageCallBack] Received message {}", cmd);

    switch (cmd) {
//     // 门禁刷卡事件
//     case DH_ALARM_ACCESS_CTL_EVENT: {
//         ALARM_ACCESS_CTL_EVENT_INFO *pInfo = (ALARM_ACCESS_CTL_EVENT_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ACCESS_CTL_BREAK_IN: {
//         ALARM_ACCESS_CTL_BREAK_IN_INFO *pInfo = (ALARM_ACCESS_CTL_BREAK_IN_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ACCESS_CTL_DURESS: {
//         ALARM_ACCESS_CTL_DURESS_INFO *pInfo = (ALARM_ACCESS_CTL_DURESS_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ACCESS_CTL_NOT_CLOSE: {
//         ALARM_ACCESS_CTL_NOT_CLOSE_INFO *pInfo = (ALARM_ACCESS_CTL_NOT_CLOSE_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ACCESS_CTL_REPEAT_ENTER: {
//         ALARM_ACCESS_CTL_REPEAT_ENTER_INFO *pInfo = (ALARM_ACCESS_CTL_REPEAT_ENTER_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ACCESS_CTL_STATUS: {
//         ALARM_ACCESS_CTL_STATUS_INFO *pInfo = (ALARM_ACCESS_CTL_STATUS_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_CHASSISINTRUDED: {
//         ALARM_CHASSISINTRUDED_INFO *pInfo = (ALARM_CHASSISINTRUDED_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_OPENDOORGROUP: {
//         ALARM_OPEN_DOOR_GROUP_INFO *pInfo = (ALARM_OPEN_DOOR_GROUP_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_FINGER_PRINT: {
//         ALARM_CAPTURE_FINGER_PRINT_INFO *pInfo = (ALARM_CAPTURE_FINGER_PRINT_INFO *)buffer;
//         break;
//     }
//     case DH_ALARM_ALARM_EX2: {
//         ALARM_ALARM_INFO_EX2 *pInfo = (ALARM_ALARM_INFO_EX2 *)buffer;
//         break;
//     }
//     case DH_ALARM_TRAFFICSTROBESTATE: {
//         ALARM_TRAFFICSTROBESTATE_INFO *pInfo = (ALARM_TRAFFICSTROBESTATE_INFO *)buffer;
//         break;
//     }
    case DH_ALARM_RIOTERDETECTION: {
        ALARM_RIOTERDETECTION_INFO *pInfo = (ALARM_RIOTERDETECTION_INFO *)buffer;

        GatherEvent info;
        info.action = pInfo->nAction;

        //抓拍一张图片
        uint32_t imgBufSize = 5 * 1024 * 1024;
        std::unique_ptr<uint8_t[]> imgBuf(new uint8_t[imgBufSize]);
        this->SnapPicture(std::to_string(pInfo->nChannelID), imgBuf.get(), imgBufSize);

        context->fn(cmd, AlarmEventType::GATHER, ((json)info).dump(), imgBuf.get(), imgBufSize, context->userData);

        break;
    }
    default:
        break;
    }

    return true;
}


int32_t SdkStubImpl::StartEventAnalyze(const std::string &devId, OnAnalyzeData onData, void *userData,
                                       intptr_t &jobId) {
    std::unique_ptr< EventAnalyzeContext> context(new EventAnalyzeContext());
    context->fn = onData;
    context->thisClass = this;
    context->userData = userData;
    context->analyzeId = CLIENT_RealLoadPictureEx(handle_, atoi(devId.c_str()), EVENT_IVS_ALL, true, analyzerDataCallBack,
                         (LDWORD)context.get(), nullptr);
    if (context->analyzeId < 0) {
        int ret = lastError();
        STUB_LLOG_ERROR("CLIENT_RealLoadPictureEx error, ret = {}", ret);
        return ret;
    }

    NET_IN_ATTACH_VIDEOSTAT_SUM videoStatIn = { 0 };
    videoStatIn.nChannel = atoi(devId.c_str());
    videoStatIn.cbVideoStatSum = videoStatSumCallBack;
    NET_OUT_ATTACH_VIDEOSTAT_SUM videoStatOut = { 0 };
    context->videoStatHandle = CLIENT_AttachVideoStatSummary(handle_, &videoStatIn, &videoStatOut, TIMEOUT);
    if (context->videoStatHandle < 0) {
        int ret = lastError();
        STUB_LLOG_ERROR("CLIENT_AttachVideoStatSummary error, ret = {}", ret);
        return ret;
    }

    CLIENT_SetDVRMessCallBackEx1(messageCallBack, (LDWORD)context.get());
    CHECK(CLIENT_StartListenEx(handle_), "CLIENT_StartListenEx");

    STUB_LLOG_INFO("Start event analyze succeed, dev {}, jobId {}", devId, context->analyzeId);

    jobId = (intptr_t)context.get();
    context.release();  // 释放控制权
    return 0;
}

int32_t SdkStubImpl::StopEventAnalyze(intptr_t &jobId) {
    std::unique_ptr<EventAnalyzeContext> context((EventAnalyzeContext *)jobId); // auto delete
    if (context->analyzeId >= 0) {
        STUB_LLOG_INFO("Stop event analyze, jobId {}", jobId);
        CHECK(CLIENT_StopLoadPic(context->analyzeId), "CLIENT_StopLoadPic");
    }
    if (context->videoStatHandle > 0) {
        STUB_LLOG_INFO("Stop video stat summary, jobId {}", jobId);
        CHECK(CLIENT_DetachVideoStatSummary(context->videoStatHandle), "CLIENT_DetachVideoStatSummary");
    }
    STUB_LLOG_INFO("Stop listen");
    CHECK(CLIENT_StopListen(handle_), "CLIENT_StopListen");
    return 0;
}

int32_t SdkStubImpl::SnapPicture(const std::string &devId, uint8_t *imgBuf, uint32_t &imgBufSize) {
    NET_IN_SNAP_PIC_TO_FILE_PARAM inParam = {0};
    inParam.dwSize = sizeof(NET_IN_SNAP_PIC_TO_FILE_PARAM);
    inParam.stuParam.Channel = atoi(devId.c_str());
    inParam.stuParam.Channel = atoi(devId.c_str());
    inParam.stuParam.Quality = 6;
    inParam.stuParam.ImageSize = 2;
    inParam.stuParam.mode = 0;

    NET_OUT_SNAP_PIC_TO_FILE_PARAM outParam = {0};
    outParam.dwSize = sizeof(NET_OUT_SNAP_PIC_TO_FILE_PARAM);
    outParam.szPicBuf = (char *)imgBuf;
    outParam.dwPicBufLen = imgBufSize;

    //clear
    imgBufSize = 0;

    CHECK(CLIENT_SnapPictureToFile(handle_, &inParam, &outParam, TIMEOUT), "CLIENT_SnapPictureToFile");

    imgBufSize = outParam.dwPicBufRetLen;

    STUB_LLOG_INFO("Snap picture succeed for device {}", devId);

    return 0;
}

int32_t SdkStubImpl::GetFtp(const std::string &devId, FtpInfo &ftpInfo) {
    DHDEV_FTP_PROTO_CFG ftpCfg = { 0 };
    DWORD bytesReturn = 0;
    CHECK(CLIENT_GetDevConfig(handle_, DH_DEV_FTP_PROTO_CFG, atoi(devId.c_str()), &ftpCfg, sizeof(DHDEV_FTP_PROTO_CFG),
                              &bytesReturn),
          "CLIENT_GetDevConfig");
    ftpInfo.enable = ftpCfg.bEnable;
    ftpInfo.hostIp = ftpCfg.szHostIp;
    ftpInfo.hostPort = ftpCfg.wHostPort;
    ftpInfo.user = ftpCfg.szUserName;
    ftpInfo.password = ftpCfg.szPassword;
    ftpInfo.dir0 = ftpCfg.szDirName;
    return 0;
}

int32_t SdkStubImpl::SetFtp(const std::string &devId, const FtpInfo &ftpInfo) {
    DHDEV_FTP_PROTO_CFG ftpCfg = { 0 };
    ftpCfg.bEnable = ftpInfo.enable;
    strcpy(ftpCfg.szHostIp, ftpInfo.hostIp.c_str());
    ftpCfg.wHostPort = ftpInfo.hostPort;
    strcpy(ftpCfg.szUserName, ftpInfo.user.c_str());
    strcpy(ftpCfg.szPassword, ftpInfo.password.c_str());
    strcpy(ftpCfg.szDirName, ftpInfo.dir0.c_str());
    CHECK(CLIENT_SetDevConfig(handle_, DH_DEV_FTP_PROTO_CFG, atoi(devId.c_str()), &ftpCfg, sizeof(DHDEV_FTP_PROTO_CFG)),
          "CLIENT_GetDevConfig");
    return 0;
}

int32_t SdkStubImpl::QueryVisitorsFlowRateHistory(const std::string &devId, int32_t granularity,
        const TimePoint &startTime, const TimePoint &endTime, std::vector<VisitorsFlowRateHistory> &histories) {

    NET_IN_FINDNUMBERSTAT inParam = { sizeof(NET_IN_FINDNUMBERSTAT) };
    inParam.nChannelID = atoi(devId.c_str());
    //  查询粒度0：分钟，1：小时，2：日，3：周，4：月，5：季，6：年
    inParam.nGranularityType = granularity;
    inParam.nWaittime = TIMEOUT;
    fromTimePoint(startTime, inParam.stStartTime);
    fromTimePoint(endTime, inParam.stEndTime);
    NET_OUT_FINDNUMBERSTAT outParam{ sizeof(NET_OUT_FINDNUMBERSTAT) };

    STUB_LLOG_INFO("Start to query visitors flow rate history, devId {}, granularity {}, startTime {}, endTime {}",
                   devId, granularity, startTime.ToString(), endTime.ToString());

    LLONG findHand = CLIENT_StartFindNumberStat(handle_, &inParam, &outParam);
    if (findHand == 0) {
        int ret = lastError();
        STUB_LLOG_ERROR("CLIENT_StartFindNumberStat failed! ret {}", ret);
        return ret;
    }

    int start = 0;
    int limit = 100;

    while (true) {
        NET_IN_DOFINDNUMBERSTAT inDoFind = { sizeof(NET_IN_DOFINDNUMBERSTAT) };
        NET_OUT_DOFINDNUMBERSTAT outDoFind = { sizeof(NET_OUT_DOFINDNUMBERSTAT) };
        inDoFind.nBeginNumber = start;
        inDoFind.nCount = limit;
        inDoFind.nWaittime = TIMEOUT;

        std::unique_ptr<DH_NUMBERSTAT[]> stats(new DH_NUMBERSTAT[limit]);
        outDoFind.pstuNumberStat = stats.get();
        outDoFind.nBufferLen = limit * sizeof(DH_NUMBERSTAT);
        for (int i = 0; i < limit; i++) {
            outDoFind.pstuNumberStat[i].dwSize = sizeof(DH_NUMBERSTAT);
        }

        CHECK(CLIENT_DoFindNumberStat(findHand, &inDoFind, &outDoFind), "CLIENT_DoFindNumberStat");
        for (int i = 0; i < outDoFind.nCount; i++) {
            sdkproxy::sdk::VisitorsFlowRateHistory his;
            DH_NUMBERSTAT &data = outDoFind.pstuNumberStat[i];
            his.ruleName = data.szRuleName;
            his.startDateTime = netTimeToString(data.stuStartTime);
            his.endDateTime = netTimeToString(data.stuEndTime);
            his.inTotal = data.nEnteredSubTotal;
            his.outTotal = data.nExitedSubtotal;
            his.avgInside = data.nAvgInside;
            his.maxInside = data.nMaxInside;
            histories.push_back(his);
        }

        // 已处理最后一页
        if (outDoFind.nCount < limit) {
            break;
        }

        start += limit;
    }

    CLIENT_StopFindNumberStat(findHand);

    return 0;
}

}
}
}
