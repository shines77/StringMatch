
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "StringMatch.h"
#include "support/stop_watch.h"

using namespace StringMatch;

#ifndef _DEBUG
const size_t Iterations = 10000000;
#else
const size_t Iterations = 10000;
#endif

int main(int argn, char * argv[])
{
    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    jimi::StopWatch sw;
    int sum, index_of;

    AnsiString::kmp::Matcher matcher;

    AnsiString::kmp::Pattern pattern;
    pattern.prepare("example");    

    sum = 0;
    sw.start();    
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern.display_test(index_of, sum, sw.getElapsedMillisec());

    AnsiString::kmp::Pattern pattern1(pattern_text_1);
    //pattern1.prepare(pattern_text_1);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern1.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern1.display_test(index_of, sum, sw.getElapsedMillisec());

    AnsiString::kmp::Pattern pattern2(pattern_text_2);
    //pattern2.prepare(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern2.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern2.display_test(index_of, sum, sw.getElapsedMillisec());

    ::system("pause");
    return 0;
}
