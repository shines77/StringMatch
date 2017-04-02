
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "StringMatch.h"
#include "support/stop_watch.h"

using namespace StringMatch;

int main(int argn, char * argv[])
{
    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    AnsiString::kmp_pattern pattern;
    pattern.prepare("example");

    AnsiString::kmp kmp;
    int index_of = kmp.find("Here is a sample example.", pattern);
    kmp.display(index_of);

    AnsiString::kmp_pattern pattern1(pattern_text_1);
    AnsiString::kmp_pattern pattern2(pattern_text_2);

    //pattern1.prepare(pattern_text_1);
    //pattern2.prepare(pattern_text_2);

    index_of = kmp.find("Here is a sample example.", pattern1);
    kmp.display(index_of);

    index_of = kmp.find("Here is a sample example.", pattern2);
    kmp.display(index_of);

    ::system("pause");
    return 0;
}
