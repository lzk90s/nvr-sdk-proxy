#pragma once

#include "json/json.hpp"
#include "3rdsdk/stub/po_type.h"
#include "3rdsdk/stub/sdk_stub.h"

// json序列化/反序列化

namespace sdkproxy {
namespace sdk {

using json = nlohmann::json;

void to_json(json &j, const RecordInfo &p) {
    j["fileName"]  = p.fileName;
    j["fileSize"]  = p.fileSize;
    j["startTime"] = p.startTime.ToString();
    j["endTime"]   = p.endTime.ToString();
}

void to_json(json &j, const Device &p) {
    j["id"]   = p.id;
    j["name"] = p.name;
    j["ip"]   = p.ip;
}

void to_json(json &j, const FtpInfo &p) {
    j["enable"]   = p.enable;
    j["hostIp"]   = p.hostIp;
    j["hostPort"] = p.hostPort;
    j["dir0"]     = p.dir0;
    j["dir1"]     = p.dir1;
    j["user"]     = p.user;
    j["password"] = p.password;
}

void from_json(const json &j, FtpInfo &p) {
    try {
        p.enable   = j.at("enable").get<bool>();
        p.hostIp   = j.at("hostIp").get<std::string>();
        p.hostPort = j.at("hostPort").get<int>();
        p.user     = j.at("user").get<std::string>();
        p.password = j.at("password").get<std::string>();
        p.dir0     = j.at("dir0").get<std::string>();
        p.dir1     = j.at("dir1").get<std::string>();
    } catch (std::exception e) {
    }
}

void to_json(json &j, const Object &p) {
    j["id"]            = p.id;
    j["type"]          = p.type;
    j["subType"]       = p.subType;
    j["rect"]          = p.rect;
    j["text"]          = p.text;
    j["image"]         = p.image;
    j["startDateTime"] = p.startDateTime;
    j["endDateTime"]   = p.endDateTime;
}

void from_json(const json &j, BaseEvent &p) {}

void to_json(json &j, const IllegalParkingEvent &p) {
    j["id"]            = p.id;
    j["dateTime"]      = p.dateTime;
    j["eventType"]     = p.eventType;
    j["plateNumber"]   = p.plateNumber;
    j["plateColor"]    = p.plateColor;
    j["plateType"]     = p.plateType;
    j["vehicleType"]   = p.vehicleType;
    j["vehicleColor"]  = p.vehicleColor;
    j["vehicleSize"]   = p.vehicleSize;
    j["lane"]          = p.lane;
    j["imageGroupId"]  = p.imageGroupId;
    j["imageCount"]    = p.imageCount;
    j["imageIndex"]    = p.imageIndex;
    j["violationCode"] = p.violationCode;
    j["violationDesc"] = p.violationDesc;
    j["rect"]          = p.rect;
}

void to_json(json &j, const VisitorsCount &p) {
    j["total"] = p.total;
    j["hour"]  = p.hour;
    j["today"] = p.today;
    j["osd"]   = p.osd;
}

void to_json(json &j, const VisitorsFlowRateEvent &p) {
    j["ruleName"] = p.ruleName;
    j["dateTime"] = p.dateTime;
    j["devCode"]  = p.devCode;
    j["inTotal"]  = p.inCount;
    j["outTotal"] = p.outCount;
}

void to_json(json &j, const VehicleCaptureEvent &p) {
    j["plateColor"]   = p.plateColor;
    j["plateNumber"]  = p.plateNumber;
    j["plateType"]    = p.plateType;
    j["vehicleColor"] = p.vehicleColor;
    j["vehicleType"]  = p.vehicleType;
    j["rect"]         = p.rect;
}

void to_json(json &j, const VisitorsFlowRateHistory &p) {
    j["ruleName"]      = p.ruleName;
    j["startDateTime"] = p.startDateTime;
    j["endDateTime"]   = p.endDateTime;
    j["inTotal"]       = p.inTotal;
    j["outTotal"]      = p.outTotal;
    j["avgInside"]     = p.avgInside;
    j["maxInside"]     = p.maxInside;
}

void to_json(json &j, const ObjectLeftEvent &p) {
    j["ruleName"] = p.ruleName;
    j["dateTime"] = p.dateTime;
}

void to_json(json &j, const AcsEvent &p) {
    j["doorNumber"] = p.doorNumber;
    j["doorName"]   = p.doorName;
    j["dateTime"]   = p.dateTime;
    j["type"]       = p.type;
    j["openMethod"] = p.openMethod;
}

void to_json(json &j, const GatherEvent &p) {
    j["action"] = p.action;
}

void to_json(json &j, const GarbageExposureEvent &p) {
    j["action"] = p.action;
}

void to_json(json &j, const AgeRecognizeEvent &p) {
    j["age"] = p.age;
}

} // namespace sdk
} // namespace sdkproxy