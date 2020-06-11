#pragma once

#include <map>
#include <string>
#include <mutex>
#include <functional>

#include "3rdsdk/stub/po_type.h"

namespace sdkproxy {
namespace sdk {

//过期时间30分钟
const uint64_t EXPIRE_TIME = 1800;

template<typename T>
class Cache {
public:
    typedef struct Value_ {
        std::string key;
        T value;
        uint64_t createTime;
    } Value;

    using QueryFunc = std::function<int32_t(const std::string &key, Value &value)>;

public:
    int Query(const std::string& key, Value& value) {
        std::unique_lock<std::mutex> lck(mutex_);

        if (key.empty() || nullptr == queryFunc_) {
            return -1;
        }

        auto cache_iter = cache_.find(key);
        if (cache_iter != cache_.end()) { /*缓存里面存在值*/
            const Value& value_t = cache_iter->second;
            /*判断缓存是否已过期*/
            if ((time(NULL) - value_t.createTime) < EXPIRE_TIME) {
                /*未过期则返回值*/
                value = value_t;
                return 0;
            }
        }

        /*已过期或不存在缓存中,通过DB去查询需要的Value*/
        int32_t ret = queryFunc_(key, value);
        if (0 != ret) {
            /*DB查询失效*/
            return ret;
        }

        /*设置时间，并放入存储*/
        value.createTime = time(NULL);
        cache_[key] = value;
        return 0;
    }

    void setQueryFunc(QueryFunc q) {
        this->queryFunc_ = q;
    }

private:
    std::mutex mutex_;
    typedef std::map<std::string, Value> MyCache;
    MyCache cache_;
    QueryFunc queryFunc_;
};

}
}