#pragma once

#include <chrono>
#include <glog/logging.h>

class Timer {
    private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    public:

    void start();
    void stop();
};