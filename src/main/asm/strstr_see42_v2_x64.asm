;*************************  strstr_sse42_v2_64.asm  ***************************
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

global A_strstr_v2              ; Function A_strstr

; Direct entries to CPU-specific versions
global strstrGeneric_v2         ; Generic version for processors without SSE4.2
; global strstrSSE42_v2			; Version for processors with SSE4.2

; Imported from instrset64.asm:
extern InstructionSet           ; Instruction set for CPU dispatcher

section .text

; strstr function

%if ALLOW_OVERRIDE
global ?OVR_strstr_v2
?OVR_strstr_v2:
%endif

align 16
A_strstr_v2: ; function dispatching
	; jmp     near [strstrDispatch_v2]  ; Go to appropriate version, depending on instruction set

; define register use
%ifdef  WINDOWS
; Args: rcx，rdx，r8，r9 | Float: XMM0，XMM1，XMM2，XMM3
%define arg1       rcx                  ; parameter 1, pointer to haystack
%define arg2       rdx                  ; parameter 2, pointer to needle
%define bit_index  ecx                  ; bit index in eax mask
%define bit_indexr rcx                  ; bit index in eax mask
%define text       r8                   ; pointer to match in haystack
%define pattern    r9                   ; pointer to match in needle
%define text_n     rax                  ; pointer to new_haystack
%define tempb      r8b                  ; temporary byte
%else
; Args <= 6: rdi, rsi, rdx, rcx, r8, r9
; Args >  6: rdi, rsi, rdx, rcx, rax, (esp + 8), (esp)
%define arg1       rdi                  ; parameter 1, pointer to haystack
%define arg2       rsi                  ; parameter 2, pointer to needle
%define bit_index  ecx                  ; bit index in eax mask
%define bit_indexr rcx                  ; bit index in eax mask
%define text       r8                   ; pointer to match in haystack
%define pattern    r9                   ; pointer to match in needle
%define text_n     rdx                  ; pointer to new_haystack
%define tempb      cl                   ; temporary byte
%endif

; strstrSSE42_v2: ; SSE4.2 version
%ifdef _WINDOWS_
    push    text_n
%endif

    movdqu  xmm1, [arg2]                ; needle
    mov     text, arg1

STRSTR_MAIN_LOOP:
    ; [text] = haystack
    pcmpistri xmm1, [text], 00001100B   ; unsigned byte search, equal ordered, return mask in xmm0
    jc      MATCH_BEGIN                 ; found beginning of a match
    jz      STRSTR_NOT_FOUND            ; end of haystack found, dismatch and not found
    add     text, 16
    jmp     STRSTR_MAIN_LOOP

MATCH_BEGIN:
    jz      STRSTR_FOUND                ; haystack ends here, a short match is found
    ; bit_index = ecx                   ; index of first bit in mask of possible matches

    ; compare strings for full match
    lea     text, [text + bit_indexr]   ; haystack + index
    mov     text_n, text
    mov     pattern, arg2               ; needle

COMPARE_LOOP: ; compare loop for long match
    movdqu    xmm2, [pattern]           ; paragraph of needle
    ; 01011000B, equal each and mask, negative
    pcmpistri xmm2, [text_n], 00001100B ; unsigned bytes, equal ordered, modifies xmm0
    ; (can't use "equal each, masked" because it inverts when past end of needle, but not when past end of both)

    jno     LONGMATCH_FAILED            ; difference found after extending partial match
    js      STRSTR_FOUND                ; end of needle found, and no difference
    add     pattern, 16
    add     text_n, 16
    jmp     COMPARE_LOOP                ; loop to next 16 bytes

LONGMATCH_FAILED:
    ; mask exhausted for possible matches, continue to next haystack paragraph
    add     text, 1
    jmp     STRSTR_MAIN_LOOP            ; loop to next paragraph of haystack

STRSTR_FOUND: ; match found over more than one paragraph
    lea     rax, [text + bit_indexr]    ; haystack + index to begin of long match
%ifdef _WINDOWS_
    pop     text_n
%endif
    ret

STRSTR_NOT_FOUND: ; needle not found, return 0
    xor     rax, rax
%ifdef _WINDOWS_
    pop     text_n
%endif
    ret

found_short: ; match found within single paragraph
    ; rcx                               ; index of first match
    lea     rax, [text + rcx]           ; pointer to first match
%ifdef _WINDOWS_
    pop     text_n
%endif
    ret

; strstrSSE42: endp


align 16
strstrGeneric_v2: ; generic version

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


; align 16
; ; CPU dispatching for strstr. This is executed only once
; strstrCPUDispatch_v2:
;     ; get supported instruction set
;     push    arg1
;     push    arg2
;     call    InstructionSet
;     pop     arg2
;     pop     arg1
;     ; Point to generic version of strstr
;     lea     r9, [strstrGeneric_v2]
;     cmp     eax, 10                ; check SSE4.2
;     jb      Q100
;     ; SSE4.2 supported
;     ; Point to SSE4.2 version of strstr
;     lea     r9, [strstrSSE42_v2]
; Q100:   mov     [strstrDispatch_v2], r9
;     ; Continue in appropriate version of strstr
;     jmp     r9

SECTION .data

; Pointer to appropriate version. Initially points to dispatcher
; strstrDispatch_v2 DQ strstrCPUDispatch_v2

; Append 16 bytes to end of last data section to allow reading past end of strings:
; (We might use names .bss$zzz etc. under Windows to make it is placed
; last, but the assembler gives sections with unknown names wrong attributes.
; Here, we are just relying on library data being placed after main data.
; This can be verified by making a link map file)
SECTION .bss
    dq      0, 0
