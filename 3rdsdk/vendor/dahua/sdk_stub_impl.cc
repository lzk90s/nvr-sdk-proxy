#include "common/helper/logger.h"

#include "3rdsdk/vendor/dahua/sdk_stub_impl.h"

#include "DPSDK_Core_Define.h"
#include "DPSDK_Core.h"
#include "dhplay.h"

namespace sdkproxy {
namespace sdk {
namespace dahua {
using json = nlohmann::json;

static Logger logger("dahua");

#define SUFFIX(msg)               std::string("[{}] ").append(msg)
#define STUB_LLOG_DEBUG(fmt, ...) LLOG_DEBUG(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_INFO(fmt, ...)  LLOG_INFO(logger, SUFFIX(fmt), (this)->ip_, ##__VA_ARGS__)
#define STUB_LLOG_WARN(fmt, ...)  LLOG_WARN(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)
#define STUB_LLOG_ERROR(fmt, ...) LLOG_ERROR(logger, SUFFIX(fmt), this->ip_, ##__VA_ARGS__)

#define CHECK(method_call, msg)                                                 \
    do {                                                                        \
        int __result = 0;                                                       \
        if ((__result = (method_call)) != 0) {                                  \
            STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", __result, msg); \
            return __result;                                                    \
        }                                                                       \
    } while (0)

#define CHECK_EX(method_call, msg, defer)                                       \
    do {                                                                        \
        int __result = 0;                                                       \
        if ((__result = (method_call)) != 0) {                                  \
            STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", __result, msg); \
            (defer)();                                                          \
            return __result;                                                    \
        }                                                                       \
    } while (0)

#define PLAY_CHECK(port, method_call, msg)                                      \
    do {                                                                        \
        if (((method_call)) != TRUE) {                                          \
            DWORD __result = PLAY_GetLastError(port);                           \
            STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", __result, msg); \
            return __result;                                                    \
        }                                                                       \
    } while (0)

#define PLAY_CHECK_EX(port, method_call, msg, defer)                            \
    do {                                                                        \
        if (((method_call)) != TRUE) {                                          \
            DWORD __result = PLAY_GetLastError(port);                           \
            STUB_LLOG_ERROR("Call sdk error, result [{}] - {}", __result, msg); \
            (defer)();                                                          \
            return __result;                                                    \
        }                                                                       \
    } while (0)

static void toTimePoint(TimePoint &tp, const time_t &tm) {
    tp.FromTime(tm);
}

static void fromTimePoint(const TimePoint &tp, time_t &tm) {
    tm = tp.ToTime();
}

SdkStubImpl::SdkStubImpl() : SdkStub("dahua", "Dahua platform sdk", 9000) {
    handle_ = -1;
}

SdkStubImpl::~SdkStubImpl() {
    Logout();
}

int32_t SdkStubImpl::QueryDevice(std::vector<Device> &devices) {
    int nGroupLen = 0;
    CHECK(DPSDK_LoadDGroupInfo(handle_, nGroupLen, 120000), "DPSDK_LoadDGroupInfo");

    if (DPSDK_HasLogicOrg(handle_)) {
        Dep_Info_Ex_t pDepInfoEx;
        memset(&pDepInfoEx, 0, sizeof(pDepInfoEx));
        if (0 == DPSDK_GetLogicRootDepInfo(handle_, &pDepInfoEx)) {
            CHECK(createNodeDep(pDepInfoEx.szCoding, 1, [&](Device &c) { devices.push_back(c); }), "createNodeDep");
        }
    } else {
        CHECK(createNodeDep("001", 1, [&](Device &c) { devices.push_back(c); }), "createNodeDep root");
    }

    return 0;
}

int32_t SdkStubImpl::Login(const std::string &ip, const std::string &user, const std::string &password) {
    this->ip_ = ip;

    CHECK(DPSDK_Create(DPSDK_CORE_SDK_SERVER, handle_), "DPSDK_Create");

    Login_Info_t loginInfo = {0};
    strcpy(loginInfo.szIp, ip.c_str());
    loginInfo.nPort = GetPort();
    strcpy(loginInfo.szUsername, user.c_str());
    strcpy(loginInfo.szPassword, password.c_str());
    loginInfo.nProtocol = DPSDK_PROTOCOL_VERSION_II;

    CHECK(DPSDK_Login(handle_, &loginInfo), "DPSDK_Login");
    CHECK(DPSDK_SetLog(handle_, dpsdk_log_level_e::DPSDK_LOG_LEVEL_INFO, "/var/log/dpsdk.log", true, false), "DPSDK_SetLog");

    STUB_LLOG_INFO("Succeed to login {}, handle {}", ip, handle_);

    return 0;
}

int32_t SdkStubImpl::Logout() {
    if (handle_ >= 0) {
        DPSDK_Logout(handle_);
    }
    return 0;
}

typedef struct tagRealPlayInfo : public CallbackClosure {
    int32_t realSeq;
    int32_t port;
    SdkStub::OnRealPlayData fn;

    tagRealPlayInfo() {
        realSeq = -1;
        port    = -1;
        fn      = nullptr;
    }
} RealPlayContext;

int32_t mediaDataCallback(int32_t nPDLLHandle, int32_t nSeq, int32_t nMediaType, const char *szNodeId, int32_t nParamVal, char *szData,
                          int32_t nDataLen, void *pUserParam) {
    RealPlayContext *context = (RealPlayContext *)pUserParam;
    if (nullptr == context || nullptr == context->fn) {
        return 0;
    }
    context->fn(nSeq, (const uint8_t *)szData, nDataLen);
    return 0;
}

int32_t SdkStubImpl::StartRealStream(const std::string &devId, OnRealPlayData onData, intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context(new RealPlayContext());

    LOG_DEBUG("StartRealStream: devId {}", devId);

    //获取实时流
    Get_RealStream_Info_t realInfo = {0};
    strcpy(realInfo.szCameraId, devId.c_str());
    realInfo.nRight      = DPSDK_CORE_NOT_CHECK_RIGHT;
    realInfo.nStreamType = DPSDK_CORE_STREAMTYPE_MAIN;
    realInfo.nTransType  = DPSDK_CORE_TRANSTYPE_TCP;
    realInfo.nMediaType  = DPSDK_CORE_MEDIATYPE_VIDEO;
    int32_t nRealSeq;
    CHECK(DPSDK_GetRealStream(handle_, nRealSeq, &realInfo, mediaDataCallback, (void *)context.get()), "DPSDK_GetRealStream");

    STUB_LLOG_INFO("Start real play succeed, playId {}", nRealSeq);

    context->fn        = onData;
    context->thisClass = this;
    context->realSeq   = nRealSeq;
    jobId              = (intptr_t)context.get();

    context.release(); //释放控制权

    return 0;
}

int32_t SdkStubImpl::StopRealStream(intptr_t &jobId) {
    std::unique_ptr<RealPlayContext> context((RealPlayContext *)jobId); // auto delete

    if (context->realSeq >= 0) {
        DPSDK_CloseRealStreamBySeq(handle_, context->realSeq);
    }

    STUB_LLOG_INFO("Stop real play succeed, playId {}", context->realSeq);

    return 0;
}

int32_t SdkStubImpl::QueryRecord(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime, std::vector<RecordInfo> &records) {
    Query_Record_Info_t queryInfo = {0};
    strcpy(queryInfo.szCameraId, devId.c_str());
    queryInfo.nRight      = DPSDK_CORE_NOT_CHECK_RIGHT;
    queryInfo.nSource     = DPSDK_CORE_PB_RECSOURCE_PLATFORM;
    queryInfo.nRecordType = DPSDK_CORE_PB_RECORD_UNKONWN;
    fromTimePoint(startTime, (time_t &)queryInfo.uBeginTime);
    fromTimePoint(endTime, (time_t &)queryInfo.uEndTime);

    int nRecordCount = -1;
    CHECK(DPSDK_QueryRecord(handle_, &queryInfo, nRecordCount), "DPSDK_QueryRecord");
    if (nRecordCount > 0) {
        Record_Info_t stuRecordInfo = {0};
        strncpy(stuRecordInfo.szCameraId, devId.c_str(), sizeof(stuRecordInfo.szCameraId));
        stuRecordInfo.nBegin = 0;
        stuRecordInfo.nCount = nRecordCount;
        std::unique_ptr<Single_Record_Info_t[]> rs(new Single_Record_Info_t[stuRecordInfo.nCount]);
        stuRecordInfo.pSingleRecord = rs.get();
        CHECK(DPSDK_GetRecordInfo(handle_, &stuRecordInfo), "DPSDK_GetRecordInfo");
        for (int i = 0; i < (int)stuRecordInfo.nRetCount; i++) {
            Single_Record_Info_t &stuSingle = stuRecordInfo.pSingleRecord[i];
            sdkproxy::sdk::RecordInfo r;
            r.fileName = std::to_string(stuSingle.nFileIndex);
            r.fileSize = stuSingle.uLength;
            toTimePoint(r.startTime, stuSingle.uBeginTime);
            toTimePoint(r.endTime, stuSingle.uEndTime);
            records.push_back(r);
        }
    }
    return 0;
}

int32_t SdkStubImpl::DownloadRecordByTime(const std::string &devId, const TimePoint &startTime, const TimePoint &endTime, OnDownloadData onData,
                                          intptr_t &jobId) {
    STUB_LLOG_ERROR("Unimplemented method");
    return -1;
}

int32_t SdkStubImpl::StopDownloadRecord(intptr_t &jobId) {
    STUB_LLOG_ERROR("Unimplemented method");
    return -1;
}

int32_t SdkStubImpl::createNodeDep(const std::string &depId, int nType, std::function<void(Device &c)> onData) {
    std::unique_ptr<Get_Dep_Count_Info_t> pGetCountInfo(new Get_Dep_Count_Info_t{0});
    strcpy(pGetCountInfo->szCoding, depId.c_str());
    CHECK(DPSDK_GetDGroupCount(handle_, pGetCountInfo.get()), "DPSDK_GetDGroupCount");

    std::unique_ptr<Get_Dep_Info_Ex_t> pGetDepInfo(new Get_Dep_Info_Ex_t{0});
    strcpy(pGetDepInfo->szCoding, depId.c_str());
    pGetDepInfo->nDepCount     = pGetCountInfo->nDepCount;
    pGetDepInfo->nDeviceCount  = pGetCountInfo->nDeviceCount;
    pGetDepInfo->nChannelCount = pGetCountInfo->nChannelCount;
    std::unique_ptr<Dep_Info_t[]> pDepInfo(new Dep_Info_t[pGetCountInfo->nDepCount]);
    memset(pDepInfo.get(), 0, sizeof(Dep_Info_t) * pGetCountInfo->nDepCount);
    std::unique_ptr<Device_Info_Ex_t[]> pDeviceInfo(new Device_Info_Ex_t[pGetCountInfo->nDeviceCount]);
    memset(pDeviceInfo.get(), 0, sizeof(Device_Info_Ex_t) * pGetCountInfo->nDeviceCount);
    std::unique_ptr<Enc_Channel_Info_Ex_t[]> pEncChannelnfo(new Enc_Channel_Info_Ex_t[pGetCountInfo->nChannelCount]);
    memset(pEncChannelnfo.get(), 0, sizeof(Enc_Channel_Info_Ex_t) * pGetCountInfo->nChannelCount);
    pGetDepInfo->pDepInfo       = pDepInfo.get();
    pGetDepInfo->pDeviceInfo    = pDeviceInfo.get();
    pGetDepInfo->pEncChannelnfo = pEncChannelnfo.get();
    CHECK(DPSDK_GetDGroupInfoEx(handle_, pGetDepInfo.get()), "DPSDK_GetDGroupInfoEx");

    int ChildGroupCount = 0;
    for (uint32_t i = 0; i < pGetDepInfo->nDepCount; ++i) {
        int32_t res = createNodeDep(pGetDepInfo->pDepInfo[i].szCoding, nType, onData);
        if (res == 0) {
            ChildGroupCount++;
        }
    }

    for (uint32_t i = 0; i < pGetDepInfo->nDeviceCount; i++) {
        std::unique_ptr<Get_Channel_Info_Ex_t> pGetChannelInfo(new Get_Channel_Info_Ex_t{0});
        strcpy(pGetChannelInfo->szDeviceId, pGetDepInfo->pDeviceInfo[i].szId);
        pGetChannelInfo->nEncChannelChildCount = pGetDepInfo->pDeviceInfo[i].nEncChannelChildCount;
        std::unique_ptr<Enc_Channel_Info_Ex_t[]> pEncChannelnfo(new Enc_Channel_Info_Ex_t[pGetChannelInfo->nEncChannelChildCount]);
        if (pGetChannelInfo->nEncChannelChildCount > 0) {
            pGetChannelInfo->pEncChannelnfo = pEncChannelnfo.get();
            memset(pGetChannelInfo->pEncChannelnfo, 0, sizeof(Enc_Channel_Info_Ex_t) * pGetChannelInfo->nEncChannelChildCount);
        }
        CHECK(DPSDK_GetChannelInfoEx(handle_, pGetChannelInfo.get()), "DPSDK_GetChannelInfoEx");

        for (uint32_t nChannel = 0; nChannel < pGetChannelInfo->nEncChannelChildCount; ++nChannel) {
            Device dev = {pGetChannelInfo->pEncChannelnfo[nChannel].szId, pGetChannelInfo->pEncChannelnfo[nChannel].szName, ""};
            onData(dev);
        }
    }

    return 0;
}

} // namespace dahua
} // namespace sdk
} // namespace sdkproxy