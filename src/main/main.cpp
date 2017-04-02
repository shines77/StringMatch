
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

    AnsiString::kmp kmp;

    AnsiString::kmp_pattern pattern;
    pattern.prepare("example");    

    sum = 0;
    sw.start();    
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = kmp.find("Here is a sample example.", pattern);
        sum += index_of;
    }
    sw.stop();
    kmp.display(index_of, sum, sw.getElapsedMillisec());

    AnsiString::kmp_pattern pattern1(pattern_text_1);
    AnsiString::kmp_pattern pattern2(pattern_text_2);

    //pattern1.prepare(pattern_text_1);
    //pattern2.prepare(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = kmp.find("Here is a sample example.", pattern1);
        sum += index_of;
    }
    sw.stop();
    kmp.display(index_of, sum, sw.getElapsedMillisec());

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = kmp.find("Here is a sample example.", pattern2);
        sum += index_of;
    }
    sw.stop();
    kmp.display(index_of, sum, sw.getElapsedMillisec());

    ::system("pause");
    return 0;
}
