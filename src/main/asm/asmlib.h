/*************************** asmlib.h ***************************************
* Author:        Agner Fog
* Date created:  2003-12-12
* Last modified: 2013-10-04
* Project:       asmlib.zip
* Source URL:    www.agner.org/optimize
*
* Description:
* Header file for the asmlib function library.
* This library is available in many versions for different platforms.
* See asmlib-instructions.pdf for details.
*
* (c) Copyright 2003 - 2013 by Agner Fog. 
* GNU General Public License http://www.gnu.org/licenses/gpl.html
*****************************************************************************/

#ifndef STRINGMATCH_ASMLIB_H
#define STRINGMATCH_ASMLIB_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/***********************************************************************
Define compiler-specific types and directives
***********************************************************************/

#include "basic/stddef.h"
#include "basic/stdint.h"

// Turn off name mangling
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
Function prototypes, memory and string functions
***********************************************************************/

// Search for substring in string
char * A_strstr(char * haystack, const char * needle);
char * A_strstr_v0(char * haystack, const char * needle);
char * A_strstr_v2(char * haystack, const char * needle);

// Tell which instruction set is supported
int    InstructionSet(void);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#ifdef __cplusplus

static inline
const char * A_strstr(const char * haystack, const char * needle) {
   return A_strstr((char *)haystack, needle);
}  // Overload A_strstr with const char * version

static inline
const char * A_strstr_v0(const char * haystack, const char * needle) {
   return A_strstr_v0((char *)haystack, needle);
}  // Overload A_strstr with const char * version

static inline
const char * A_strstr_v2(const char * haystack, const char * needle) {
   return A_strstr_v2((char *)haystack, needle);
}  // Overload A_strstr with const char * version

#endif

// Test if emmintrin.h is included and __m128i defined
#if defined(__GNUC__) && defined(_EMMINTRIN_H_INCLUDED) && !defined(__SSE2__)
#error Please compile with -sse2 or higher 
#endif

#if defined(_INCLUDED_EMM) || (defined(_EMMINTRIN_H_INCLUDED) && defined(__SSE2__))
#define VECTORDIVISIONDEFINED
#endif

#endif // STRINGMATCH_ASMLIB_H
