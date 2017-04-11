
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

#include "Kmp.h"
#include "BoyerMoore.h"
#include "support/StopWatch.h"

using namespace StringMatch;

#ifndef _DEBUG
const size_t Iterations = 5000000;
#else
const size_t Iterations = 10000;
#endif

//
// See: http://volnitsky.com/project/str_search/index.html
//
static const char * szSearchText[] = {
    "Here is a sample example.",

    "8'E . It consists of a number of low-lying, largely mangrove covered islands covering an area of around 665 km^2. "
    "The population of Bakassi is the subject of some dispute, but is generally put at between 150,000 and 300,000 people."
};

static const char * szPatterns[] = {
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
        AnsiString::Kmp::Pattern p("example");
        int pos = p.match("Here is a sample example.");
    }

    // Usage 2
    {
        AnsiString::Kmp::Pattern p;
        p.prepare("example");
        int pos = p.match("Here is a sample example.");
    }

    // Usage 3
    {
        AnsiString::Kmp::Pattern p("example");
        AnsiString::Kmp::Matcher m("Here is a sample example.");
        int pos = m.find(p);
    }

    // Usage 4
    {
        AnsiString::Kmp::Pattern p("example");
        AnsiString::Kmp::Matcher m;
        m.set_text("Here is a sample example.");
        int pos = m.find(p);
    }

    // Usage 5
    {
        AnsiString::Kmp::Pattern p("example");
        AnsiString::Kmp::Matcher m;
        int pos = m.find("Here is a sample example.", p);
    }

    // Usage 6
    {
        AnsiString::Kmp::Pattern p("example");
        AnsiString::Kmp::Matcher m("Here is a sample example.");
        int pos = AnsiString::Kmp::find(m, p);
    }
}

template <typename algorithm_type>
void StringMatch_test()
{
    typedef typename algorithm_type::Pattern pattern_type;

    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    printf("-----------------------------------------------------------\n");
    printf("  Test: %s\n", typeid(algorithm_type).name());
    printf("-----------------------------------------------------------\n\n");

    test::StopWatch sw;
    int sum, index_of;

    // pattern: "example"
    pattern_type pattern;
    pattern.prepare("example");

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern.display(index_of, sum, sw.getElapsedMillisec());

    // pattern: pattern_text_1
    pattern_type pattern1(pattern_text_1);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern1.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern1.display(index_of, sum, sw.getElapsedMillisec());

    // pattern: pattern_text_2
    pattern_type pattern2(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern2.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern2.display(index_of, sum, sw.getElapsedMillisec());
}

template <typename algorithm_type>
void StringMatch_benchmark()
{
    typedef typename algorithm_type::Pattern pattern_type;

    test::StopWatch sw;
    int sum;
    static const size_t iters = Iterations / (sm_countof(szSearchText) * sm_countof(szPatterns));

    printf("-----------------------------------------------------------\n");
    printf("  Benchmark: %s\n", typeid(algorithm_type).name());
    printf("-----------------------------------------------------------\n\n");

    pattern_type pattern[sm_countof_i(szPatterns)];
    for (int i = 0; i < sm_countof_i(szPatterns); ++i) {
        pattern[i].prepare(szPatterns[i]);
    }

    StringRef search_text[sm_countof_i(szSearchText)];
    for (int i = 0; i < sm_countof_i(szSearchText); ++i) {
        search_text[i].set_ref(szSearchText[i], strlen(szSearchText[i]));
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (int i = 0; i < sm_countof_i(szSearchText); ++i) {
            for (int j = 0; j < sm_countof_i(szPatterns); ++j) {
                int index_of = pattern[j].match(search_text[i]);
                sum += index_of;
            }
        }
    }
    sw.stop();

    printf("sum: %-11d, time spent: %0.3f ms\n\n", sum, sw.getElapsedMillisec());
}

void StringMatch_strstr_benchmark()
{
    test::StopWatch sw;
    int sum;
    static const size_t iters = Iterations / (sm_countof(szSearchText) * sm_countof(szPatterns));

    printf("-----------------------------------------------------------\n");
    printf("  Benchmark: %s\n", "strstr()");
    printf("-----------------------------------------------------------\n\n");

    StringRef search_text[sm_countof_i(szSearchText)];
    for (int i = 0; i < sm_countof_i(szSearchText); ++i) {
        search_text[i].set_ref(szSearchText[i], strlen(szSearchText[i]));
    }

    sum = 0;
    sw.start();
    for (size_t loop = 0; loop < iters; ++loop) {
        for (int i = 0; i < sm_countof_i(szSearchText); ++i) {
            for (int j = 0; j < sm_countof_i(szPatterns); ++j) {
                const char * substr = strstr(search_text[i].c_str(), szPatterns[j]);
                if (substr != nullptr) {
                    int index_of = (int)(substr - search_text[i].c_str());
                    sum += index_of;
                }
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

    StringMatch_strstr_benchmark();
    StringMatch_benchmark<AnsiString::Kmp>();
    StringMatch_benchmark<AnsiString::BoyerMoore>();

#if defined(_WIN32) || defined(OS_WINDOWS)
    int result = ::system("pause");
#endif
    return 0;
}
