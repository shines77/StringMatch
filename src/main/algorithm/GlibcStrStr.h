
#ifndef STRING_MATCH_GLIBC_STRSTR_H
#define STRING_MATCH_GLIBC_STRSTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <limits.h>
#include <assert.h>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"

#ifndef CHAR_BIT
# define CHAR_BIT    8
#endif

#if CHAR_BIT < 10
# define LONG_NEEDLE_THRESHOLD  32U
#else
# define LONG_NEEDLE_THRESHOLD  SIZE_MAX
#endif

/*
  For case-insensitivity, you may optionally define:

     CMP_FUNC(p1, p2, l)     A macro that returns 0 if the first L
			     characters of P1 and P2 are equal.
     CANON_ELEMENT(c)        A macro that canonicalizes an element right after
			     it has been fetched from one of the two strings.
			     The argument is an 'unsigned char'; the result
			     must be an 'unsigned char' as well.
*/

#ifndef CANON_ELEMENT
# define CANON_ELEMENT(c)   (uchar_type)(c)
#endif
#ifndef CMP_FUNC
# define CMP_FUNC   memcmp
#endif

/*
     AVAILABLE(h, h_l, j, n_l)
			     A macro that returns nonzero if there are
			     at least N_L bytes left starting at H[J].
			     H is 'unsigned char *', H_L, J, and N_L
			     are 'size_t'; H_L is an lvalue.  For
			     NUL-terminated searches, H_L can be
			     modified each iteration to avoid having
			     to compute the end of H up front.
*/
#ifndef AVAILABLE
#define AVAILABLE(h, h_l, j, n_l)			                \
        (!memchr((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
                  && ((h_l) = (j) + (n_l)))
#endif

/* Check for end-of-line in strstr and strcasestr routines.
   We piggy-back matching procedure for detecting EOL where possible,
   and use AVAILABLE macro otherwise.  */
#ifndef CHECK_EOL
# define CHECK_EOL (1)
#endif

/* Return nullptr if argument is '\0'.  */
#ifndef RET0_IF_0
# define RET0_IF_0(arg)  if (arg == char_type('\0')) goto ret0
#endif

namespace StringMatch {

/* Perform a critical factorization of NEEDLE, of length NEEDLE_LEN.
   Return the index of the first byte in the right half, and set
   *PERIOD to the global period of the right half.

   The global period of a string is the smallest index (possibly its
   length) at which all remaining bytes in the string are repetitions
   of the prefix (the last repetition may be a subset of the prefix).

   When NEEDLE is factored into two halves, a local period is the
   length of the smallest word that shares a suffix with the left half
   and shares a prefix with the right half.  All factorizations of a
   non-empty NEEDLE have a local period of at least 1 and no greater
   than NEEDLE_LEN.

   A critical factorization has the property that the local period
   equals the global period.  All strings have at least one critical
   factorization with the left half smaller than the global period.

   Given an ordered alphabet, a critical factorization can be computed
   in linear time, with 2 * NEEDLE_LEN comparisons, by computing the
   larger of two ordered maximal suffixes.  The ordered maximal
   suffixes are determined by lexicographic comparison of
   periodicity.  */

template <typename char_type>
static
size_t
critical_factorization(const char_type * pneedle, size_t needle_len, size_t * period)
{
    typedef typename jstd::uchar_traits<char_type>::type uchar_type;

    /* Index of last byte of left half, or SIZE_MAX.  */
    size_t max_suffix, max_suffix_rev;
    size_t j;               /* Index into NEEDLE for current candidate suffix.  */
    size_t k;               /* Offset into current period.  */
    size_t p;               /* Intermediate period.  */
    unsigned char a, b;     /* Current comparison bytes.  */

    const uchar_type * needle = (const uchar_type * )pneedle;

    /* Invariants:
       0 <= j < NEEDLE_LEN - 1
       -1 <= max_suffix{,_rev} < j (treating SIZE_MAX as if it were signed)
       min(max_suffix, max_suffix_rev) < global period of NEEDLE
       1 <= p <= global period of NEEDLE
       p == global period of the substring NEEDLE[max_suffix{,_rev}+1...j]
       1 <= k <= p
    */

    /* Perform lexicographic search.  */
    max_suffix = SIZE_MAX;
    j = 0;
    k = p = 1;
    while (j + k < needle_len) {
        a = CANON_ELEMENT(needle[j + k]);
        b = CANON_ELEMENT(needle[max_suffix + k]);
        if (a < b) {
            /* Suffix is smaller, period is entire prefix so far.  */
            j += k;
            k = 1;
            p = j - max_suffix;
        }
        else if (a == b) {
            /* Advance through repetition of the current period.  */
            if (k != p) {
                ++k;
            }
            else {
                j += p;
                k = 1;
            }
        }
        else {  /* b < a */
            /* Suffix is larger, start over from current location.  */
            max_suffix = j++;
            k = p = 1;
        }
    }

    *period = p;

    /* Perform reverse lexicographic search.  */
    max_suffix_rev = SIZE_MAX;
    j = 0;
    k = p = 1;
    while (j + k < needle_len) {
        a = CANON_ELEMENT(needle[j + k]);
        b = CANON_ELEMENT(needle[max_suffix_rev + k]);
        if (b < a) {
            /* Suffix is smaller, period is entire prefix so far.  */
            j += k;
            k = 1;
            p = j - max_suffix_rev;
        }
        else if (a == b) {
            /* Advance through repetition of the current period.  */
            if (k != p) {
                ++k;
            }
            else {
                j += p;
                k = 1;
            }
        }
        else {  /* a < b */
            /* Suffix is larger, start over from current location.  */
            max_suffix_rev = j++;
            k = p = 1;
        }
    }

    /* Choose the longer suffix.  Return the first byte of the right
       half, rather than the last byte of the left half.  */
    if (max_suffix_rev + 1 < max_suffix + 1) {
        return max_suffix + 1;
    }

    *period = p;
    return (max_suffix_rev + 1);
}

/* Return the first location of non-empty NEEDLE within HAYSTACK, or
   NULL.  HAYSTACK_LEN is the minimum known length of HAYSTACK.  This
   method is optimized for NEEDLE_LEN < LONG_NEEDLE_THRESHOLD.
   Performance is guaranteed to be linear, with an initialization cost
   of 2 * NEEDLE_LEN comparisons.

   If AVAILABLE does not modify HAYSTACK_LEN (as in memmem), then at
   most 2 * HAYSTACK_LEN - NEEDLE_LEN comparisons occur in searching.
   If AVAILABLE modifies HAYSTACK_LEN (as in strstr), then at most 3 *
   HAYSTACK_LEN - NEEDLE_LEN comparisons occur in searching.  */

template <typename char_type>
static
const char_type *
two_way_short_needle(const char_type * haystack, size_t haystack_len,
                     const char_type * needle, size_t needle_len)
{
    typedef typename jstd::uchar_traits<char_type>::type uchar_type;

    size_t i;       /* Index into current byte of NEEDLE.  */
    size_t j;       /* Index into current window of HAYSTACK.  */
    size_t period;  /* The period of the right half of needle.  */
    size_t suffix;  /* The index of the right half of needle.  */

    /* Factor the needle into two halves, such that the left half is
       smaller than the global period, and the right half is
       periodic (with a period as large as NEEDLE_LEN - suffix).  */
    suffix = critical_factorization(needle, needle_len, &period);

    /* Perform the search.  Each iteration compares the right half first.  */
    if (CMP_FUNC(needle, needle + period, suffix) == 0) {
        /* Entire needle is periodic; a mismatch can only advance by the
           period, so use memory to avoid rescanning known occurrences
           of the period.  */
        size_t memory = 0;
        j = 0;
        while (AVAILABLE(haystack, haystack_len, j, needle_len)) {
            const uchar_type * pneedle;
            const uchar_type * phaystack;

            /* Scan for matches in right half.  */
            i = sm_max(suffix, memory);
            pneedle = (const uchar_type *)&needle[i];
            phaystack = (const uchar_type *)&haystack[i + j];
            while (i < needle_len && (CANON_ELEMENT(*pneedle++)
                   == CANON_ELEMENT(*phaystack++))) {
                ++i;
            }
            if (needle_len <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = (const uchar_type *)&needle[i];
                phaystack = (const uchar_type *)&haystack[i + j];
                while (memory < i + 1 && (CANON_ELEMENT(*pneedle--)
                       == CANON_ELEMENT(*phaystack--))) {
                    --i;
                }
                if (i + 1 < memory + 1) {
                    return (const char_type *)(haystack + j);
                    /* No match, so remember how many repetitions of period
                       on the right half were scanned.  */
                }
                j += period;
                memory = needle_len - period;
            }
            else {
                j += i - suffix + 1;
                memory = 0;
            }
        }
    }
    else {
        const uchar_type * phaystack = (const uchar_type *)&haystack[suffix];
        /* The comparison always starts from needle[suffix], so cache it
           and use an optimized first-character loop.  */
        unsigned char needle_suffix = CANON_ELEMENT(needle[suffix]);

#if CHECK_EOL
        /* We start matching from the SUFFIX'th element, so make sure we
           don't hit '\0' before that.  */
        if (haystack_len < suffix + 1
            && !AVAILABLE(haystack, haystack_len, 0, suffix + 1)) {
            return nullptr;
        }
#endif

        /* The two halves of needle are distinct; no extra memory is
           required, and any mismatch results in a maximal shift.  */
        period = sm_max(suffix, needle_len - suffix) + 1;
        j = 0;
        while (1
#if !CHECK_EOL
            && AVAILABLE(haystack, haystack_len, j, needle_len)
#endif
            ) {
            uchar_type haystack_char;
            const uchar_type * pneedle;

            /* TODO: The first-character loop can be speed up by adapting
               longword-at-a-time implementation of memchr/strchr.  */
            if (needle_suffix
                != (haystack_char = CANON_ELEMENT(*phaystack++))) {
                RET0_IF_0(haystack_char);
#if !CHECK_EOL
                ++j;
#endif
                continue;
            }

#if CHECK_EOL
            /* Calculate J if it wasn't kept up-to-date in the first-character loop.  */
            j = phaystack - (const uchar_type * )&haystack[suffix] - 1;
#endif

            /* Scan for matches in right half.  */
            i = suffix + 1;
            pneedle = (const uchar_type *)&needle[i];
            while (i < needle_len) {
                if (CANON_ELEMENT(*pneedle++)
                    != (haystack_char = CANON_ELEMENT(*phaystack++))) {
                    RET0_IF_0(haystack_char);
                    break;
                }
                ++i;
            }
            if (needle_len <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = (const uchar_type *)&needle[i];
                phaystack = (const uchar_type *)&haystack[i + j];
                while (i != SIZE_MAX) {
                    if (CANON_ELEMENT(*pneedle--)
                        != (haystack_char = CANON_ELEMENT(*phaystack--))) {
                        RET0_IF_0(haystack_char);
                        break;
                    }
                    --i;
                }
                if (i == SIZE_MAX) {
                    return (const char_type *)(haystack + j);
                }
                j += period;
            }
            else {
                j += i - suffix + 1;
            }

#if CHECK_EOL
            if (!AVAILABLE(haystack, haystack_len, j, needle_len))
                break;
#endif
            phaystack = (const uchar_type *)&haystack[suffix + j];
        }
    }

ret0:
    return nullptr;
}

/* Return the first location of non-empty NEEDLE within HAYSTACK, or
   NULL.  HAYSTACK_LEN is the minimum known length of HAYSTACK.  This
   method is optimized for LONG_NEEDLE_THRESHOLD <= NEEDLE_LEN.
   Performance is guaranteed to be linear, with an initialization cost
   of 3 * NEEDLE_LEN + (1 << CHAR_BIT) operations.

   If AVAILABLE does not modify HAYSTACK_LEN (as in memmem), then at
   most 2 * HAYSTACK_LEN - NEEDLE_LEN comparisons occur in searching,
   and sublinear performance O(HAYSTACK_LEN / NEEDLE_LEN) is possible.
   If AVAILABLE modifies HAYSTACK_LEN (as in strstr), then at most 3 *
   HAYSTACK_LEN - NEEDLE_LEN comparisons occur in searching, and
   sublinear performance is not possible.  */

template <typename char_type>
static
const char_type *
two_way_long_needle(const char_type * haystack, size_t haystack_len,
                    const char_type * needle, size_t needle_len)
{
    typedef typename jstd::uchar_traits<char_type>::type uchar_type;

    size_t i;           /* Index into current byte of NEEDLE.  */
    size_t j;           /* Index into current window of HAYSTACK.  */
    size_t period;      /* The period of the right half of needle.  */
    size_t suffix;      /* The index of the right half of needle.  */
    size_t shift_table[1U << CHAR_BIT];     /* See below.  */

    /* Factor the needle into two halves, such that the left half is
       smaller than the global period, and the right half is
       periodic (with a period as large as NEEDLE_LEN - suffix).  */
    suffix = critical_factorization(needle, needle_len, &period);

    /* Populate shift_table.  For each possible byte value c,
       shift_table[c] is the distance from the last occurrence of c to
       the end of NEEDLE, or NEEDLE_LEN if c is absent from the NEEDLE.
       shift_table[NEEDLE[NEEDLE_LEN - 1]] contains the only 0.  */
    for (i = 0; i < (1U << CHAR_BIT); i++)
        shift_table[i] = needle_len;
    for (i = 0; i < needle_len; i++)
        shift_table[CANON_ELEMENT(needle[i])] = needle_len - i - 1;

    /* Perform the search.  Each iteration compares the right half first.  */
    if (CMP_FUNC(needle, needle + period, suffix) == 0) {
        /* Entire needle is periodic; a mismatch can only advance by the
           period, so use memory to avoid rescanning known occurrences
           of the period.  */
        size_t memory = 0;
        size_t shift;
        j = 0;
        while (AVAILABLE(haystack, haystack_len, j, needle_len)) {
            const uchar_type * pneedle;
            const uchar_type * phaystack;

            /* Check the last byte first; if it does not match, then
               shift to the next possible match location.  */
            shift = shift_table[CANON_ELEMENT(haystack[j + needle_len - 1])];
            if (0 < shift) {
                if (memory && shift < period) {
                    /* Since needle is periodic, but the last period has
                       a byte out of place, there can be no match until
                       after the mismatch.  */
                    shift = needle_len - period;
                }
                memory = 0;
                j += shift;
                continue;
            }
            /* Scan for matches in right half.  The last byte has
               already been matched, by virtue of the shift table.  */
            i = sm_max(suffix, memory);
            pneedle = (const uchar_type *)&needle[i];
            phaystack = (const uchar_type *)&haystack[i + j];
            while (i < needle_len - 1 && (CANON_ELEMENT(*pneedle++)
                   == CANON_ELEMENT(*phaystack++))) {
                ++i;
            }
            if (needle_len - 1 <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = (const uchar_type *)&needle[i];
                phaystack = (const uchar_type *)&haystack[i + j];
                while (memory < i + 1 && (CANON_ELEMENT(*pneedle--)
                       == CANON_ELEMENT(*phaystack--))) {
                    --i;
                }
                if (i + 1 < memory + 1) {
                    return (const char_type *)(haystack + j);
                    /* No match, so remember how many repetitions of period
                       on the right half were scanned.  */
                }
                j += period;
                memory = needle_len - period;
            }
            else {
                j += i - suffix + 1;
                memory = 0;
            }
        }
    }
    else {
        /* The two halves of needle are distinct; no extra memory is
           required, and any mismatch results in a maximal shift.  */
        size_t shift;
        period = sm_max(suffix, needle_len - suffix) + 1;
        j = 0;
        while (AVAILABLE(haystack, haystack_len, j, needle_len)) {
            const uchar_type * pneedle;
            const uchar_type * phaystack;

            /* Check the last byte first; if it does not match, then
               shift to the next possible match location.  */
            shift = shift_table[CANON_ELEMENT(haystack[j + needle_len - 1])];
            if (0 < shift) {
                j += shift;
                continue;
            }
            /* Scan for matches in right half.  The last byte has
               already been matched, by virtue of the shift table.  */
            i = suffix;
            pneedle = (const uchar_type *)&needle[i];
            phaystack = (const uchar_type *)&haystack[i + j];
            while (i < needle_len - 1 && (CANON_ELEMENT(*pneedle++)
                   == CANON_ELEMENT(*phaystack++))) {
                ++i;
            }
            if (needle_len - 1 <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = (const uchar_type *)&needle[i];
                phaystack = (const uchar_type *)&haystack[i + j];
                while (i != SIZE_MAX && (CANON_ELEMENT(*pneedle--)
                       == CANON_ELEMENT(*phaystack--))) {
                    --i;
                }
                if (i == SIZE_MAX) {
                    return (const char_type *)(haystack + j);
                }
                j += period;
            }
            else {
                j += i - suffix + 1;
            }
        }
    }
    return nullptr;
}

template <typename char_type>
static
SM_NOINLINE_DECLARE(const char_type *)
strstr_glibc(const char_type * haystack_start,
             const char_type * needle_start)
{
    const char_type * haystack = haystack_start;
    const char_type * needle = needle_start;
    size_t needle_len;    /* Length of NEEDLE.  */
    size_t haystack_len;  /* Known minimum length of HAYSTACK.  */
    bool ok = true;       /* True if NEEDLE is prefix of HAYSTACK.  */

    /* Determine length of NEEDLE, and in the process, make sure
       HAYSTACK is at least as long (no point processing all of a long
       NEEDLE if HAYSTACK is too short).  */
    while (*haystack && *needle) {
        ok &= *haystack++ == *needle++;
    }

    if (*needle)
        return nullptr;

    if (ok)
        return (const char_type *)haystack_start;

    /* Reduce the size of haystack using strchr, since it has a smaller
       linear coefficient than the Two-Way algorithm.  */
    needle_len = needle - needle_start;
    haystack = strchr(haystack_start + 1, *needle_start);
    if (haystack == nullptr || unlikely(needle_len == 1))
        return (const char_type *)haystack;

    needle -= needle_len;
    haystack_len = ((haystack > haystack_start + needle_len) ? 1
        : (needle_len + haystack_start - haystack));

    /* Perform the search.  Abstract memory is considered to be an array
       of 'unsigned char' values, not an array of 'char' values.  See
       ISO C 99 section 6.2.6.1.  */
    if (needle_len < LONG_NEEDLE_THRESHOLD) {
        return two_way_short_needle((const char_type *)haystack,
                                    haystack_len,
                                    (const char_type *)needle, needle_len);
    }
    else {
        return two_way_long_needle((const char_type *)haystack, haystack_len,
                                   (const char_type *)needle, needle_len);
    }
}

template <typename CharTy>
class GlibcStrStrImpl {
public:
    typedef GlibcStrStrImpl<CharTy> this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;

    GlibcStrStrImpl() {}
    ~GlibcStrStrImpl() {
        this->destroy();
    }

    static const char * name() { return "strstr_glibc()"; }
    static bool need_preprocessing() { return false; }

    bool is_alive() const { return true; }

    void destroy() {
    }

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        /* Don't need to do preprocessing. */
        SM_UNUSED_VAR(pattern);
        SM_UNUSED_VAR(length);
        return true;
    }

    /* Searching */
    Long search(const char_type * text, size_type text_len,
                const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);
        const char_type * substr = strstr_glibc(text, pattern);
        if (likely(substr != nullptr))
            return (Long)(substr - text);
        else
            return Status::NotFound;
    }
};

namespace AnsiString {
    typedef AlgorithmWrapper< GlibcStrStrImpl<char> >    GlibcStrStr;
}

namespace UnicodeString {
    typedef AlgorithmWrapper< GlibcStrStrImpl<wchar_t> > GlibcStrStr;
}

} // namespace StringMatch

#undef LONG_NEEDLE_THRESHOLD

#undef CHAR_BIT
#undef AVAILABLE
#undef CANON_ELEMENT
#undef CMP_FUNC
#undef RET0_IF_0
#undef CHECK_EOL

#endif // STRING_MATCH_GLIBC_STRSTR_H
