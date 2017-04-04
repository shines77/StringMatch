
#ifndef JIMI_STRING_MATCH_H
#define JIMI_STRING_MATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

//
// marco __CountOf(array)
//
#if defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
    #define StringMatch_Unaligned __unaligned
#else
    #define StringMatch_Unaligned
#endif

#ifdef __cplusplus
    extern "C++" {
        template <typename CountOfType, size_t SizeOfArray>
        char(*__CountOf_Helper(StringMatch_Unaligned CountOfType(&_Array)[SizeOfArray]))[SizeOfArray];

        #define __CountOf(_Array) (sizeof(*__CountOf_Helper(_Array)) + 0)
    }
#else
    #define __CountOf(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace StringMatch {

class StringRef {
private:
    const char * data_;
    size_t length_;

public:
    StringRef() : data_(nullptr), length_(0) {
        // Do nothing!
    }
    StringRef(const char * data)
        : data_(data), length_(strlen(data)) {
    }
    StringRef(const char * data, size_t length)
        : data_(data), length_(length) {
    }
    template <size_t N>
    StringRef(const char(&data)[N])
        : data_(data), length_(length) {
        return prepare(pattern, N);
    }
    StringRef(const std::string & src)
        : data_(src.c_str()), length_(src.size()) {
    }
    StringRef(const StringRef & src)
        : data_(src.c_str()), length_(src.size()) {
    }
    ~StringRef() {}

    const char * c_str() const { return data_; }
    char * data() const { return const_cast<char *>(data_); }

    size_t size() const { return length_; }
    size_t length() const { return this->size(); }

    void set_ref(const char * data, size_t length) {
        data_ = data;
        length_ = length;
    }

    void set_ref(const char * data) {
        set_ref(data, strlen(data));
    }

    template <size_t N>
    void set_ref(const char(&data)[N]) {
        set_ref(data, N);
    }

    void set_ref(const std::string & data) {
        set_ref(data.c_str(), data.size());
    }
};

} // namespace StringMatch

#endif // JIMI_STRING_MATCH_H
