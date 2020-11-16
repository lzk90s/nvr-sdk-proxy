#pragma once

#include <iostream>
#include <string>
#include <chrono>

class CountTimer {
public:
    CountTimer(const std::string &name, int onlyShowOverTimeUs = 0) : name_(name), onlyShowOverTimeUs_(onlyShowOverTimeUs) {
        start_ = std::chrono::steady_clock::now();
    }

    ~CountTimer() {
        end_      = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
        if (diff > onlyShowOverTimeUs_) {
            std::cout << "[" << name_ << "] -> " << readableTime(diff) << std::endl;
        }
    }

private:
    std::string readableTime(long us) {
        float s;
        std::string unit;
        if (us / (1000 * 1000) >= 1) {
            s    = (float)us / (1000 * 1000);
            unit = "s"; //秒
        } else if (us / (1000) >= 1) {
            s    = (float)us / (1000);
            unit = "ms"; //毫秒
        } else if (us >= 1) {
            s    = (float)us;
            unit = "us"; //微秒
        }

        return std::to_string(s) + unit;
    }

    std::string name_;
    int onlyShowOverTimeUs_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
};
