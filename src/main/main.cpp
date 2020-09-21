
#ifdef _DEBUG
//#include <vld.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#ifndef __cplusplus
#include <stdalign.h>   // C11 defines _Alignas().  This header defines alignas()
#endif


#define TEST_ALL_BENCHMARK          1
#define USE_ALIGNED_PATTAEN         1

#define SWITCH_BENCHMARK_TEST       0
#define ENABLE_AHOCORASICK_TEST     0

#include "StringMatch.h"
#include "support/StopWatch.h"

#include "algorithm/StrStr.h"
#include "algorithm/MemMem.h"
#include "algorithm/MyStrStr.h"
#include "algorithm/GlibcStrStr.h"
#include "algorithm/GlibcStrStrOld.h"
#include "algorithm/SSEStrStr.h"
#include "algorithm/SSEStrStr2.h"
#include "algorithm/SSEStrStrA.h"
#include "algorithm/SSEStrStrA_v0.h"
#include "algorithm/SSEStrStrA_v2.h"
#include "algorithm/MyMemMem.h"
#include "algorithm/MyMemMemBw.h"
#include "algorithm/FastStrStr.h"
#include "algorithm/StdSearch.h"
#include "algorithm/StdBoyerMoore.h"
#include "algorithm/Kmp.h"
#include "algorithm/KmpStd.h"
#include "algorithm/BoyerMoore.h"
#include "algorithm/Sunday.h"
#include "algorithm/Horspool.h"
#include "algorithm/QuickSearch.h"
#include "algorithm/BMTuned.h"
#include "algorithm/ShiftAnd.h"
#include "algorithm/ShiftOr.h"
#include "algorithm/WordHash.h"
#include "algorithm/Volnitsky.h"
#include "algorithm/Rabin-Karp.h"
#include "algorithm/AhoCorasick.h"

using namespace StringMatch;

#if defined(NDEBUG)
static const size_t kIterations = 2000000;
#else
static const size_t kIterations = 1000;
#endif

//
// EXACT STRING MATCHING ALGORITHMS (Animation in Java)
//
// See: http://www-igm.univ-mlv.fr/~lecroq/string/
//

//
// See: http://volnitsky.com/project/str_search/index.html
// See: https://github.com/ox/Volnitsky-ruby/blob/master/volnitsky.cc
//
static const char * SearchTexts[] = {
    "Here is a sample example.",

    "8'E . It consists of a number of low-lying, largely mangrove "
    "covered islands covering an area of around 665 km^2. "
    "The population of Bakassi is theTsubject of some dispute, "
    "but is generally put at between 150,000 and 300,000 people."

    "This historical depiction of the coat of arms of California was illustrated "
    "by American engraver "
    "Henry Mitchell in State Arms of the Union, published in 1876 by Louis Prang. "
    "A state in the Pacific region of the U.S., California was admitted into the "
    "Union on September 9, 1850. The escutcheon depicts the goddess Minerva, "
    "representing the political birth of the state, seated underneath the state "
    "motto Eureka. At her feet crouches a grizzly bear, feeding upon bunches of grapes that, "
    "with the plough and sheaf of wheat on the right, are emblematic of the state's bounty. "
    "On the left, a miner at work, with a rocker and bowl at his side, illustrates the golden "
    "wealth of the land, while the snow-clad peaks of the Sierra Nevada and shipping on the "
    "Sacramento River make up the background. A similar design appears on the current Great "
    "Seal of California."
};

static const char * Patterns[] = {
    "sample",
    "example",

    "islands",
    "around",
    "subject",
    "between",
    "people",

    "love you",
    "I love you",

    ///*
    "largely mangrove",
    "largely Mangrove",
    "some dispute",
    "some dicpute",
    "The population of Bakassi",
    "The population of bakassi",

    "between 150,000",
    "between 150,0000",
    "between 150,000 and 300,000 people.",

    "River",
    "Sacramento River",
    "grizzly bear",
    "California",
    "Henry Mitchell",
    "the Sierra Nevada"
    //*/
};

static const size_t kSearchTexts = sm_countof_i(SearchTexts);
static const size_t kPatterns = sm_countof_i(Patterns);

static const int kWarmupMillsecs = 600;

void cpu_warmup(int delayTime)
{
#if defined(NDEBUG)
    double startTime, stopTime;
    double delayTimeLimit = (double)delayTime / 1000.0;
    volatile int sum = 0;

    printf("CPU warm-up begin ...\n");
    fflush(stdout);
    startTime = test::StopWatch::timestamp();
    double elapsedTime;
    do {
        for (int i = 0; i < 500; ++i) {
            sum += i;
            for (int j = 5000; j >= 0; --j) {
                sum -= j;
            }
        }
        stopTime = test::StopWatch::timestamp();
        elapsedTime = stopTime - startTime;
    } while (elapsedTime < delayTimeLimit);

    printf("sum = %u, time: %0.3f ms\n", sum, elapsedTime * 1000.0);
    printf("CPU warm-up end   ... \n\n");
    fflush(stdout);
#endif // !_DEBUG
}

void StringMatch_usage_examples()
{
    // Usage 1
    {
        AnsiString::Kmp::Pattern pattern("example");
        if (pattern.has_compiled()) {
            Long pos = pattern.match("Here is a sample example.");
        }
    }

    // Usage 2
    {
        AnsiString::Kmp::Pattern pattern;
        bool compiled = pattern.preprocessing("example");
        if (compiled) {
            Long pos = pattern.match("Here is a sample example.");
        }
    }

    // Usage 3
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher("Here is a sample example.");
        if (pattern.has_compiled()) {
            Long pos = matcher.find(pattern);
        }
    }

    // Usage 4
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher;
        matcher.set_text("Here is a sample example.");
        if (pattern.has_compiled()) {
            Long pos = matcher.find(pattern);
        }
    }

    // Usage 5
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher;
        if (pattern.has_compiled()) {
            Long pos = matcher.find("Here is a sample example.", pattern);
        }
    }

    // Usage 6
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher("Here is a sample example.");
        if (pattern.has_compiled()) {
            Long pos = AnsiString::Kmp::match(matcher, pattern);
        }
    }
}

template <typename AlgorithmTy>
void StringMatch_unittest()
{
    typedef typename AlgorithmTy::Pattern pattern_type;

    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    printf("------------------------------------------------------\n");
    printf("  UnitTest for: %s\n", AlgorithmTy::name());
    printf("------------------------------------------------------\n\n");

    test::StopWatch sw;
    Long sum, index_of;

    // pattern: "example"
    pattern_type pattern;
    pattern.preprocessing("example");

    sum = 0;
    sw.start();
    for (size_t i = 0; i < kIterations; ++i) {
        index_of = pattern.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern.print_result("Here is a sample example.",
                         (int)index_of, (int)sum, sw.getMillisec());

    // pattern1: "sample"
    pattern_type pattern1(pattern_text_1);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < kIterations; ++i) {
        index_of = pattern1.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern1.print_result("Here is a sample example.",
                          (int)index_of, (int)sum, sw.getMillisec());

    // pattern2: "a sample"
    pattern_type pattern2(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < kIterations; ++i) {
        index_of = pattern2.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern2.print_result("Here is a sample example.",
                          (int)index_of, (int)sum, sw.getMillisec());
}

template <typename AlgorithmTy, typename StandardAlgorithmTy>
void StringMatch_verify()
{
    // Let search texts first address align for 16 bytes.
    StringRef texts[kSearchTexts];
    char * text_data[kPatterns];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        size_t length = ::strlen(SearchTexts[i]);

        // Don't use C++11 alloc aligned memory version:
        // void * aligned_alloc(size_t alignment, size_t size);
        // is for more versatility.
#if defined(_MSC_VER)
        text_data[i] = (char *)::_aligned_malloc(length + 1, 16);
#else
        text_data[i] = nullptr;
        int err = ::posix_memalign((void **)&text_data[i], 16, length + 1);
        if (text_data[i] == nullptr)
            return;
#endif
        ::memcpy((void *)text_data[i], (const void *)SearchTexts[i], length + 1);
        texts[i].set_data(text_data[i], length);
    }

    // Let pattern first address align for 16 bytes.
    StringRef pattern[kPatterns];
    char * pattern_data[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        size_t length = ::strlen(Patterns[i]);

        // Don't use C++11 alloc aligned memory version:
        // void * aligned_alloc(size_t alignment, size_t size);
        // is for more versatility.
#if defined(_MSC_VER)
        pattern_data[i] = (char *)::_aligned_malloc(length + 1, 16);
#else
        pattern_data[i] = nullptr;
        int err = ::posix_memalign((void **)&pattern_data[i], 16, length + 1);
        if (pattern_data[i] == nullptr)
            return;
#endif
        ::memcpy((void *)pattern_data[i], (const void *)Patterns[i], length + 1);
        pattern[i].set_data(pattern_data[i], length);
    }

    for (size_t i = 0; i < kSearchTexts; ++i) {
        for (size_t j = 0; j < kPatterns; ++j) {
            Long index_of_1 = AlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                 pattern[j].c_str(), pattern[j].size());
            Long index_of_2 = StandardAlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                         pattern[j].c_str(), pattern[j].size());
            if (index_of_1 != index_of_2) {
                printf("text[%" PRIuPTR "] = \"%s\",\n", i, texts[i].c_str());
                printf("pattern[%" PRIuPTR "] = \"%s\"\n", j, pattern[j].c_str());
                printf("index_of_1: %" PRIiPTR ", index_of_2: %" PRIiPTR "\n\n",
                       index_of_1, index_of_2);
            }
        }
    }
    //printf("\n");
}

template <typename AlgorithmTy>
void StringMatch_benchmark()
{
    typedef typename AlgorithmTy::Pattern pattern_type;

#if SWITCH_BENCHMARK_TEST
    static const size_t iters = 1;
#else
    static const size_t iters = kIterations / (kSearchTexts * kPatterns) + 1;
#endif

    test::StopWatch sw;
    double preprocessing_time, searching_time, full_searching_time;
    Long searching_sum, full_searching_sum;
    int rnd_num = 0;

#if USE_ALIGNED_PATTAEN
    // Let search texts first address align for 16 bytes.
    StringRef texts[kSearchTexts];
    char * text_data[kPatterns];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        size_t length = ::strlen(SearchTexts[i]);

        // Don't use C++11 alloc aligned memory version:
        // void * aligned_alloc(size_t alignment, size_t size);
        // is for more versatility.
#if defined(_MSC_VER)
        text_data[i] = (char *)::_aligned_malloc(length + 1, 16);
#else
        text_data[i] = nullptr;
        int err = ::posix_memalign((void **)&text_data[i], 16, length + 1);
        if (text_data[i] == nullptr)
            return;
#endif
        ::memcpy((void *)text_data[i], (const void *)SearchTexts[i], length + 1);
        texts[i].set_data(text_data[i], length);
    }
#else
    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(SearchTexts[i], ::strlen(SearchTexts[i]));
    }
#endif // USE_ALIGNED_PATTAEN

#if USE_ALIGNED_PATTAEN
    // Let pattern first address align for 16 bytes.
    StringRef pattern_ref[kPatterns];
    char * pattern_data[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        size_t length = ::strlen(Patterns[i]);

        // Don't use C++11 alloc aligned memory version:
        // void * aligned_alloc(size_t alignment, size_t size);
        // is for more versatility.
#if defined(_MSC_VER)
        pattern_data[i] = (char *)::_aligned_malloc(length + 1, 16);
#else
        pattern_data[i] = nullptr;
        int err = ::posix_memalign((void **)&pattern_data[i], 16, length + 1);
        if (pattern_data[i] == nullptr)
            return;
#endif
        ::memcpy((void *)pattern_data[i], (const void *)Patterns[i], length + 1);
        pattern_ref[i].set_data(pattern_data[i], length);
    }

    // Preprocessing
    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(pattern_ref[i]);
    }
#else
    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(Patterns[i]);
    }
#endif // USE_ALIGNED_PATTAEN

    // Searching
    searching_sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                Long index_of = pattern[j].match(texts[i].c_str(), texts[i].size());
#if SWITCH_BENCHMARK_TEST
                printf("index_of = %d\n", (int)index_of);
#endif
                searching_sum += index_of;
            }
#if SWITCH_BENCHMARK_TEST
            printf("\n");
#endif
        }
        rnd_num += rand();
    }
    sw.stop();
    searching_time = sw.getMillisec();

    // Full searching
    full_searching_sum = 0;
    full_searching_time = 0.0;
    if (AlgorithmTy::need_preprocessing()) {
        sw.start();
        for (size_t loop = 0; loop < iters; ++loop) {
            for (size_t i = 0; i < kSearchTexts; ++i) {
                for (size_t j = 0; j < kPatterns; ++j) {
                    Long index_of = AlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                       pattern[j].c_str(), pattern[j].size());
#if SWITCH_BENCHMARK_TEST
                    printf("index_of = %d\n", (int)index_of);
#endif
                    full_searching_sum += index_of;
                }
#if SWITCH_BENCHMARK_TEST
                printf("\n");
#endif
            }

            AlgorithmTy::reset_counter();
            rnd_num += rand();
        }
        sw.stop();
        full_searching_time = sw.getMillisec();
    }

    if (AlgorithmTy::need_preprocessing()) {
        preprocessing_time = full_searching_time - searching_time;
        if (preprocessing_time < 0.0)
            preprocessing_time = 0.0;
        printf("  %-22s   %-12u (%u)    %8.3f ms    %8.3f ms    %8.3f ms\n",
               AlgorithmTy::name(), (unsigned int)((searching_sum + full_searching_sum) / 2),
               ((unsigned int)rnd_num % 10),
               preprocessing_time, searching_time, full_searching_time);
    }
    else {
        preprocessing_time = 0.0;
        SM_UNUSED_VAR(preprocessing_time);
        SM_UNUSED_VAR(full_searching_time);
        printf("  %-22s   %-12u (%u)    %s    %8.3f ms    %s\n",
               AlgorithmTy::name(), (unsigned int)(searching_sum + full_searching_sum),
               ((unsigned int)rnd_num % 10),
               "   -----   ", searching_time, "   -----   ");
    }

#if USE_ALIGNED_PATTAEN
    for (size_t i = 0; i < kSearchTexts; ++i) {
        if (text_data[i] != nullptr) {
#if defined(_MSC_VER)
            ::_aligned_free(text_data[i]);
#else
            ::free(text_data[i]);
#endif
            text_data[i] = nullptr;
        }
    }

    for (size_t i = 0; i < kPatterns; ++i) {
        if (pattern_data[i] != nullptr) {
#if defined(_MSC_VER)
            ::_aligned_free(pattern_data[i]);
#else
            ::free(pattern_data[i]);
#endif
            pattern_data[i] = nullptr;
        }
    }
#endif
}

void print_arch_type()
{
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__aarch64__)
    printf("Arch type: __amd64__\n\n");
#elif defined(_M_IA64)
    printf("Arch type: __itanium64__\n\n");
#elif defined(_M_ARM64)
    printf("Arch type: __arm64__\n\n");
#elif defined(_M_ARM)
    printf("Arch type: __arm__\n\n");
#else
    printf("Arch type: __x86__\n\n");
#endif
}

int main(int argc, char * argv[])
{
    print_arch_type();

    StringMatch_usage_examples();

    cpu_warmup(1000);

    ::srand((unsigned int)::time(nullptr));

#if 0
    StringMatch_unittest<AnsiString::StrStr>();
    StringMatch_unittest<AnsiString::MemMem>();
    StringMatch_unittest<AnsiString::MyMemMem>();
    StringMatch_unittest<AnsiString::SSEStrStr>();
    StringMatch_unittest<AnsiString::Kmp>();
    StringMatch_unittest<AnsiString::BoyerMoore>();
    StringMatch_unittest<AnsiString::ShiftOr>();
#endif

    StringMatch_verify<AnsiString::WordHash, AnsiString::StrStr>();
    StringMatch_verify<AnsiString::Volnitsky, AnsiString::StrStr>();
    StringMatch_verify<AnsiString::FastStrStr, AnsiString::StrStr>();

    if (1) {
#if SWITCH_BENCHMARK_TEST
        StringMatch_benchmark<AnsiString::StrStr>();
        StringMatch_benchmark<AnsiString::SSEStrStr>();
#else
        printf("  Algorithm Name           CheckSum    Rand   Preprocessing   Search Time    Full Search Time\n");
        printf("-------------------------------------------------------------------------------------------------\n");

        StringMatch_benchmark<AnsiString::StrStr>();
        StringMatch_benchmark<AnsiString::SSEStrStr>();
        StringMatch_benchmark<AnsiString::SSEStrStr2>();
        StringMatch_benchmark<AnsiString::SSEStrStrA>();
        StringMatch_benchmark<AnsiString::SSEStrStrA_v0>();
        StringMatch_benchmark<AnsiString::SSEStrStrA_v2>();
        StringMatch_benchmark<AnsiString::GlibcStrStr>();
        StringMatch_benchmark<AnsiString::GlibcStrStrOld>();
        StringMatch_benchmark<AnsiString::MyStrStr>();
        printf("\n");
        StringMatch_benchmark<AnsiString::MemMem>();
#if 0
        StringMatch_benchmark<AnsiString::MyMemMem>();
        StringMatch_benchmark<AnsiString::MyMemMemBw>();
#endif
        StringMatch_benchmark<AnsiString::FastStrStr>();
        printf("\n");
        StringMatch_benchmark<AnsiString::StdSearch>();
#if ((defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)) || (defined(__cplusplus) && (__cplusplus >= 201703L)))
        StringMatch_benchmark<AnsiString::StdBoyerMoore>();
#endif
        printf("\n");
        StringMatch_benchmark<AnsiString::Kmp>();
#if TEST_ALL_BENCHMARK
        //StringMatch_benchmark<AnsiString::KmpStd>();
        printf("\n");
        StringMatch_benchmark<AnsiString::BoyerMoore>();
#endif
        StringMatch_benchmark<AnsiString::BMTuned>();
        StringMatch_benchmark<AnsiString::Sunday>();
        StringMatch_benchmark<AnsiString::Horspool>();
        StringMatch_benchmark<AnsiString::QuickSearch>();
        printf("\n");
#if TEST_ALL_BENCHMARK
        StringMatch_benchmark<AnsiString::ShiftAnd>();
#endif
        StringMatch_benchmark<AnsiString::ShiftOr>();
        StringMatch_benchmark<AnsiString::WordHash>();
        StringMatch_benchmark<AnsiString::Volnitsky>();
        StringMatch_benchmark<AnsiString::RabinKarp2>();
        StringMatch_benchmark<AnsiString::RabinKarp31>();
        printf("\n");
#if ENABLE_AHOCORASICK_TEST
        StringMatch_benchmark<AnsiString::AhoCorasick>();
#endif

        printf("-------------------------------------------------------------------------------------------------\n");
        //printf("  ps: (*) indicates that not included the preprocessing time.\n");
        printf("\n");
#endif

#if (defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_))
        //::system("pause");
#endif
    }
    return 0;
}
