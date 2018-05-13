
#ifdef _DEBUG
//#include <vld.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>

#include "StrStr.h"
#include "Kmp.h"
#include "BoyerMoore.h"
#include "ShiftOr.h"
#include "support/StopWatch.h"

using namespace StringMatch;

#ifndef _DEBUG
static const size_t kIterations = 5000000;
#else
static const size_t kIterations = 10000;
#endif

//
// See: http://volnitsky.com/project/str_search/index.html
//
static const char * s_SearchTexts[] = {
    "Here is a sample example.",

    "8'E . It consists of a number of low-lying, largely mangrove covered islands covering an area of around 665 km^2. "
    "The population of Bakassi is the subject of some dispute, but is generally put at between 150,000 and 300,000 people."
};

static const char * s_Patterns[] = {
    "sample",
    "example",

    "islands",
    "around",
    "subject",
    "between",
    "people",

    "between 150,000"
};

void StringMatch_examples()
{
    // Usage 1
    {
        AnsiString::Kmp::Pattern pattern("example");
        if (pattern.has_compiled()) {
            int pos = pattern.match("Here is a sample example.");
        }
    }

    // Usage 2
    {
        AnsiString::Kmp::Pattern pattern;
        bool compiled = pattern.preprocessing("example");
        if (compiled) {
            int pos = pattern.match("Here is a sample example.");
        }
    }

    // Usage 3
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher("Here is a sample example.");
        if (pattern.has_compiled()) {
            int pos = matcher.find(pattern);
        }
    }

    // Usage 4
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher;
        matcher.set_text("Here is a sample example.");
        if (pattern.has_compiled()) {
            int pos = matcher.find(pattern);
        }
    }

    // Usage 5
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher;
        if (pattern.has_compiled()) {
            int pos = matcher.find("Here is a sample example.", pattern);
        }
    }

    // Usage 6
    {
        AnsiString::Kmp::Pattern pattern("example");
        AnsiString::Kmp::Matcher matcher("Here is a sample example.");
        if (pattern.has_compiled()) {
            int pos = AnsiString::Kmp::find(matcher, pattern);
        }
    }
}

template <typename algorithm_type>
void StringMatch_test()
{
    typedef typename algorithm_type::Pattern pattern_type;

    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    printf("---------------------------------------------------------------------------------------\n");
    printf("  Test: %s\n", typeid(algorithm_type).name());
    printf("---------------------------------------------------------------------------------------\n\n");

    test::StopWatch sw;
    int sum, index_of;

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
    pattern.print_result("Here is a sample example.", index_of, sum, sw.getElapsedMillisec());

    // pattern1: "sample"
    pattern_type pattern1(pattern_text_1);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < kIterations; ++i) {
        index_of = pattern1.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern1.print_result("Here is a sample example.", index_of, sum, sw.getElapsedMillisec());

    // pattern2: "a sample"
    pattern_type pattern2(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < kIterations; ++i) {
        index_of = pattern2.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern2.print_result("Here is a sample example.", index_of, sum, sw.getElapsedMillisec());
}

void StringMatch_strstr_benchmark()
{
    test::StopWatch sw;
    int sum;
    static const size_t iters = kIterations / (sm_countof(s_SearchTexts) * sm_countof(s_Patterns));

    printf("-----------------------------------------------------------\n");
    printf("  Benchmark: %s\n", "strstr()");
    printf("-----------------------------------------------------------\n\n");

    static const int kSearchTexts = sm_countof_i(s_SearchTexts);
    static const int kPatterns = sm_countof_i(s_Patterns);

    StringRef texts[kSearchTexts];
    for (int i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    StringRef patterns[kPatterns];
    for (int i = 0; i < kPatterns; ++i) {
        patterns[i].set_data(s_Patterns[i], strlen(s_Patterns[i]));
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (int i = 0; i < kSearchTexts; ++i) {
            for (int j = 0; j < kPatterns; ++j) {
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

    printf("sum: %-11d, time spent: %0.3f ms\n\n", sum, sw.getElapsedMillisec());
}

template <typename algorithm_type>
void StringMatch_benchmark()
{
    typedef typename algorithm_type::Pattern pattern_type;

    test::StopWatch sw;
    int sum;
    static const size_t iters = kIterations / (sm_countof(s_SearchTexts) * sm_countof(s_Patterns));

    printf("---------------------------------------------------------------------------------------\n");
    printf("  Benchmark: %s\n", typeid(algorithm_type).name());
    printf("---------------------------------------------------------------------------------------\n\n");

    static const int kSearchTexts = sm_countof_i(s_SearchTexts);
    static const int kPatterns = sm_countof_i(s_Patterns);

    StringRef texts[kSearchTexts];
    for (int i = 0; i < kSearchTexts; ++i) {
        texts[i].set_data(s_SearchTexts[i], strlen(s_SearchTexts[i]));
    }

    pattern_type pattern[kPatterns];
    for (int i = 0; i < kPatterns; ++i) {
        pattern[i].preprocessing(s_Patterns[i]);
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (int i = 0; i < kSearchTexts; ++i) {
            for (int j = 0; j < kPatterns; ++j) {
                int index_of = pattern[j].match(texts[i].c_str());
                sum += index_of;
            }
        }
    }
    sw.stop();

    printf("sum: %-11d, time spent: %0.3f ms\n\n", sum, sw.getElapsedMillisec());
}

int main(int argc, char * argv[])
{
    StringMatch_examples();

    StringMatch_test<AnsiString::Kmp>();
    StringMatch_test<AnsiString::BoyerMoore>();
    StringMatch_test<AnsiString::ShiftOr>();

    StringMatch_strstr_benchmark();

    StringMatch_benchmark<AnsiString::StrStr>();
    StringMatch_benchmark<AnsiString::Kmp>();
    StringMatch_benchmark<AnsiString::BoyerMoore>();
    StringMatch_benchmark<AnsiString::ShiftOr>();

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
    ::system("pause");
#endif
    return 0;
}
