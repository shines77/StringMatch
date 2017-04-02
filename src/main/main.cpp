
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
    AnsiString::kmp kmp;
    kmp.pattern("example");
    int index_of = kmp.match("Here is a sample example.");
    kmp.display(index_of);
    ::system("pause");
    return 0;
}
