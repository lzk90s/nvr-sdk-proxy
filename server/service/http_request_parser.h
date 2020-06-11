#pragma once

#include <string>
#include "common/helper/logger.h"
#include "common/helper/url_helper.h"
#include "server/rpc/service.pb.h"

#include "3rdsdk/stub/sdk_stub.h"
#include "3rdsdk/stub/sdk_manager.h"

namespace sdkproxy {

class HttpRequestParser {
public:
    const std::string PARAM_IP = "ip";
    const std::string PARAM_USEER = "user";
    const std::string PARAM_PASSWORD = "password";
    const std::string PARAM_CHANNEL_IP = "channelIp";

public:
    HttpRequestParser(brpc::Controller *cntl) {
        cntl_ = cntl;
    }

    void SetResponseError(int status, const std::string &msg) {
        cntl_->http_response().set_status_code(status);
        cntl_->response_attachment().append(msg);
    }

    std::string GetIp() {
        return GetQueryByKey(PARAM_IP);
    }

    std::string GetChannelIp() {
        return GetQueryByKey(PARAM_CHANNEL_IP);
    }

    std::string GetUser() {
        return GetQueryByKey(PARAM_USEER);
    }

    std::string GetPassword() {
        return GetQueryByKey(PARAM_PASSWORD);
    }

    std::string GetQueryByKey(const std::string &key) const {
        const std::string *value = cntl_->http_request().uri().GetQuery(key);
        if (nullptr == value) {
            return "";
        }
        return urlhelper::URLDecode(*value);
    }

    std::shared_ptr<sdk::SdkStub> GetSdkStubByRequest() {
        if (!validateBasicParams()) {
            return nullptr;
        }

        try {
            return sdk::SDK_MNG().TryLoginAndGet(GetIp(), GetUser(), GetPassword());
        } catch (sdk::NetworkException &e) {
            LOG_INFO("TryLoginAndGet error, {}", e.what());
            SetResponseError(brpc::HTTP_STATUS_REQUEST_TIMEOUT, e.what());
            return nullptr;
        } catch (sdk::LoginException &e) {
            LOG_INFO("TryLoginAndGet error, {}", e.what());
            SetResponseError(brpc::HTTP_STATUS_UNAUTHORIZED, e.what());
            return nullptr;
        }
    }

private:
    //是否存在基本参数
    bool validateBasicParams() {
        if (GetQueryByKey(PARAM_USEER).empty() ||
                GetQueryByKey(PARAM_PASSWORD).empty() ||
                GetQueryByKey(PARAM_IP).empty()) {
            SetResponseError(brpc::HTTP_STATUS_BAD_REQUEST, "Invalid arguments");
            return false;
        }
        return true;
    }

private:
    brpc::Controller *cntl_;
};
}