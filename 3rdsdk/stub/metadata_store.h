#pragma once

#include <string>
#include <cstdint>
#include <exception>

#include "leveldb/db.h"

namespace sdkproxy {
namespace sdk {

class MetaDataStore {
public:
    MetaDataStore(const std::string &path = "/tmp/sdk_meta") {
        leveldb::Options options;
        options.create_if_missing = true;
        leveldb::Status status    = leveldb::DB::Open(options, path, &db_);
        // if (!status.ok()) {
        //     throw std::runtime_error(status.ToString());
        // }
        assert(status.ok());
    }

    ~MetaDataStore() { delete db_; }

    bool Put(const std::string &key, const std::string &value) { return db_->Put(leveldb::WriteOptions(), key, value).ok(); }

    bool Get(const std::string &key, std::string &value) { return db_->Get(leveldb::ReadOptions(), key, &value).ok(); }

private:
    leveldb::DB *db_;
};

} // namespace sdk
} // namespace sdkproxy