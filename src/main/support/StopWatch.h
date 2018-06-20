
#ifndef SUPPORT_STOPWATCH_H
#define SUPPORT_STOPWATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#include <chrono>
#include <mutex>
#endif

#ifndef __COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER) || defined(__ICL)
#define __COMPILER_BARRIER()        _ReadWriteBarrier()
#else
#define __COMPILER_BARRIER()        asm volatile ("" : : : "memory")
#endif
#endif

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
using namespace std::chrono;
#endif

namespace test {

namespace detail {

template <typename T>
class duration_time {
public:
    typedef T time_float_t;

private:
    time_float_t duration_;

    // 1 second = 1,000 millisec
    static const time_float_t kMillisecCoff = static_cast<time_float_t>(1000.0);
    // 1 second = 1,000,000 microsec
    static const time_float_t kMicrosecCoff = static_cast<time_float_t>(1000000.0);
    // 1 second = 1,000,000,000 nanosec
    static const time_float_t kNanosecCoff = static_cast<time_float_t>(1000000000.0);

public:
    duration_time(time_float_t duration) : duration_(duration) {}
    duration_time(const duration_time<T> & src) : duration_(src.duration_) {}
    ~duration_time() {}

    time_float_t seconds() const {
        return this->duration_;
    }

    time_float_t millisecs() const {
        return (this->seconds() * kMillisecCoff);
    }

    time_float_t microsecs() const {
        return (this->seconds() * kMicrosecCoff);
    }

    time_float_t nanosecs() const {
        return (this->seconds() * kNanosecCoff);
    }
};

} // namespace detail

template <typename T>
class StopWatchBase {
public:
    typedef T                                   impl_type;
    typedef typename impl_type::time_float_t    time_float_t;
    typedef typename impl_type::time_stamp_t    time_stamp_t;
    typedef typename impl_type::time_point_t    time_point_t;
    typedef typename impl_type::duration_type   duration_type;

private:
    time_point_t start_time_;
    time_point_t stop_time_;

    static time_point_t base_time_;

    // 1 second = 1000 millisecond
    static time_float_t kMillisecCoff;      // static_cast<time_float_t>(1000.0);

public:
    StopWatchBase() :
        start_time_(impl_type::now()), stop_time_(start_time_) {
    }
    StopWatchBase(StopWatchBase<T> const & src) :
        start_time_(src.start_time_), stop_time_(src.stop_time_) {
    }
    ~StopWatchBase() {};

    void reset() {
        __COMPILER_BARRIER();
        start_time_ = impl_type::now();
        stop_time_ = start_time_;
        __COMPILER_BARRIER();
    }

    void start() {
        __COMPILER_BARRIER();
        start_time_ = impl_type::now();
        __COMPILER_BARRIER();
    }

    void stop() {
        __COMPILER_BARRIER();
        stop_time_ = impl_type::now();
        __COMPILER_BARRIER();
    }

    static time_point_t now() {
        __COMPILER_BARRIER();
        time_point_t _now = impl_type::now();
        __COMPILER_BARRIER();
        return _now;
    }

    static time_stamp_t timestamp() {
        __COMPILER_BARRIER();
        time_stamp_t _timestamp = impl_type::timestamp(impl_type::now(), base_time_);
        __COMPILER_BARRIER();
        return _timestamp;
    }

    template <typename U>
    static detail::duration_time<time_float_t> duration(U now, U old) {
        __COMPILER_BARRIER();
        detail::duration_time<time_float_t> _duration(impl_type::duration_time(now, old));
        __COMPILER_BARRIER();
        return _duration;
    }

    time_float_t peekSecond() const {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(impl_type::now(), start_time_);
        __COMPILER_BARRIER();
        return elapsed_time;
    }

    time_float_t peekMillisec() const {
        return (this->peekSecond() * kMillisecCoff);
    }

    time_float_t getSecond() {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(stop_time_, start_time_);
        __COMPILER_BARRIER();
        return elapsed_time;
    }

    time_float_t getMillisec() {
        return (this->getSecond() * kMillisecCoff);
    }
};

template <typename T>
typename StopWatchBase<T>::time_point_t
StopWatchBase<T>::base_time_ = StopWatchBase<T>::impl_type::now();

// 1 second = 1000 millisec
template <typename T>
typename StopWatchBase<T>::time_float_t
StopWatchBase<T>::kMillisecCoff = static_cast<typename StopWatchBase<T>::time_float_t>(1000.0);

template <typename T>
class StopWatchExBase {
public:
    typedef T                                   impl_type;
    typedef StopWatchExBase<T>                  this_type;
    typedef typename impl_type::time_float_t    time_float_t;
    typedef typename impl_type::time_stamp_t    time_stamp_t;
    typedef typename impl_type::time_point_t    time_point_t;
    typedef typename impl_type::duration_type   duration_type;

private:
    time_point_t start_time_;
    time_point_t stop_time_;
    time_float_t elapsed_time_;
    time_float_t total_elapsed_time_;
    bool         running_;

    static time_point_t base_time_;

    // The zero value time.
    static time_float_t kTimeZero;      // static_cast<time_float_t>(0.0);
    // 1 second = 1000 millisec
    static time_float_t kMillisecCoff;  // static_cast<time_float_t>(1000.0);

public:
    StopWatchExBase() : elapsed_time_(kTimeZero),
        total_elapsed_time_(kTimeZero), running_(false) {
        start_time_ = impl_type::now();
    }
    StopWatchExBase(StopWatchExBase<T> const & src) :
        start_time_(src.start_time_), stop_time_(src.stop_time_),
        elapsed_time_(src.elapsed_time_), total_elapsed_time_(src.total_elapsed_time_),
        running_(src.running_) {
    }
    ~StopWatchExBase() {}

    void reset() {
        __COMPILER_BARRIER();
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = impl_type::now();
        running_ = false;
        __COMPILER_BARRIER();
    }

    void restart() {
        __COMPILER_BARRIER();
        running_ = false;
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = impl_type::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = kTimeZero;
            start_time_ = impl_type::now();
            running_ = true;
        }
        __COMPILER_BARRIER();
    }

    void stop() {
        __COMPILER_BARRIER();
        if (running_) {
            stop_time_ = impl_type::now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getDurationTime();
        }
    }

    void mark_start() {
        start_time_ = impl_type::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = impl_type::now();
        running_ = false;
    }

    void resume() {
        this->start();
    }

    void pause() {
        __COMPILER_BARRIER();
        if (running_) {
            stop_time_ = impl_type::now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getDurationTime();
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

    static time_point_t now() {
        __COMPILER_BARRIER();
        time_point_t _now = impl_type::now();
        __COMPILER_BARRIER();
        return _now;
    }

    static time_stamp_t timestamp() {
        __COMPILER_BARRIER();
        time_stamp_t _timestamp = impl_type::timestamp(impl_type::now(), this_type::base_time_);
        __COMPILER_BARRIER();
        return _timestamp;
    }

    template <typename U>
    static detail::duration_time<time_float_t> duration(U now, U old) {
        __COMPILER_BARRIER();
        detail::duration_time<time_float_t> _duration(impl_type::duration_time(now, old));
        __COMPILER_BARRIER();
        return _duration;
    }

    time_float_t getDurationTime() const {
        __COMPILER_BARRIER();
        detail::duration_time<time_float_t> _duration_time = impl_type::duration_time(stop_time_, start_time_);
        __COMPILER_BARRIER();
        return _duration_time;
    }

    time_float_t getDurationSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            this->elapsed_time_ = this->getDurationTime();
        }
        return this->elapsed_time_;
    }

    time_float_t getDurationMillisec() {
        return (this->getDurationSecond() * kMillisecCoff);
    }

    time_float_t peekSecond() const {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(impl_type::now(), start_time_);
        __COMPILER_BARRIER();
        return elapsed_time;
    }

    time_float_t peekMillisec() const {
        return (this->peekSecond() * kMillisecCoff);
    }

    time_float_t getSecond() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        return this->elapsed_time_;
    }

    time_float_t getMillisec() {
        return (this->getSecond() * kMillisecCoff);
    }

    time_float_t getTotalSecond() const {
        __COMPILER_BARRIER();
        return this->total_elapsed_time_;
    }

    time_float_t getTotalMillisec() const {
        __COMPILER_BARRIER();
        return (this->total_elapsed_time_ * kMillisecCoff);
    }
};

template <typename T>
typename StopWatchExBase<T>::time_point_t
StopWatchExBase<T>::base_time_ = StopWatchExBase<T>::impl_type::now();

// The zero value time.
template <typename T>
typename StopWatchExBase<T>::time_float_t
StopWatchExBase<T>::kTimeZero = static_cast<typename StopWatchExBase<T>::time_float_t>(0.0);

// 1 second = 1000 millisec
template <typename T>
typename StopWatchExBase<T>::time_float_t
StopWatchExBase<T>::kMillisecCoff = static_cast<typename StopWatchExBase<T>::time_float_t>(1000.0);

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)

template <typename TimeFloatTy>
class StdStopWatchImpl {
public:
    typedef TimeFloatTy                                     time_float_t;
    typedef double                                          time_stamp_t;
    typedef std::chrono::time_point<high_resolution_clock>  time_point_t;
    typedef std::chrono::duration<time_stamp_t>             duration_type;

public:
    StdStopWatchImpl() {}
    ~StdStopWatchImpl() {}

    static time_point_t now() {
        return std::chrono::high_resolution_clock::now();
    }

    static time_float_t duration_time(time_stamp_t now_time, time_stamp_t old_time) {
        return static_cast<time_float_t>(now_time - old_time);
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        duration_type _duration_time = std::chrono::duration_cast<duration_type>(now_time - old_time);
        return _duration_time.count();
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return static_cast<time_stamp_t>(duration_time(now_time, base_time));
    }
};

typedef StopWatchBase< StdStopWatchImpl<double> >       StopWatch;
typedef StopWatchExBase< StdStopWatchImpl<double> >     StopWatchEx;

#endif // (_MSC_VER >= 1800)

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <typename TimeFloatTy>
class timeGetTimeImpl {
public:
    typedef TimeFloatTy     time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    timeGetTimeImpl() {}
    ~timeGetTimeImpl() {}

    static time_point_t now() {
        return ::timeGetTime();
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        time_point_t _duration_time = now_time - old_time;
        return (static_cast<time_float_t>(_duration_time) / static_cast<time_float_t>(1000));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase< timeGetTimeImpl<double> >    timeGetTimeStopWatch;
typedef StopWatchExBase< timeGetTimeImpl<double> >  timeGetTimeStopWatchEx;

#else

typedef StopWatchBase< StdStopWatchImpl<double> >   timeGetTimeStopWatch;
typedef StopWatchExBase< StdStopWatchImpl<double> > timeGetTimeStopWatchEx;

#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <typename TimeFloatTy>
class getTickCountImpl {
public:
    typedef TimeFloatTy     time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    getTickCountImpl() {}
    ~getTickCountImpl() {}

    static time_point_t now() {
        return ::GetTickCount();
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        time_point_t _duration_time = now_time - old_time;
        return (static_cast<time_float_t>(_duration_time) / static_cast<time_float_t>(1000));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase< getTickCountImpl<double> >   getTickCountStopWatch;
typedef StopWatchExBase< getTickCountImpl<double> > getTickCountStopWatchEx;

#else

typedef StopWatchBase< StdStopWatchImpl<double> >   getTickCountStopWatch;
typedef StopWatchExBase< StdStopWatchImpl<double> > getTickCountStopWatchEx;

#endif // _WIN32

#if defined(_MSC_VER) && (_MSC_VER < 1800)

typedef StopWatchBase< timeGetTimeImpl<double> >    StopWatch;
typedef StopWatchExBase< timeGetTimeImpl<double> >  StopWatchEx;

#endif

#undef __COMPILER_BARRIER

} // namespace test

#endif // SUPPORT_STOPWATCH_H
