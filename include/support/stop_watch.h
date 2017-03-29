#pragma once

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32
#include <chrono>
#include <mutex>

#ifndef __COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER) || defined(__ICL)
#define __COMPILER_BARRIER()		_ReadWriteBarrier()
#else
#define __COMPILER_BARRIER()		asm volatile ("" : : : "memory")
#endif
#endif

using namespace std::chrono;

namespace jimi {
namespace detail {

template <typename T>
class duration_time {
public:
    typedef T time_float_t;

    // 1 second = 1,000 millisec
    const time_float_t kMillisecCoff = static_cast<time_float_t>(1000.0);
    // 1 second = 1,000,000 microsec
    const time_float_t kMicrosecCoff = static_cast<time_float_t>(1000000.0);
    // 1 second = 1,000,000,000 nanosec
    const time_float_t kNanosecCoff = static_cast<time_float_t>(1000000.0);

private:
    time_float_t duration_;

public:
    duration_time(time_float_t duration) : duration_(duration) {}
    ~duration_time() {}

    time_float_t seconds() const {
        return duration_;
    }

    time_float_t millisecs() const {
        return seconds() * kMillisecCoff;
    }

    time_float_t microsecs() const {
        return seconds() * kMicrosecCoff;
    }

    time_float_t nanosecs() const {
        return seconds() * kNanosecCoff;
    }
};

} // namespace detail

template <typename T>
class StopWatchBase {
public:
    typedef T                                       implement_type;
    typedef typename implement_type::time_float_t   time_float_t;
    typedef typename implement_type::time_stamp_t   time_stamp_t;
    typedef typename implement_type::time_point_t   time_point_t;
    typedef typename implement_type::duration_type  duration_type;

    // The zero value time.
    const time_float_t kTimeZero = static_cast<time_float_t>(0.0);
    // 1 second = 1000 millisec
    const time_float_t kMillisecCoff = static_cast<time_float_t>(1000.0);

private:
    time_point_t start_time_;
    time_point_t stop_time_;
    time_float_t elapsed_time_;
    time_float_t total_elapsed_time_;
    bool         running_;

    static time_point_t base_time_;

public:
    StopWatchBase() : elapsed_time_(kTimeZero),
        total_elapsed_time_(kTimeZero), running_(false) {
        start_time_ = implement_type::get_timepoint_now();
    };
    ~StopWatchBase() {};

	void reset() {
        __COMPILER_BARRIER();
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = implement_type::get_timepoint_now();
        running_ = false;
        __COMPILER_BARRIER();
	}

    void restart() {
        __COMPILER_BARRIER();
        running_ = false;
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = implement_type::get_timepoint_now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = kTimeZero;
            start_time_ = implement_type::get_timepoint_now();
            running_ = true;
        }
        __COMPILER_BARRIER();
    }

    void stop() {
		__COMPILER_BARRIER();
        if (running_) {
            stop_time_ = implement_type::get_timepoint_now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getIntervalTime();
        }
    }

    void mark_start() {
        start_time_ = implement_type::get_timepoint_now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = implement_type::get_timepoint_now();
        running_ = false;
    }

    void continues() {
        this->start();
    }

	void pause() {
        __COMPILER_BARRIER();
        if (running_) {
            stop_time_ = implement_type::get_timepoint_now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getIntervalTime();
            total_elapsed_time_ += elapsed_time_;
        }
        __COMPILER_BARRIER();
	}

    void again() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        if (elapsed_time_ != kTimeZero) {
            total_elapsed_time_ += elapsed_time_;
            elapsed_time_ = kTimeZero;
        }
    }

    static time_stamp_t now() {
        __COMPILER_BARRIER();
        time_stamp_t _now = implement_type::get_timestamp_now(implement_type::get_timepoint_now(), base_time_);
        __COMPILER_BARRIER();
        return _now;
    }

    static time_point_t timepoint_now() {
        __COMPILER_BARRIER();
        return implement_type::get_timepoint_now();
    }

    template <typename U>
    static detail::duration_time<time_float_t> duration(U now, U old) {
        __COMPILER_BARRIER();
        detail::duration_time<time_float_t> _duration(implement_type::get_interval_time(now, old));
        return _duration;
    }

    time_float_t getIntervalTime() const {
        __COMPILER_BARRIER();
        return implement_type::get_interval_time(stop_time_, start_time_);
    }

    time_float_t getIntervalSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            this->elapsed_time_ = this->getIntervalTime();
        }
        return this->elapsed_time_;
    }

    time_float_t getIntervalMillisec() {
        return this->getIntervalSecond() * kMillisecCoff;
    }

    time_float_t peekElapsedSecond() const {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = implement_type::get_interval_time(implement_type::get_timepoint_now(), start_time_);
        __COMPILER_BARRIER();
        return elapsed_time;
    }

    time_float_t peekElapsedMillisec() const {
        return this->peekElapsedSecond() * kMillisecCoff;
    }

    time_float_t getElapsedSecond() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        return this->elapsed_time_;
    }

    time_float_t getElapsedMillisec() {
        return this->getElapsedSecond() * kMillisecCoff;
    }

    time_float_t getTotalSecond() const {
        __COMPILER_BARRIER();
        return this->total_elapsed_time_;
    }

    time_float_t getTotalMillisec() const {
        __COMPILER_BARRIER();
        return this->total_elapsed_time_ * kMillisecCoff;
    }
};

template <typename T>
typename StopWatchBase<T>::time_point_t
StopWatchBase<T>::base_time_ = StopWatchBase<T>::implement_type::get_timepoint_now();

template <typename TimeFloatType>
class StdStopWatchImpl {
public:
    typedef TimeFloatType                                   time_float_t;
    typedef double                                          time_stamp_t;
    typedef std::chrono::time_point<high_resolution_clock>  time_point_t;
    typedef std::chrono::duration<time_stamp_t>             duration_type;

public:
    StdStopWatchImpl() {}
    ~StdStopWatchImpl() {}

    static time_point_t get_timepoint_now() {
        return std::chrono::high_resolution_clock::now();
    }

    static time_float_t get_interval_time(time_stamp_t now_time, time_stamp_t old_time) {
        return static_cast<time_float_t>(now_time - old_time);
    }

    static time_float_t get_interval_time(time_point_t now_time, time_point_t old_time) {
        duration_type interval_time = std::chrono::duration_cast<duration_type>(now_time - old_time);
        return interval_time.count();
    }

    static time_stamp_t get_timestamp_now(time_point_t now_time, time_point_t base_time) {
        return static_cast<time_stamp_t>(get_interval_time(now_time, base_time));
    }
};

typedef StopWatchBase<StdStopWatchImpl<double>> StopWatch;

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <typename TimeFloatType>
class timeGetTimeStopWatchImpl {
public:
    typedef TimeFloatType   time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    timeGetTimeStopWatchImpl() {}
    ~timeGetTimeStopWatchImpl() {}

    static time_point_t get_timepoint_now() {
        return ::timeGetTime();
    }

    static time_float_t get_interval_time(time_point_t now_time, time_point_t old_time) {
        time_point_t interval_time = now_time - old_time;
        return (static_cast<time_float_t>(interval_time) / static_cast<time_float_t>(1000));
    }

    static time_stamp_t get_timestamp_now(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase<timeGetTimeStopWatchImpl<double>> timeGetTimeStopWatch;

#else

typedef StopWatchBase<StdStopWatchImpl<double>> timeGetTimeStopWatch;

#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <typename TimeFloatType>
class getTickCountStopWatchImpl {
public:
    typedef TimeFloatType   time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    getTickCountStopWatchImpl() {}
    ~getTickCountStopWatchImpl() {}

    static time_point_t get_timepoint_now() {
        return ::GetTickCount();
    }

    static time_float_t get_interval_time(time_point_t now_time, time_point_t old_time) {
        time_point_t interval_time = now_time - old_time;
        return (static_cast<time_float_t>(interval_time) / static_cast<time_float_t>(1000));
    }

    static time_stamp_t get_timestamp_now(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase<getTickCountStopWatchImpl<double>> getTickCountStopWatch;

#else

typedef StopWatchBase<StdStopWatchImpl<double>> getTickCountStopWatch;

#endif // _WIN32

#undef __COMPILER_BARRIER

} // namespace jimi
