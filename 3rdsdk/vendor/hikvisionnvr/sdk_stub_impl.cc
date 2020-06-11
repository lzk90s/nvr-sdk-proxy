#include <iomanip>
#include <ctime>
#include <chrono>

#include "common/helper/logger.h"
#include "common/helper/singleton.h"
#include "common/helper/timer.h"

#include "3rdsdk/stub/po_type_serialization.h"
#include "3rdsdk/vendor/hikvisionnvr/sdk_stub_impl.h"

#include "HCNetSDK.h"

namespace sdkproxy {
namespace sdk {
namespace hikvisionnvr {
using json = nlohmann::json;

static Logger logger("hik_nvr");

#define CHECK(method_call, msg)                                                         \
do {                                                                                    \
    BOOL __result = FALSE;                                                              \
    if ((__result= (method_call)) != TRUE) {                                            \
        DWORD err = NET_DVR_GetLastError();                                             \
        LLOG_ERROR(logger, "Call sdk error, result [{}] - {}", err, msg); return err;   \
    }                                                                                   \
} while (0)

#define CHECK_EX(method_call, msg, defer)                                               \
do {                                                                                    \
    BOOL __result = FALSE;                                                              \
    if ((__result= (method_call)) != TRUE) {                                            \
        DWORD err = NET_DVR_GetLastError();                                             \
        LLOG_ERROR(logger, "Call sdk error, result [{}] - {}", err, msg); return err;   \
        (defer)();                                                                      \
    }                                                                                   \
} while (0)

#define GET_YEAR(_time_)      (((_time_)>>26) + 2000)
#define GET_MONTH(_time_)     (((_time_)>>22) & 15)
#define GET_DAY(_time_)       (((_time_)>>17) & 31)
#define GET_HOUR(_time_)      (((_time_)>>12) & 31)
#define GET_MINUTE(_time_)    (((_time_)>>6)  & 63)
#define GET_SECOND(_time_)    (((_time_)>>0)  & 63)

class SdkHolder {
public:
    SdkHolder() {
        if (FALSE == NET_DVR_Init()) {
            throw std::runtime_error("Init sdk error");
        }

        NET_DVR_SetReconnect(5000, 1);
        NET_DVR_SetConnectTime(5000, 3);
        NET_DVR_SetLogPrint(FALSE);

        LLOG_INFO(logger, "Succeed to initialize hikvision nvr sdk, the sdk version is {}, build {}", NET_DVR_GetSDKVersion(),
                  NET_DVR_GetSDKBuildVersion());
    }

    ~SdkHolder() {
        NET_DVR_Cleanup();
    }
};

static void toTimePoint(TimePoint &tp, const NET_DVR_TIME_SEARCH &tm) {
    tp.year = tm.wYear;
    tp.month = tm.byMonth;
    tp.day = tm.byDay;
    tp.hour = tm.byHour;
    tp.minute = tm.byMinute;
    tp.second = tm.bySecond;
}

static void fromTimePoint(const TimePoint &tp, NET_DVR_TIME &tm) {
    tm.dwYear = tp.year;
    tm.dwMonth = tp.month;
    tm.dwDay = tp.day;
    tm.dwHour = tp.hour;
    tm.dwMinute = tp.minute;
    tm.dwSecond = tp.second;
}

static void fromTimePoint(const TimePoint &tp, NET_DVR_TIME_SEARCH_COND &tm) {
    tm.wYear = tp.year;
    tm.byMonth = tp.month;
    tm.byDay = tp.day;
    tm.byHour = tp.hour;
    tm.byMinute = tp.minute;
    tm.bySecond = tp.second;
}

static std::string getCurrentDateTime(const std::string &format = "%F %T") {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), format.c_str());
    return ss.str();
}

SdkStubImpl::SdkStubImpl() : SdkStub("hikvisionnvr", "Hikvision net sdk", 8000) {
    //init singleton
    Singleton<SdkHolder>::getInstance();
    channelNum_ = 0;
    startChan_ = 0;
}

SdkStubImpl::~SdkStubImpl() {
    //退出登录
    Logout();
}

int32_t SdkStubImpl::Login(const std::string &ip, const std::string &user, const std::string &password) {
    int ret = 0;

    NET_DVR_USER_LOGIN_INFO loginInfo = { 0 };
    strncpy(loginInfo.sDeviceAddress, ip.c_str(), NET_DVR_DEV_ADDRESS_MAX_LEN - 1);
    loginInfo.wPort = GetPort();
    strncpy(loginInfo.sUserName, user.c_str(), NET_DVR_LOGIN_USERNAME_MAX_LEN - 1);
    strncpy(loginInfo.sPassword, password.c_str(), NET_DVR_LOGIN_PASSWD_MAX_LEN - 1);
    loginInfo.bUseAsynLogin = FALSE;

    NET_DVR_DEVICEINFO_V40 devInfo = { 0 };
    handle_ = NET_DVR_Login_V40(&loginInfo, &devInfo);
    if ((LONG)handle_ < 0) {
        ret = NET_DVR_GetLastError();
        LLOG_ERROR(logger, "Failed to login {}, ret {}", ip, ret);
        return ret;
    }

    channelNum_ = devInfo.struDeviceV30.byChanNum + devInfo.struDeviceV30.byIPChanNum;
    startChan_ = devInfo.struDeviceV30.byStartChan;
    ip_ = ip;

    LLOG_INFO(logger, "Succeed to login {}, handle {}", ip, handle_);
    LLOG_INFO(logger, "SerialNumber={}, DVRType={}, ChannelNum={}", devInfo.struDeviceV30.sSerialNumber,
              devInfo.struDeviceV30.byDVRType, channelNum_);

    return 0;
}

int32_t SdkStubImpl::Logout() {
    NET_DVR_Logout((LONG)handle_);
    return 0;
}

int32_t SdkStubImpl::QueryDevice(std::vector<Device> &devices) {
    if (channelNum_ <= 0) {
        return 0;
    }

    DWORD  dwReturned;
    NET_DVR_IPPARACFG_V40 ipAccessCfg = { 0 };
    BOOL succeed = NET_DVR_GetDVRConfig(handle_, NET_DVR_GET_IPPARACFG_V40, 0, &ipAccessCfg,
                                        sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);
    if (!succeed) {//不支持ip接入,9000以下设备不支持禁用模拟通道
        NET_DVR_DEVICECFG devCfg = { 0 };
        DWORD bytesreturned = 0;
        CHECK(NET_DVR_GetDVRConfig(handle_, NET_DVR_GET_DEVICECFG, 0, &devCfg, sizeof(NET_DVR_DEVICECFG),
                                   &bytesreturned), "NET_DVR_GetDVRConfig");
        for (int i = devCfg.byStartChan; i <= devCfg.byChanNum; i++) {
            Device dev;
            dev.id = std::to_string(i);
            dev.name = std::string("Camera_") + std::to_string(i);
            dev.ip = ip_;
            LLOG_INFO(logger, "(1)Found channel, id={}, name={}, ip={}", dev.id, dev.name, dev.ip);
            devices.push_back(dev);
        }
    } else { //支持IP接入，9000设备
        //模拟通道
        for (int i = 0; i < MAX_ANALOG_CHANNUM; i++) {
            if (i < channelNum_) {
                int iChannelIdx = i + startChan_;
                if (ipAccessCfg.byAnalogChanEnable[i]) {
                    Device dev;
                    dev.id = std::to_string(iChannelIdx);
                    dev.name = getChannelName(iChannelIdx);
                    dev.ip = ip_;
                    LLOG_INFO(logger, "(2)Found channel, id={}, name={}, ip={}", dev.id, dev.name, dev.ip);
                    devices.push_back(dev);
                }
            }
        }

        //数字通道
        for (int i = 0; i < MAX_IP_CHANNEL; i++) {
            if (ipAccessCfg.struStreamMode[i].uGetStream.struChanInfo.byEnable &&
                    ipAccessCfg.struIPDevInfo[i].byEnable) { //ip通道在线
                int iChannelIdx = i + ipAccessCfg.dwStartDChan;
                Device dev;
                dev.id = std::to_string(iChannelIdx);
                dev.name = getChannelName(iChannelIdx);
                dev.ip = ipAccessCfg.struIPDevInfo[i].struIP.sIpV4;
                LLOG_INFO(logger, "(3)Found channel, id={}, name={}, ip={}", dev.id, dev.name, dev.ip);
                devices.push_back(dev);
            }
        }
    }

    return 0;
}

typedef struct tagRealPlayInfo {
    LONG playHandle;
    SdkStub::OnRealPlayData fn;

    tagRealPlayInfo() {
        playHandle = -1;
        fn = nullptr;
    }
} RealPlayContext;

void CALLBACK previewDataCallback (LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    RealPlayContext *context = (RealPlayContext *)pUser;
    if (nullptr == context || nullptr == context->fn) {
        return;
    }
    context->fn((intptr_t)lPlayHandle, pBuffer, dwBufSize);
}

int32_t SdkStubImpl::StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context(new RealPlayContext());

    NET_DVR_PREVIEWINFO previewInfo = { 0 };
    previewInfo.hPlayWnd = 0;
    previewInfo.lChannel = atoi(devId.c_str());
    previewInfo.dwLinkMode = 0;
    previewInfo.bBlocked = 1;
    previewInfo.dwDisplayBufNum = 1;

    LONG playHandle = NET_DVR_RealPlay_V40(handle_, &previewInfo, previewDataCallback, (void *)context.get());
    if (playHandle < 0) {
        int ret = NET_DVR_GetLastError();
        LLOG_ERROR(logger, "NET_DVR_RealPlay_V40 error, ret = {}", ret);
        return ret;
    }

    context->playHandle = playHandle;
    jobId = (intptr_t)context.get();
    context.release();

    return 0;
}

int32_t SdkStubImpl::StopRealStream(intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context((RealPlayContext *)jobId); //auto delete

    if (context->playHandle >= 0) {
        NET_DVR_StopRealPlay(context->playHandle);
    }

    LLOG_INFO(logger, "Stop real play succeed, playId {}", context->playHandle);

    return 0;
}

int32_t SdkStubImpl::QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime,
                                 std::vector<RecordInfo> &records) {
    NET_DVR_FILECOND_V50 fileCond = { 0 };
    fileCond.struStreamID.dwChannel = atoi(devId.c_str());
    fileCond.dwFileType = 0xff;
    fromTimePoint(startTime, fileCond.struStartTime);
    fromTimePoint(endTime, fileCond.struStopTime);

    // find file
    LONG findHandle = NET_DVR_FindFile_V50(handle_, &fileCond);
    if (findHandle < 0) {
        DWORD ret = NET_DVR_GetLastError();
        LLOG_ERROR(logger, "Failed to find file for dev {}, ret {}", devId, ret);
        return ret;
    }

    while (true) {
        NET_DVR_FINDDATA_V50 fileData = { 0 };
        int result = NET_DVR_FindNextFile_V50(findHandle, &fileData);
        if (result == NET_DVR_FILE_SUCCESS) {
            sdkproxy::sdk::RecordInfo r;
            r.fileName = fileData.sFileName;
            r.fileSize = fileData.dwFileSize;
            toTimePoint(r.startTime, fileData.struStartTime);
            toTimePoint(r.endTime, fileData.struStopTime);
            records.push_back(r);
        } else if (result == NET_DVR_ISFINDING) {
            continue;
        } else if (result == NET_DVR_FILE_NOFIND || result == NET_DVR_NOMOREFILE) {
            break;
        } else {
            break;
        }
    }

    NET_DVR_FindClose_V30(findHandle);

    return 0;
}

//playback closure
typedef struct tagPlaybackInfo {
    intptr_t downloadId;
    SdkStub::OnDownloadData fn;
    void *thisClass;
    std::string file;
    CppTime::Timer timer;
    bool complete;

    tagPlaybackInfo() {
        downloadId = -1;
        fn = nullptr;
        thisClass = nullptr;
        complete = false;
    }
} PlaybackContext;

static void CALLBACK downloadDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,
                                          void *pUser) {
    PlaybackContext *context = static_cast<PlaybackContext *>(pUser);
    if (nullptr != context) {
        context->fn(lPlayHandle, pBuffer, dwBufSize);
    }
}

int32_t SdkStubImpl::DownloadRecordByTime(const std::string &devId, const TimePoint &startTime,
                                          const TimePoint &endTime, OnDownloadData onData, intptr_t &jobId) {
    //new context
    std::unique_ptr<PlaybackContext> context(new PlaybackContext());
    context->fn = onData;
    context->thisClass = this;

    NET_DVR_PLAYCOND downloadCond = { 0 };
    downloadCond.dwChannel = atoi(devId.c_str());
    fromTimePoint(startTime, downloadCond.struStartTime);
    fromTimePoint(endTime, downloadCond.struStopTime);

    // download by time
    context->file = "/tmp/" + std::to_string((uintptr_t)context.get());
    context->downloadId = NET_DVR_GetFileByTime_V40(handle_, (char *)context->file.c_str(), &downloadCond);
    if (0 > context->downloadId) {
        DWORD err = NET_DVR_GetLastError();
        LLOG_ERROR(logger, "NET_DVR_GetFileByTime_V40 error {}", err);
        return err;
    }

    // start download
    CHECK(NET_DVR_SetPlayDataCallBack_V40(context->downloadId, downloadDataCallBack,
                                          (void *)context.get()), "NET_DVR_SetPlayDataCallBack");
    CHECK(NET_DVR_PlayBackControl_V40(context->downloadId, NET_DVR_PLAYSTART), "NET_DVR_PlayBackControl");

    // period check the download position
    PlaybackContext *playContextPtr = context.get();
    auto interval = std::chrono::milliseconds(100);
    context->timer.add(interval, [playContextPtr](CppTime::timer_id) {
        if (!playContextPtr->complete) {
            if (100 == NET_DVR_GetDownloadPos(playContextPtr->downloadId) ) {
                LLOG_INFO(logger, "Record download 100%, downloadId {}", playContextPtr->downloadId);
                playContextPtr->complete = true;
                playContextPtr->fn(playContextPtr->downloadId, nullptr, 0);
            }
        }
    }, interval);

    LLOG_INFO(logger, "Downloading file for dev {} between {} and {}, downloadId {}", devId, startTime.ToString(),
              endTime.ToString(), context->downloadId);

    jobId = (intptr_t)context.get();
    context.release();  // 释放控制权

    return 0;
}

int32_t SdkStubImpl::StopDownloadRecord(intptr_t &jobId) {
    std::unique_ptr<PlaybackContext> context((PlaybackContext *)jobId); // auto delete
    if (nullptr != context) {
        if (context->downloadId >= 0) {
            LLOG_INFO(logger, "Stop download file, downloadId {}", context->downloadId);
            CHECK(NET_DVR_StopGetFile(context->downloadId), "NET_DVR_StopGetFile");
            unlink(context->file.c_str());
        }
        jobId = 0;
    }
    return 0;
}

typedef struct tagEventContext {
    LONG handle;
    SdkStub::OnAnalyzeData fn;
    void *thisClass;
    void *userData;

    tagEventContext() {
        handle = -1;
        fn = nullptr;
        thisClass = nullptr;
    }
} EventContext;

static std::map<LONG, EventContext *> alarmContextMapping;

static BOOL alarmMsgCallback(LONG lCommand, LONG lUserID, char *pBuf, DWORD dwBufLen) {
    EventContext *context = nullptr;
    //double check
    if (alarmContextMapping.find(lUserID) != alarmContextMapping.end()) {
        if (alarmContextMapping.find(lUserID) != alarmContextMapping.end()) {
            context = alarmContextMapping[lUserID];
        }
    }

    if (nullptr == context || nullptr == context->fn || nullptr == pBuf) {
        return FALSE;
    }

    LLOG_INFO(logger, "Received alarm, type {}, handle {}", lCommand, lUserID);

    switch (lCommand) {
    case COMM_ALARM_RULE: {
        NET_VCA_RULE_ALARM &struVcaRuleAlarm = *((NET_VCA_RULE_ALARM *)pBuf);
        uint8_t *imgData = (uint8_t *)struVcaRuleAlarm.pImage;
        uint32_t imgLen = (uint32_t)struVcaRuleAlarm.dwPicDataLen;

        switch (struVcaRuleAlarm.struRuleInfo.wEventTypeEx) {
        case ENUM_VCA_EVENT_LEFT: {
            NET_DVR_TIME struAbsTime = { 0 };
            struAbsTime.dwYear = GET_YEAR(struVcaRuleAlarm.dwAbsTime);
            struAbsTime.dwMonth = GET_MONTH(struVcaRuleAlarm.dwAbsTime);
            struAbsTime.dwDay = GET_DAY(struVcaRuleAlarm.dwAbsTime);
            struAbsTime.dwHour = GET_HOUR(struVcaRuleAlarm.dwAbsTime);
            struAbsTime.dwMinute = GET_MINUTE(struVcaRuleAlarm.dwAbsTime);
            struAbsTime.dwSecond = GET_SECOND(struVcaRuleAlarm.dwAbsTime);

            char csRuleName[33] = { 0 };
            memcpy(csRuleName, struVcaRuleAlarm.struRuleInfo.byRuleName, 32);
            NET_VCA_POLYGON tempPolygon;
            memcpy(&tempPolygon, &struVcaRuleAlarm.struRuleInfo.uEventParam.struTake.struRegion, sizeof(NET_VCA_POLYGON));
            for (int i = 0; i < (int)tempPolygon.dwPointNum; i++) {
                char szInfoBuf[128] = { 0 };
                sprintf(szInfoBuf, "[%f][%f]", tempPolygon.struPos[i].fX, tempPolygon.struPos[i].fY);
            }

            ObjectLeftEvent obj;
            obj.ruleName = csRuleName;
            obj.dateTime = getCurrentDateTime();
            context->fn(lCommand, AlarmEventType::OBJECT_LEFT, ((json)obj).dump(), imgData, imgLen, context->userData);
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    default:
        break;
    }

    return TRUE;
}

int32_t SdkStubImpl::StartEventAnalyze(const std::string &devId, OnAnalyzeData onData, void *userData,
                                       intptr_t &jobId) {
    std::unique_ptr<EventContext> context(new EventContext());
    context->fn = onData;
    context->thisClass = this;
    context->userData = userData;

    LLOG_INFO(logger, "Start event analyze, devId {}, ip {}", devId, this->ip_);

    NET_DVR_SetDVRMessCallBack_EX(alarmMsgCallback);

    //启用布防
    NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
    struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
    struSetupParam.byRetVQDAlarmType = TRUE;
    struSetupParam.byRetAlarmTypeV40 = TRUE;
    //布防优先级：0- 一等级（高），1- 二等级（中）
    struSetupParam.byLevel = 1;
    //上传报警信息类型: 0- 老报警信息(NET_DVR_PLATE_RESULT), 1- 新报警信息(NET_ITS_PLATE_RESULT)
    struSetupParam.byAlarmInfoType = 1;

    context->handle = NET_DVR_SetupAlarmChan_V41(handle_, &struSetupParam);
    if (context->handle < 0) {
        int ret = NET_DVR_GetLastError();
        LLOG_ERROR(logger, "NET_DVR_SetupAlarmChan_V50 failed, ret {}", ret);
        return ret;
    }

    //保存到mapping中
    alarmContextMapping[(LONG)this->handle_] = context.get();

    jobId = (intptr_t)context.get();
    context.release();

    return 0;
}

int32_t SdkStubImpl::StopEventAnalyze(intptr_t &jobId) {
    std::unique_ptr<EventContext> context((EventContext *)jobId); // auto delete
    if (context->handle >= 0) {
        LLOG_INFO(logger, "Stop event analyze succeed, jobId {}", jobId);
        CHECK(NET_DVR_CloseAlarmChan_V30(context->handle), "NET_DVR_CloseAlarmChan_V30");
    }
    alarmContextMapping.erase((LONG)this->handle_);
    return 0;
}

int32_t SdkStubImpl::GetFtp(const std::string &devId, FtpInfo &ftpInfo) {
    NET_DVR_FTPCFG_V40 ftpCfg = { 0 };
    int result = 0;
    NET_DVR_FTP_TYPE ftpType = { 0 };
    ftpType.byType = 0; //首选ftp
    CHECK(NET_DVR_GetDeviceConfig(handle_, NET_DVR_GET_FTPCFG_V40, 1, &ftpType, sizeof(NET_DVR_FTP_TYPE), &result, &ftpCfg,
                                  sizeof(NET_DVR_FTPCFG_V40)), "CLIENT_GetDevConfig");
    ftpInfo.enable = (bool)ftpCfg.byEnableFTP;
    ftpInfo.hostIp = (char *)ftpCfg.unionServer.struAddrIP.struIp.sIpV4;
    ftpInfo.hostPort = ftpCfg.wFTPPort;
    ftpInfo.user = (char *)ftpCfg.szUserName;
    ftpInfo.password = (char *)ftpCfg.szPassWORD;
    ftpInfo.dir0 = (char *)ftpCfg.szTopCustomDir;
    ftpInfo.dir1 = (char *)ftpCfg.szSubCustomDir;

    return 0;
}

int32_t SdkStubImpl::SetFtp(const std::string &devId, const FtpInfo &ftpInfo) {
    NET_DVR_FTPCFG_V40 ftpCfg = { 0 };
    int result = 0;
    NET_DVR_FTP_TYPE ftpType = { 0 };
    ftpType.byType = 0; //首选ftp
    ftpCfg.byEnableFTP = ftpInfo.enable;
    ftpCfg.byProtocolType = 0;  //ftp protocol
    strcpy((char *)ftpCfg.unionServer.struAddrIP.struIp.sIpV4, ftpInfo.hostIp.c_str());
    ftpCfg.wFTPPort = ftpInfo.hostPort;
    strcpy((char *)ftpCfg.szUserName, ftpInfo.user.c_str());
    strcpy((char *)ftpCfg.szPassWORD, ftpInfo.password.c_str());
    if (!ftpInfo.dir0.empty() && !ftpInfo.dir1.empty()) {
        ftpCfg.byDirLevel = 2;
        //byTopDirMode: 0x1- 使用设备名，0x2- 使用设备号，0x3- 使用设备ip地址，0x4- 使用监测点，0x5- 使用时间(年月)，0x6- 使用自定义，0x7- 违规类型，0x8- 方向，0x9- 地点，0xa- 栋号单元号，0xb- 期号
        ftpCfg.byTopDirMode = 6;
        //bySubDirMode: 0x1- 使用通道名，0x2- 使用通道号，0x3- 使用时间(年月日)，0x4- 使用车道号，0x5- 使用自定义，0x6- 违规类型，0x7- 方向，0x8- 地点，0x9- 车位编号，0xa- 栋号单元号，0xb- 门口机编号，0xc- 设备编号
        ftpCfg.bySubDirMode = 5;
    } else if (!ftpInfo.dir0.empty()) {
        ftpCfg.byDirLevel = 1;
        //byTopDirMode: 0x1- 使用设备名，0x2- 使用设备号，0x3- 使用设备ip地址，0x4- 使用监测点，0x5- 使用时间(年月)，0x6- 使用自定义，0x7- 违规类型，0x8- 方向，0x9- 地点，0xa- 栋号单元号，0xb- 期号
        ftpCfg.byTopDirMode = 6;
    } else {
        ftpCfg.byDirLevel = 0;
    }
    strcpy((char *)ftpCfg.szTopCustomDir, ftpInfo.dir0.c_str());
    strcpy((char *)ftpCfg.szSubCustomDir, ftpInfo.dir1.c_str());

    CHECK(NET_DVR_SetDeviceConfig(handle_, NET_DVR_SET_FTPCFG_V40, 1, &ftpType, sizeof(NET_DVR_FTP_TYPE), &result, &ftpCfg,
                                  sizeof(NET_DVR_FTPCFG_V40)), "NET_DVR_SetDeviceConfig");
    return 0;
}

std::string  SdkStubImpl::getChannelName(int channelIdx) {
    NET_DVR_PICCFG_V40 picCfg = {0};
    DWORD  dwReturned;
    if (TRUE != NET_DVR_GetDVRConfig(handle_, NET_DVR_GET_PICCFG_V40, channelIdx, &picCfg, sizeof(NET_DVR_PICCFG_V40),
                                     &dwReturned)) {
        LLOG_WARN(logger, "Failed to get channel name for {}", channelIdx);
        return "";
    }
    return std::string((char *)picCfg.sChanName);
}


}
}
}