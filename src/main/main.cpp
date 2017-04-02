
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
    AnsiString::kmp_pattern pattern;
    pattern.prepare("example");

    AnsiString::kmp kmp;
    int index_of = kmp.find("Here is a sample example.", pattern);
    kmp.display(index_of);
    ::system("pause");
    return 0;
}
