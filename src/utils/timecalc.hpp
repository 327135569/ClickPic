#pragma once

#include <chrono>
#include <stdio.h>
#include <string>

class TimeCalc {

  public:
    TimeCalc(const char *tag) {
        this->tag = tag;
        begin = clock.now().time_since_epoch().count();
    }
    ~TimeCalc() {
        if (mfinish) {
            return;
        }
        end = clock.now().time_since_epoch().count();
        printf("%s %lldms", tag, (end - begin) / 1000000);
    }

    std::string finish() {
        mfinish = true;
        end = clock.now().time_since_epoch().count();
        char buff[200];
        sprintf(buff, "%s %lldms", tag, (end - begin) / 1000000);
        return buff;
    }

  private:
    std::chrono::high_resolution_clock clock;
    const char *tag;
    long long begin;
    long long end;
    bool mfinish = false;
};