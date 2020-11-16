#pragma once

// from https://blog.csdn.net/f110300641/article/details/82147885

#include <iostream>
#include <chrono>
#include <thread>

#include <time.h>
#include <sys/time.h>

class TimeTool {
public:
    static uint32_t elapse_seconds(std::chrono::system_clock::time_point &start) {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count();
    }

    static uint64_t elapse_ms(std::chrono::system_clock::time_point &start) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    }

    static uint32_t now_to_seconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static uint64_t now_to_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static uint64_t to_ms(const std::chrono::system_clock::time_point &time) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
    }

    static std::string to_str(const std::chrono::system_clock::time_point &time) {
        char _time[25] = {0};
        time_t tt      = std::chrono::system_clock::to_time_t(time);
        struct tm local_time;
        localtime_r(&tt, &local_time);
        strftime(_time, 22, "%Y-%m-%d %H:%M:%S", &local_time);

        return std::string(_time);
    }

    //"%d-%02d-%02d %02d:%02d:%02d.%03d"
    static std::string to_str_ex(const std::chrono::system_clock::time_point &time) {
        uint64_t mill = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count()
                        - std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count() * 1000;

        char _time[25] = {0};
        time_t tt      = std::chrono::system_clock::to_time_t(time);
        struct tm local_time;
        localtime_r(&tt, &local_time);
        // strftime(_time, 22, "%Y-%m-%d %H:%M:%S", local_time);
        sprintf(_time, "%d-%02d-%02d %02d:%02d:%02d.%03d", local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour,
                local_time.tm_min, local_time.tm_sec, mill);

        return std::string(_time);
    }

    static uint64_t now_to_ms_ex() {
        struct timeval tv;
        ::gettimeofday(&tv, 0);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    //"%Y-%m-%d %H:%M:%S"
    static time_t to_time(std::string str) {
        time_t t_;
        tm tm_;
        strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm_); //将字符串转换为tm时间
        t_ = mktime(&tm_);                                //将tm时间转换为秒时间

        return t_;
    }

    ////"%d-%02d-%02d %02d:%02d:%02d.%03d"
    static std::chrono::system_clock::time_point to_time_ex(std::string str) {
        int pos = str.length() - 3;

        time_t t_;
        tm tm_;
        strptime(str.substr(0, pos).c_str(), "%Y-%m-%d %H:%M:%S", &tm_); //将字符串转换为tm时间
        t_ = mktime(&tm_);                                               //将tm时间转换为秒时间

        int milli = std::stoi(str.substr(pos));

        return std::chrono::system_clock::time_point(std::chrono::milliseconds(t_ * 1000 + milli));
    }

    static std::string to_str_ex(uint64_t ms) {
        uint64_t mill = ms % 1000;

        char _time[25]        = {0};
        time_t tt             = ms / 1000;
        struct tm *local_time = localtime(&tt);
        // strftime(_time, 22, "%Y-%m-%d %H:%M:%S", local_time);
        sprintf(_time, "%d-%02d-%02d %02d:%02d:%02d.%03d", local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
                local_time->tm_hour, local_time->tm_min, local_time->tm_sec, mill);

        return std::string(_time);
    }

    static uint32_t elapse_seconds(time_t &start) {
        time_t now;
        time(&now);
        return static_cast<uint32_t>(std::difftime(now, start));
    }

    //"%Y-%m-%d %H:%M:%S"
    static std::string to_str(const std::time_t &time) {
        char _time[25] = {0};
        struct tm local_time;
        localtime_r(&time, &local_time);
        strftime(_time, 22, "%Y-%m-%d %H:%M:%S", &local_time);

        return std::string(_time);
    }
};