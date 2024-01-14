#include <timer.hpp>

void Timer::start() {
    this->start_time = std::chrono::high_resolution_clock::now();
}

void Timer::stop() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration<double, std::micro>(end_time - start_time).count();
    LOG(WARNING) << "Timer Duration: " << dur << "µs";
}