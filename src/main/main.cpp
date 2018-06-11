
#ifdef _DEBUG
//#include <vld.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#ifndef __cplusplus
#include <stdalign.h>   // C11 defines _Alignas().  This header defines alignas()
#endif

#define USE_ALIGNED_PATTAEN     1

#include "StringMatch.h"
#include "support/StopWatch.h"

#include "algorithm/StrStr.h"
#include "algorithm/MemMem.h"
#include "algorithm/MyStrStr.h"
#include "algorithm/GlibcStrStr.h"
#include "algorithm/GlibcStrStrOld.h"
#include "algorithm/SSEStrStr.h"
#include "algorithm/SSEStrStr2.h"
#include "algorithm/MyMemMem.h"
#include "algorithm/MyMemMemBw.h"
#include "algorithm/StdSearch.h"
#include "algorithm/Kmp.h"
#include "algorithm/Kmp2.h"
#include "algorithm/BoyerMoore.h"
#include "algorithm/Sunday.h"
#include "algorithm/Horspool.h"
#include "algorithm/ShiftAnd.h"
#include "algorithm/ShiftOr.h"

#define USE_BENCHMARK_TEST      0

using namespace StringMatch;

#if defined(NDEBUG)
static const size_t kIterations = 5000000;
#else
static const size_t kIterations = 10000;
#endif

//
// EXACT STRING MATCHING ALGORITHMS (Animation in Java)
//
// See: http://www-igm.univ-mlv.fr/~lecroq/string/
//

//
// See: http://volnitsky.com/project/str_search/index.html
//
static const alignas(16) char * s_SearchTexts[] = {
    "Here is a sample example.",

    "8'E . It consists of a number of low-lying, largely mangrove covered islands covering an area of around 665 km^2. "
    "The population of Bakassi is theTsubject of some dispute, but is generally put at between 150,000 and 300,000 people."
};

static const alignas(16) char * s_Patterns[] = {
    "sample",
    "example",

    "islands",
    "around",
    "subject",
    "between",
    "people",

    //"love you",
    //"I love you",

    "largely mangrove",
    "largely Mangrove",
    "some dispute",
    "some dicpute",
    "The population of Bakassi",
    "The population of bakassi",

    "between 150,000",
    "between 150,0000",
    "between 150,000 and 300,000 people."
};

static const size_t kSearchTexts = sm_countof_i(s_SearchTexts);
static const size_t kPatterns = sm_countof_i(s_Patterns);

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
                         (int)index_of, (int)sum, sw.getElapsedMillisec());

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
                          (int)index_of, (int)sum, sw.getElapsedMillisec());

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
                          (int)index_of, (int)sum, sw.getElapsedMillisec());
}

void StringMatch_strstr_benchmark1()
{
    static const size_t iters = kIterations / (kSearchTexts * kPatterns);

    test::StopWatch sw;
    Long sum;

    printf("------------------------------------------------------\n");
    printf("  Benchmark: %s\n", "strstr() **");
    printf("------------------------------------------------------\n\n");

    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    StringRef patterns[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        patterns[i].set_data(s_Patterns[i], strlen(s_Patterns[i]));
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                const char * substr = strstr(texts[i].c_str(), patterns[j].c_str());
                if (substr != nullptr) {
                    Long index_of = (Long)(substr - texts[i].c_str());
                    sum += index_of;
                }
                else {
                    sum += (int)Status::NotFound;
                }
            }
        }
    }
    sw.stop();

    printf("[Preprocessing: no ] sum: %-11d, time: %0.3f ms\n\n",
            (int)sum, sw.getElapsedMillisec());
}

void StringMatch_strstr_benchmark()
{
    static const size_t iters = kIterations / (kSearchTexts * kPatterns);

    test::StopWatch sw;
    Long sum;

    printf("---------------------------------------------------------------------\n");

    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    StringRef patterns[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        patterns[i].set_data(s_Patterns[i], strlen(s_Patterns[i]));
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                const char * substr = strstr(texts[i].c_str(), patterns[j].c_str());
                if (substr != nullptr) {
                    int index_of = (int)(substr - texts[i].c_str());
                    sum += index_of;
                }
                else {
                    sum += (int)Status::NotFound;
                }
            }
        }
    }
    sw.stop();

    printf(" strstr()** [Preprocessing: no ] sum: %d, time: %0.3f ms\n\n",
           (int)sum, sw.getElapsedMillisec());
}

template <typename AlgorithmTy>
void StringMatch_benchmark1()
{
    typedef typename AlgorithmTy::Pattern pattern_type;

    static const size_t iters = kIterations / (kSearchTexts * kPatterns);

    test::StopWatch sw;
    double matching_time, full_time;
    Long sum1, sum2;

    printf("-----------------------------------------------------------\n");
    printf("  Benchmark for: %s\n", AlgorithmTy::name());
    printf("-----------------------------------------------------------\n\n");

    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(s_Patterns[i]);
    }

#if 0
    if (AlgorithmTy::need_preprocessing())
        printf("need preprocessing: yes\n\n");
    else
        printf("need preprocessing: no\n\n");
#endif

    sum1 = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                Long index_of = pattern[j].match(texts[i].c_str(), texts[i].size());
                sum1 += index_of;
            }
        }
    }
    sw.stop();
    matching_time = sw.getElapsedMillisec();

    printf("[Preprocessing: no ] sum: %-11d, time: %0.3f ms\n", (int)sum1, matching_time);

    if (AlgorithmTy::need_preprocessing()) {
        sum2 = 0;
        sw.start();
        for (size_t loop = 0; loop < iters; ++loop) {
            for (size_t i = 0; i < kSearchTexts; ++i) {
                for (size_t j = 0; j < kPatterns; ++j) {
                    Long index_of = AlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                       pattern[j].c_str(), pattern[j].size());
                    sum2 += index_of;
                }
            }
        }
        sw.stop();
        full_time = sw.getElapsedMillisec();

        printf("[Preprocessing: yes] sum: %-11d, time: %0.3f ms\n", (int)sum2, full_time);
    }

    printf("\n");
}

template <typename AlgorithmTy>
void StringMatch_benchmark2()
{
    typedef typename AlgorithmTy::Pattern pattern_type;

#if USE_BENCHMARK_TEST
    static const size_t iters = 1;
#else
    static const size_t iters = kIterations / (kSearchTexts * kPatterns);
#endif

    test::StopWatch sw;
    double matching_time, full_time;
    Long sum1, sum2;

    printf("---------------------------------------------------------------------\n");

    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(s_Patterns[i]);
    }

    sum1 = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                Long index_of = pattern[j].match(texts[i].c_str(), texts[i].size());
#if USE_BENCHMARK_TEST
                printf("index_of = %d\n", (int)index_of);
#endif
                sum1 += index_of;
            }
#if USE_BENCHMARK_TEST
            printf("\n");
#endif
        }
    }
    sw.stop();
    matching_time = sw.getElapsedMillisec();

    printf(" %s [Preprocessing: no ] sum: %d, time: %0.3f ms\n",
           AlgorithmTy::name(), (int)sum1, matching_time);

    if (AlgorithmTy::need_preprocessing()) {
        sum2 = 0;
        sw.start();
        for (size_t loop = 0; loop < iters; ++loop) {
            for (size_t i = 0; i < kSearchTexts; ++i) {
                for (size_t j = 0; j < kPatterns; ++j) {
                    Long index_of = AlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                       pattern[j].c_str(), pattern[j].size());
#if USE_BENCHMARK_TEST
                    printf("index_of = %d\n", (int)index_of);
#endif
                    sum2 += index_of;
                }
#if USE_BENCHMARK_TEST
                printf("\n");
#endif
            }
        }
        sw.stop();
        full_time = sw.getElapsedMillisec();

        printf(" %s [Preprocessing: yes] sum: %d, time: %0.3f ms\n",
               AlgorithmTy::name(), (int)sum2, full_time);
    }

    printf("\n");
}

template <typename AlgorithmTy>
void StringMatch_benchmark()
{
    typedef typename AlgorithmTy::Pattern pattern_type;

#if USE_BENCHMARK_TEST
    static const size_t iters = 1;
#else
    static const size_t iters = kIterations / (kSearchTexts * kPatterns);
#endif

    test::StopWatch sw;
    double matching_time, full_time;
    Long sum1, sum2;

    StringRef texts[kSearchTexts];
    for (size_t i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

#if USE_ALIGNED_PATTAEN
    // Let pattern first address align for 16 bytes.
    StringRef pattern_ref[kPatterns];
    char * pattern_data[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        size_t length = strlen(s_Patterns[i]);

        // Don't use C++11 alloc aligned memory version:
        // void * aligned_alloc(size_t alignment, size_t size);
        // is for more versatility.
#if defined(_MSC_VER)
        pattern_data[i] = (char *)_aligned_malloc(length + 1, 16);
#else
        int err = posix_memalign((void **)&pattern_data[i], length + 1, 16);
        if (err)
            return;
#endif
        memcpy((void *)pattern_data[i], (const void *)s_Patterns[i], length + 1);
        pattern_ref[i].set_data(pattern_data[i], length);
    }

    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(pattern_ref[i]);
    }
#else
    pattern_type pattern[kPatterns];
    for (size_t i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(s_Patterns[i]);
    }
#endif

    sum1 = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (size_t i = 0; i < kSearchTexts; ++i) {
            for (size_t j = 0; j < kPatterns; ++j) {
                Long index_of = pattern[j].match(texts[i].c_str(), texts[i].size());
#if USE_BENCHMARK_TEST
                printf("index_of = %d\n", (int)index_of);
#endif
                sum1 += index_of;
            }
#if USE_BENCHMARK_TEST
            printf("\n");
#endif
        }
    }
    sw.stop();
    matching_time = sw.getElapsedMillisec();

    std::string strName = AlgorithmTy::name();
    if (AlgorithmTy::need_preprocessing()) {
        strName += " (*)";
    }

    printf("  %-22s   %-12d    %8.3f ms\n", strName.c_str(), (int)sum1, matching_time);

    if (AlgorithmTy::need_preprocessing()) {
        sum2 = 0;
        sw.start();
        for (size_t loop = 0; loop < iters; ++loop) {
            for (size_t i = 0; i < kSearchTexts; ++i) {
                for (size_t j = 0; j < kPatterns; ++j) {
                    Long index_of = AlgorithmTy::match(texts[i].c_str(), texts[i].size(),
                                                       pattern[j].c_str(), pattern[j].size());
#if USE_BENCHMARK_TEST
                    printf("index_of = %d\n", (int)index_of);
#endif
                    sum2 += index_of;
                }
#if USE_BENCHMARK_TEST
                printf("\n");
#endif
            }
        }
        sw.stop();
        full_time = sw.getElapsedMillisec();

        printf("  %-22s   %-12d    %8.3f ms\n", AlgorithmTy::name(), (int)sum2, full_time);
    }

#if USE_ALIGNED_PATTAEN
    for (size_t i = 0; i < kPatterns; ++i) {
        if (pattern_data[i] != nullptr) {
#if defined(_MSC_VER)
            _aligned_free(pattern_data[i]);
#else
            free(pattern_data[i]);
#endif
            pattern_data[i] = nullptr;
        }
    }
#endif
}

int main(int argc, char * argv[])
{
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(_M_ARM) || defined(_M_ARM64) \
 || defined(__amd64__) || defined(__x86_64__)
    printf("Arch type: __amd64__\n\n");
#else
    printf("Arch type: __x86__\n\n");
#endif

    StringMatch_usage_examples();

    //StringMatch_unittest<AnsiString::StrStr>();
    //StringMatch_unittest<AnsiString::MyMemMem>();
    //StringMatch_unittest<AnsiString::SSEStrStr>();

#if 0
    StringMatch_unittest<AnsiString::StrStr>();
    StringMatch_unittest<AnsiString::MemMem>();
    StringMatch_unittest<AnsiString::Kmp>();
    StringMatch_unittest<AnsiString::BoyerMoore>();
    StringMatch_unittest<AnsiString::ShiftOr>();
#endif

    //StringMatch_strstr_benchmark();

#if USE_BENCHMARK_TEST
    StringMatch_benchmark<AnsiString::StrStr>();
    StringMatch_benchmark<AnsiString::SSEStrStr>();
#else
    printf("  Algorithm Name           CheckSum        Elapsed Time\n");
    printf("----------------------------------------------------------------\n");

    StringMatch_benchmark<AnsiString::StrStr>();
    StringMatch_benchmark<AnsiString::SSEStrStr>();
    StringMatch_benchmark<AnsiString::SSEStrStr2>();
    StringMatch_benchmark<AnsiString::GlibcStrStr>();
    StringMatch_benchmark<AnsiString::GlibcStrStrOld>();
    StringMatch_benchmark<AnsiString::MyStrStr>();
    printf("\n");
    StringMatch_benchmark<AnsiString::MemMem>();
    StringMatch_benchmark<AnsiString::MyMemMem>();
    StringMatch_benchmark<AnsiString::MyMemMemBw>();
    printf("\n");
    StringMatch_benchmark<AnsiString::StdSearch>();
    printf("\n");
    StringMatch_benchmark<AnsiString::Kmp>();
    StringMatch_benchmark<AnsiString::Kmp2>();
    printf("\n");
    StringMatch_benchmark<AnsiString::BoyerMoore>();
    StringMatch_benchmark<AnsiString::Sunday>();
    StringMatch_benchmark<AnsiString::Horspool>();
    printf("\n");
    StringMatch_benchmark<AnsiString::ShiftAnd>();
    StringMatch_benchmark<AnsiString::ShiftOr>();

    printf("----------------------------------------------------------------\n");
    printf("  ps: (*) indication not included the preprocessing time.\n");
    printf("\n");
#endif

#if (defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__))
    ::system("pause");
#endif
    return 0;
}
