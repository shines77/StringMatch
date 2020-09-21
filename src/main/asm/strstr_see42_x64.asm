;*************************  strstr_sse42_64.asm  ******************************
; Author:           Agner Fog
; Date created:     2011-07-14
; Last modified:    2011-07-14

; Description:
; Faster version of the standard strstr function:
; char * A_strstr(char * haystack, const char * needle);
; Searches for substring needle in string haystack. Return value is pointer to
; first occurrence of needle, or NULL if not found. The strings must be zero-terminated.
;
; Note that this function may read up to 15 bytes beyond the end of the strings.
; This is rarely a problem but it can in principle generate a protection violation
; if a string is placed at the end of the data segment. Avoiding this would be complicated
; and make the function much slower: For every unaligned 16-bytes read we would have to
; check if it crosses a page boundary (4 kbytes), and if so check if the string ends
; before the page boundary. Only if the string does not end before the page boundary
; can we read into the next memory page.
;
; Overriding standard function strstr:
; The alias ?OVR_strstr is changed to _strstr in the object file if
; it is desired to override the standard library function strstr.
; Overriding is disabled because the function may read beyond the end of a
; string, while the standard strstr function is guaranteed to work in all cases.
;
; Position-independent code is generated if POSITIONINDEPENDENT is defined.
;
; CPU dispatching included for 386 and SSE4.2 instruction sets.
;
; Copyright (c) 2011 GNU General Public License www.gnu.org/licenses
;******************************************************************************
BITS 64

default rel

%define ALLOW_OVERRIDE 0        ; Set to one if override of standard function desired

global A_strstr                 ; Function A_strstr

; Direct entries to CPU-specific versions
global strstrGeneric            ; Generic version for processors without SSE4.2
global strstrSSE42              ; Version for processors with SSE4.2

; Imported from instrset64.asm:
extern InstructionSet           ; Instruction set for CPU dispatcher

section .text

; strstr function

%if ALLOW_OVERRIDE
global ?OVR_strstr
?OVR_strstr:
%endif

align 16
A_strstr: ; function dispatching
        ; jmp     near [strstrDispatch]   ; Go to appropriate version, depending on instruction set

; define register use
%ifdef  WINDOWS
%define arg1       rcx                  ; parameter 1, pointer to haystack
%define arg2       rdx                  ; parameter 2, pointer to needle
%define bit_index  r8d                  ; bit index in eax mask
%define bit_indexr r8                   ; bit index in eax mask
%define text       r9                   ; pointer to match in haystack
%define pattern    r10                  ; pointer to match in needle
%define tempb      r8b                  ; temporary byte
%else
%define arg1       rdi                  ; parameter 1, pointer to haystack
%define arg2       rsi                  ; parameter 2, pointer to needle
%define bit_index  ecx                  ; bit index in eax mask
%define bit_indexr rcx                  ; bit index in eax mask
%define text       r9                   ; pointer to match in haystack
%define pattern    rdx                  ; pointer to match in needle
%define tempb      cl                   ; temporary byte
%endif

align 16
strstrSSE42: ; SSE4.2 version
        movdqu  xmm1, [arg2]                ; needle

haystack_next:
        ; [arg1] = haystack
        pcmpistrm xmm1, [arg1], 00001100B   ; unsigned byte search, equal ordered, return mask in xmm0
        jc      match_begin                 ; found beginning of a match
        jz      not_found                   ; end of haystack found, dismatch
        add     arg1, 16
        jmp     haystack_next

match_begin:
        jz      found_short                 ; haystack ends here, a short match is found
        movd    eax, xmm0                   ; bit mask of possible matches
next_bitindex:
        bsf     bit_index, eax              ; index of first bit in mask of possible matches

        ; compare strings for full match
        lea     text, [arg1 + bit_indexr]   ; haystack + index
        mov     pattern, arg2               ; needle

; align 16
compare_loop: ; compare loop for long match
        movdqu  xmm2, [pattern]             ; paragraph of needle
        pcmpistrm xmm2, [text], 00001100B   ; unsigned bytes, equal ordered, modifies xmm0
        ; (can't use "equal each, masked" because it inverts when past end of needle, but not when past end of both)

        jno     longmatch_fail              ; difference found after extending partial match
        js      longmatch_success           ; end of needle found, and no difference
        add     pattern, 16
        add     text, 16
        jmp     compare_loop                ; loop to next 16 bytes

longmatch_fail:
        ; remove index bit of first partial match
        btr     eax, bit_index
        test    eax, eax
        jnz     next_bitindex               ; mask contains more index bits, loop to next bit in eax mask
        ; mask exhausted for possible matches, continue to next haystack paragraph
        add     arg1, 16
        jmp     haystack_next               ; loop to next paragraph of haystack

longmatch_success: ; match found over more than one paragraph
        lea     rax, [arg1 + bit_indexr]    ; haystack + index to begin of long match
        ret

found_short: ; match found within single paragraph
        movd    eax, xmm0                   ; bit mask of matches
        bsf     eax, eax                    ; index of first match
        add     rax, arg1                   ; pointer to first match
        ret

not_found: ; needle not found, return 0
        xor     rax, rax
        ret

; strstrSSE42: endp


align 16
strstrGeneric: ; generic version

        mov     ax, [arg2]
        test    al, al
        jz      _Found                 ; a zero-length needle is always found
        test    ah, ah
        jz      _SingleCharNeedle

_SearchLoop: ; search for first character match
        mov     tempb, [arg1]
        test    tempb, tempb
        jz      _NotFound              ; end of haystack reached without finding
        cmp     al, tempb
        je      _FirstCharMatch        ; first character match
_IncompleteMatch:
        inc     arg1
        jmp     _SearchLoop            ; loop through haystack

_FirstCharMatch:
        mov     text, arg1             ; begin of match position
        mov     pattern, arg2
_MatchLoop:
        inc     text
        inc     pattern
        mov     al, [pattern]
        test    al, al
        jz      _Found                 ; end of needle. match ok
        cmp     al, [text]
        je      _MatchLoop
        ; match failed, recover and continue
        mov     al, [arg2]
        jmp     _IncompleteMatch

_NotFound: ; needle not found. return 0
        xor     eax, eax
        ret

_Found: ; needle found. return pointer to position in haystack
        mov     rax, arg1
        ret

_SingleCharNeedle: ; Needle is a single character
        mov     tempb, byte [arg1]
        test    tempb, tempb
        jz      _NotFound              ; end of haystack reached without finding
        cmp     al, tempb
        je      _Found
        inc     arg1
        jmp     _SingleCharNeedle      ; loop through haystack


align 16
; CPU dispatching for strstr. This is executed only once
strstrCPUDispatch:
        ; get supported instruction set
        push    arg1
        push    arg2
        call    InstructionSet
        pop     arg2
        pop     arg1
        ; Point to generic version of strstr
        lea     r9, [strstrGeneric]
        cmp     eax, 10                ; check SSE4.2
        jb      Q100
        ; SSE4.2 supported
        ; Point to SSE4.2 version of strstr
        lea     r9, [strstrSSE42]
Q100:   mov     [strstrDispatch], r9
        ; Continue in appropriate version of strstr
        jmp     r9

SECTION .data

; Pointer to appropriate version. Initially points to dispatcher
strstrDispatch DQ strstrCPUDispatch

; Append 16 bytes to end of last data section to allow reading past end of strings:
; (We might use names .bss$zzz etc. under Windows to make it is placed
; last, but the assembler gives sections with unknown names wrong attributes.
; Here, we are just relying on library data being placed after main data.
; This can be verified by making a link map file)
SECTION .bss
        dq      0, 0
