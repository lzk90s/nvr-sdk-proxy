#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>

#include "json/json.hpp"

namespace sdkproxy {
namespace sdk {

typedef struct tagTimePoint {
    int32_t year, month, day, hour, minute, second;
    const char* TIME_STRING_FORMAT = "%04d-%02d-%02d %02d:%02d:%02d";
    const char* TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

    tagTimePoint() {
        year = month = day = hour = minute = second = 0;
    }

    const std::string ToString() const {
        char t[128] = { 0 };
        snprintf(t, sizeof(t) - 1, TIME_STRING_FORMAT, year, month, day, hour, minute, second);
        return std::string(t);
    }

    struct tagTimePoint & FromString(const std::string &s) {
        sscanf(s.c_str(), TIME_STRING_FORMAT, &year, &month, &day, &hour, &minute, &second);
        return *this;
    }

    const time_t ToTime() const {
        time_t t;
        tm tm;
        strptime(ToString().c_str(), TIME_FORMAT, &tm); //将字符串转换为tm时间
        t = mktime(&tm); //将tm时间转换为秒时间
        return t;
    }

    const void FromTime(time_t t) {
        char timestr[25] = { 0 };
        struct tm local_time;
        localtime_r(&t, &local_time);
        strftime(timestr, 22, TIME_FORMAT, &local_time);
        FromString(std::string(timestr));
    }

} TimePoint;

typedef struct tagRecordInfo {
    std::string     fileName;
    uint32_t        fileSize;
    TimePoint       startTime;
    TimePoint       endTime;
} RecordInfo;

typedef struct tagDevice {
    std::string id;
    std::string name;
    std::string ip;
} Device;

typedef struct tagFtpInfo {
    bool enable;
    std::string hostIp;
    int hostPort;
    std::string user;
    std::string password;
    std::string dir0;   //一级目录
    std::string dir1;   //二级目录
} FtpInfo;

enum AlarmEventType {
    //非法停车，对应IllegalParkingInfo
    ILLEGAL_PARKING = 0,
    //交通抓拍结果，对应VehicleCaptureEvent
    VEHICLE_CAPTURE,
    //人流量，对应VisitorsFlowRateEvent
    VISITORS_FLOWRATE,
    //物品遗留
    OBJECT_LEFT,
    //道闸出入记录
    BARRIER_GATE,
    //门禁出入记录
    DOOR,
};

typedef struct tagThirdPartyEventObject {
    //事件类型
    AlarmEventType type;
    //事件数据，json
    std::string data;
} ThirdPartyEventObject;

typedef struct tagBaseEvent {
} BaseEvent;

typedef struct tagRect {
    int32_t x, y, w, h;

    tagRect() {
        x = y = w = h = 0;
    }
} Rect;

typedef struct tagObject {
    //id
    std::string id;
    //类型
    std::string type;
    //子类型
    std::string subType;
    //矩形区域[x,y,w,h]
    std::vector<int32_t> rect;
    //文本信息
    std::string text;
    //图片二进制流
    std::vector<uint8_t> image;
    //出现时间
    std::string startDateTime;
    //消失时间
    std::string endDateTime;
} Object;

//违停事件
typedef struct tagIllegalParkingEvent : public BaseEvent {
    //ID
    std::string id;
    //时间
    std::string dateTime;
    //类型
    std::string eventType;
    //车牌号
    std::string plateNumber;
    //车牌颜色
    std::string plateColor;
    //车牌类型
    std::string plateType;
    //车辆类型
    std::string vehicleType;
    //车辆颜色
    std::string vehicleColor;
    //车辆大小
    std::string vehicleSize;
    //车道号
    std::string lane;
    //图片组id
    std::string imageGroupId;
    //一组图片数量
    uint32_t imageCount;
    //图片在组中的索引
    uint32_t imageIndex;
    //违法代码
    std::string violationCode;
    //违法描述
    std::string violationDesc;
    //矩形
    std::vector<int32_t> rect;
} IllegalParkingEvent;

typedef struct tagVehicleCaptureEvent : public BaseEvent {
    std::string plateNumber;
    std::string plateType;
    std::string plateColor;
    std::string vehicleColor;
    std::string vehicleType;
    std::vector<int32_t> rect;
} VehicleCaptureEvent;

//客流量统计
typedef struct tagVisitorsCount {
    // 设备运行后人数统计总数
    int32_t total;
    // 小时内的总人数
    int32_t hour;
    // 当天的总人数
    int32_t today;
    //
    int32_t osd;

    tagVisitorsCount() {
        total = 0;
        hour = 0;
        today = 0;
        osd = 0;
    }
} VisitorsCount;

//客流量统计事件
typedef struct tagVisitorsFlowRateEvent : public BaseEvent {
    // 对应的规则名
    std::string ruleName;
    // 设备编码
    std::string devCode;
    // 统计时间
    std::string dateTime;
    // 进统计
    VisitorsCount inCount;
    // 出统计
    VisitorsCount outCount;
} VisitorsFlowRateEvent;

//客流量历史
typedef struct tagVisitorsFlowRateHistory {
    // 规则名字
    std::string ruleName;
    // 开始时间
    std::string startDateTime;
    // 结束时间
    std::string endDateTime;
    // 进入总数
    int32_t inTotal;
    // 出来总数
    int32_t outTotal;
    // 平均保有人数
    int32_t avgInside;
    // 最大保有人数
    int32_t maxInside;

    tagVisitorsFlowRateHistory() {
        inTotal = 0;
        outTotal = 0;
        avgInside = 0;
        maxInside = 0;
    }
} VisitorsFlowRateHistory;

typedef struct tagPersonNumEvent : public BaseEvent {
    std::string ruleName;
    std::string dateTime;
    std::string eventId;
    uint32_t number;
    uint32_t upperLimit;
    //图片组id
    std::string imageGroupId;
    //一组图片数量
    uint32_t imageCount;
    //图片在组中的索引
    uint32_t imageIndex;
    uint32_t inNumber;
    uint32_t outNumber;
} PersonNumEvent;

//物品遗留事件
typedef struct tagObjectLeftEvent : public BaseEvent {
    std::string ruleName;
    std::string dateTime;
} ObjectLeftEvent;

//门禁事件
typedef struct tagAcsEvent : public BaseEvent {
    //门禁通道号
    int32_t doorNumber;
    //门禁名
    std::string doorName;
    //时间
    std::string dateTime;
    //类型，0，出去；1：进入
    int32_t type;
    //开门方式
    std::string openMethod;
} AcsEvent;

}
}