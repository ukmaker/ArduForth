; The core of the system is written in assembler for speed
; You can link devices from Arduino-land by writing a word
; which uses a SYSCALL instruction to call a function you
; supply in C to wrap the Ardino library
;
; Memory Map
; You should redefine this for your device depending on how
; much memory you have available
;
#SYSTEM:   0x0000 ; System reset vector
#INNER:    0x0100 ; Inner interpreter starts here
#MEMSIZE:  0x2000 ; Reserve 32K-bytes for Forth
#VARSTART: 0x1000 ; Variables grow up from here
#SPTOP:    0x1800 ; Data stack pointer grows down from here
#RSTOP:    0x1ffe ; And the return stack from here
                  ; This gives both stacks 2K bytes
#LBUF_LEN:  128   ; Maximum length of the line buffer in chars
#BASE_DEC: 10
#BASE_HEX: 16
#BASE_BIN: 2

; The predefined SYSCALLs
#SYSCALL_PRINTC: 0
#SYSCALL_TYPE: 1
#SYSCALL_TYPELN: 2
#SYSCALL_DOT: 3
#SYSCALL_GETC: 4
#SYSCALL_PUTC: 5
#SYSCALL_INLINE: 6
#SYSCALL_FLUSH: 7
#SYSCALL_NUMBER: 8
#SYSCALL_DEBUG: 9

#MODE_EXECUTE: 0
#MODE_COMPILE: 1
#STATE_NOT_FOUND: 0
#STATE_FOUND: 1

.ORG #VARSTART
%BASE: 2               ; Current system numeric base
%LBUF_IDX: 2           ; Current line buffer byte pointer
%LBUF_END: 2           ; Address of the last character in the buffer
%LBUF_DATA: #LBUF_LEN  ; Line buffer data
%DICTIONARY_POINTER: 2
%CONTEXT: 2
%CURRENT: 2
%CORE: 2
%MODE: 2
%STATE: 2

.ORG #SYSTEM ; Start vector is at address 0
START: 

  MOVIL A,DICTIONARY_END         ; Address of the cold-boot end of the dictionary
  MOVIL B,%DICTIONARY_POINTER
  ST B,A

  MOVIL A,CORE_VOCABULARY ; Last word in the dictionary
  MOVIL B,%CORE
  ST B,A

  MOVIL A,%CURRENT
  ST A,B
  MOVIL A,%CONTEXT
  ST A,B

STAR_RESTART: ; Restart from here if a runtime error occurs
  MOVIL SP,#SPTOP
  MOVIL RS,#RSTOP

  ; Set LBUF_END to LBUF_DATA
  MOVIL A,%LBUF_DATA
  MOVIL B,%LBUF_END
  ST B,A

  ; Start with BASE set to HEX
  MOVIL A,#BASE_HEX
  MOVIL B,%BASE
  ST B,A

  ; Execute mode
  MOVIL A,#MODE_EXECUTE
  MOVIL B,%MODE
  ST B,A

  MOVIL A,#STATE_NOT_FOUND
  MOVIL B,%STATE
  ST B,A

  MOVIL I,OUTER_CA
  JP NEXT

.ORG #INNER ; Beginning of the inner interpreter
COLON: 
  PUSHR I
  MOV I,WA
  JR NEXT
SEMI:
  .DATA SEMICA ; I.e. the address of the next word
SEMICA:
  POPR I
NEXT:
  LD WA,I ; WA <- (I)
  ADDI I,2
RUN:
  SYSCALL #SYSCALL_DEBUG
  LD CA,WA
  ADDI WA,2
  MOV PC,CA

MESSAGES: ; SYSTEM MESSAGES LIVE HERE
MSG_HELLO:          .DATA 22 .SDATA "Hello! I'm a TIL :-) >"
MSG_UNKNOWN_TOKEN:  .DATA 14 .SDATA "Unknown token "
MSG_COMPILE_ONLY:   .DATA 39 .SDATA "Compile-time words forbidden at runtime"
MSG_RUNTIME_ONLY:   .DATA 39 .SDATA "Runtime words forbidden at compile-time"
MSG_SYSTEM_ERROR:   .DATA 12 .SDATA "System error"
MSG_WORD_NOT_FOUND: .DATA 14 .SDATA "Word not found"
MSG_PROMPT:         .DATA  6 .SDATA " OK >>"
MSG_SP_UNDERFLOW:   .DATA 15 .SDATA "Stack underflow"
MSG_RS_UNDERFLOW:   .DATA 22 .SDATA "Return stack underflow"

BUILDS:
  .DATA 7
  .SDATA "<BUILDS"
  .DATA 0
BUILDS_WA: .DATA COLON
BUILDS_CA:
  .DATA STAR_HASH_WA
  .DATA 0
  .DATA CONSTANT_WA
  .DATA SEMI

DOES:
  .DATA 5
  .SDATA "DOES>"
  .DATA BUILDS
DOES_WA: .DATA COLON
DOES_CA:
  .DATA RFROM_WA
  .DATA ENTRY_WA
  .DATA WA_TO_CB_WA
  .DATA STORE_WA
  .DATA SCODE_WA
  PUSHR I
  LD I,WA
  ADDI WA,2
  PUSHD WA
  JP NEXT

IMMEDIATE:
  .DATA 9
  .SDATA "IMMEDIATE"
  .DATA DOES
IMMEDIATE_WA: .DATA IMMEDIATE_CA
IMMEDIATE_CA:
  MOVIL A,%CURRENT
  LD B,A ; point to vocab
  LD B,B ; point to word
  LD A,B ; get the word length
  MOVIL R0,0x8000
  OR A,R0 ; Set the immediate bit
  ST B,A
  JP NEXT

VOCABULARY:
  .DATA 10
  .SDATA "VOCABULARY"
  .DATA IMMEDIATE
VOCABULARY_WA: .DATA COLON
VOCABULARY_CA:
  .DATA BUILDS_WA
  .DATA ENTRY_WA
  .DATA COMMA_WA
  .DATA DOES_WA
  .DATA CONTEXT_WA
  .DATA STORE_WA
  .DATA SEMI

CORE:   ; My definition. Should really be as defined using VOCABULARY
        ; But I don't understand how that works yet
        ; And it needs to store values in ram, not the code-body of the word
  .DATA 4
  .SDATA "CODE"
  .DATA VOCABULARY
CORE_WA: .DATA CORE_CA
CORE_CA:
  MOVIL A,%CORE
  MOVIL B,%CONTEXT
  LD B,A
  JP NEXT

STAR_LIT:
  .DATA 2
  .SDATA "*\""
  .DATA CORE
STAR_LIT_WA: .DATA STAR_LIT_CA
STAR_LIT_CA:
  LD A,I ; Length to A
  PUSHD I
  SYSCALL #SYSCALL_TYPE
  ADD I,A
  ADDI I,3
  CLRI I,0 ; Word align
  JP NEXT

BREAKPOINT:
  .DATA 10
  .SDATA "BREAKPOINT"
  .DATA STAR_LIT
BREAKPOINT_WA: .DATA BREAKPOINT_CA
BREAKPOINT_CA:
  JP NEXT

RESTART:
  .DATA 7
  .SDATA "RESTART"
  .DATA BREAKPOINT
RESTART_WA: .DATA STAR_RESTART

; Test the stacks for underflow
; If there is underflow then print a message and restart
STAR_STACK:
  .DATA 6
  .SDATA "*STACK"
  .DATA RESTART
STAR_STACK_WA: .DATA STAR_STACK_CA
STAR_STACK_CA:
  MOVIL R0,#SPTOP
  SUB R0,SP
  JR[NC] STAR_STACK_CHECK_RS
  MOVIL SP,#SPTOP
  MOVIL A,MSG_SP_UNDERFLOW
  PUSHD A
  SYSCALL #SYSCALL_TYPELN
STAR_STACK_CHECK_RS:
  MOVIL R0,#RSTOP
  SUB R0,RS
  JR[NC] STAR_STACK_OK
  MOVIL A,MSG_RS_UNDERFLOW
  PUSHD A
  SYSCALL #SYSCALL_TYPELN
  JP STAR_RESTART
STAR_STACK_OK:
  JP NEXT

TYPE:
  .DATA 4           ; Length of TYPE
  .SDATA "TYPE"     ; And the string's characters
  .DATA STAR_STACK   
TYPE_WA:
  .DATA TYPE_CA    ; This is the word address of TYPE
TYPE_CA:             ; The Code address
  SYSCALL #SYSCALL_TYPE
  JP NEXT

EMIT:
  .DATA 4
  .SDATA "EMIT"
  .DATA TYPE
EMIT_WA: .DATA EMIT_CA
EMIT_CA:
  SYSCALL #SYSCALL_PUTC
  JP NEXT

MESSAGE: ; Print a system message ( n -- )
  .DATA 7
  .SDATA "MESSAGE"
  .DATA EMIT
MESSAGE_WA: .DATA MESSAGE_CA
MESSAGE_CA:
  POPD A                ; Message number n in A (0-based)
  MOVIL B,MESSAGES       ; B holds the message address
MESSAGE_LOOP:
  AND A,A               ; Is this the message we're looking for?
  JR[Z] MESSAGE_FOUND
  LD R0,B               ; Get the length of the message
  ADD B,R0              ; Add to the address
  ADDI B,3
  CLRI B,0               ; Word-align
  ADDI A,-1             ; Decrement A
  JR MESSAGE_LOOP
MESSAGE_FOUND:
  PUSHD B               ; Message address to the stack
  SYSCALL #SYSCALL_TYPE
  JP NEXT

DOT:  ; The Forth word "." to print the value on the top of the stack
  .DATA 1
  .SDATA "."
  .DATA MESSAGE
DOT_WA:
  .DATA DOT_CA
DOT_CA:
  MOVIL A,%BASE
  LD B,A     ; Current base in B
  PUSHD B      ; Push it to the stack
  SYSCALL #SYSCALL_DOT
  JP NEXT

MODE:
  .DATA 4
  .SDATA "MODE"
  .DATA DOT
MODE_WA:
  .DATA MODE_CA
MODE_CA:
  MOVIL A,%MODE
  PUSHD A
  JP NEXT

BASE:
  .DATA 4
  .SDATA "BASE"
  .DATA MODE
BASE_WA: .DATA BASE_CA
BASE_CA:
  MOVIL A,%BASE
  PUSHD A
  JP NEXT

HEX:
  .DATA 3
  .SDATA "HEX"
  .DATA BASE
HEX_WA: .DATA HEX_CA
HEX_CA:
  MOVIL A,%BASE
  MOVBI 0x10
  ST A,B
  JP NEXT

DECIMAL:
  .DATA 7
  .SDATA "DECIMAL"
  .DATA HEX
DECIMAL_WA: .DATA DECIMAL_CA
DECIMAL_CA:
  MOVIL A,%BASE
  MOVBI 10
  ST B,A
  JP NEXT

AT:
  .DATA 1
  .SDATA "@"
  .DATA DECIMAL
AT_WA:
  .DATA AT_CA
AT_CA:
  POPD A
  LD A,A
  PUSHD A
  JP NEXT

STORE:
  .DATA 1
  .SDATA "!"
  .DATA AT
STORE_WA:
  .DATA STORE_CA
STORE_CA:
  POPD A
  POPD B
  ST A,B
  JP NEXT

PLUS:
  .DATA 1
  .SDATA "+"
  .DATA STORE
PLUS_WA: 
  .DATA PLUS_CA
PLUS_CA:
  POPD A
  POPD B
  ADD A,B
  PUSHD A
  JP NEXT

PLUS_STORE:
  .DATA 2
  .SDATA "+!"
  .DATA PLUS
PLUS_STORE_WA: .DATA PLUS_STORE_CA
PLUS_STORE_CA:
  POPD A
  POPD B
  LD R0,A
  ADD R0,B
  ST A,R0
  JP NEXT

MINUS:
  .DATA 1
  .SDATA "-"
  .DATA PLUS_STORE
MINUS_WA: 
  .DATA MINUS_CA
MINUS_CA:
  POPD A
  POPD B
  SUB B,A
  PUSHD B
  JP NEXT

TIMES:
  .DATA 1
  .SDATA "*"
  .DATA MINUS
TIMES_WA: 
  .DATA TIMES_CA
TIMES_CA:
  POPD A
  POPD B
  MUL B,A
  PUSHD B
  JP NEXT

DIV:
  .DATA 1
  .SDATA "/"
  .DATA TIMES
DIV_WA: 
  .DATA DIV_CA
DIV_CA:
  POPD A
  POPD B
  DIV B,A
  PUSHD B
  JP NEXT

AND:
  .DATA 3
  .SDATA "AND"
  .DATA DIV
AND_WA: 
  .DATA AND_CA
AND_CA:
  POPD A
  POPD B
  AND B,A
  PUSHD B
  JP NEXT

OR:
  .DATA 2
  .SDATA "OR"
  .DATA AND
OR_WA: .DATA OR_CA
OR_CA:
  POPD A
  POPD B
  OR A,B
  PUSHD A
  JP NEXT

NOT:
  .DATA 3
  .SDATA "NOT"
  .DATA OR
NOT_WA: .DATA NOT_CA
NOT_CA:
  POPD A
  CMPI A,0
  JR[Z] NOT_IS_ZERO
  MOVI A,0 ; invert
  PUSHD A
  JP NEXT
NOT_IS_ZERO:
  MOVI A,1
  PUSHD A
  JP NEXT

INVERT:
  .DATA 6
  .SDATA "INVERT"
  .DATA NOT
INVERT_WA: .DATA INVERT_CA
INVERT_CA:
  POPD A
  NOT A
  PUSHD A
  JP NEXT

EQUALS:
  .DATA 1
  .SDATA "="
  .DATA INVERT
EQUALS_WA: .DATA EQUALS_CA
EQUALS_CA:
  POPD A
  POPD B
  CMP A,B
  JR[NZ] NOT_EQUALS
  MOVI A,1
  PUSHD A
  JP NEXT
NOT_EQUALS:
  MOVI A,0
  PUSHD A
  JP NEXT

SL:
  .DATA 2
  .SDATA "<<"
  .DATA EQUALS
SL_WA: .DATA SL_CA
SL_CA:
  POPD B
  POPD A
  SL A,B
  PUSHD A
  JP NEXT

SR:
  .DATA 2
  .SDATA ">>"
  .DATA SL
SR_WA: .DATA SR_CA
SR_CA:
  POPD B
  POPD A
  SR A,B
  PUSHD A
  JP NEXT

ALIGN:
  .DATA 5
  .SDATA "ALIGN"
  .DATA SR
ALIGN_WA:
  .DATA ALIGN_CA
ALIGN_CA:
  POPD A
  ADDI A,1
  CLRI A,0
  PUSHD A
  JP NEXT

DUP:
  .DATA 3
  .SDATA "DUP"
  .DATA ALIGN
DUP_WA: .DATA DUP_CA
DUP_CA:
  POPD A
  PUSHD A
  PUSHD A
  JP NEXT

; ( 1 2 3 -- 2 3 1 )
ROT:
  .DATA 3
  .SDATA "ROT"
  .DATA DUP
ROT_WA: .DATA ROT_CA
ROT_CA:
  POPD R3
  POPD R2
  POPD R1
  PUSHD R2
  PUSHD R3
  PUSHD R1
  JP NEXT

; ( 1 2 3 -- 3 1 2 )
RROT:
  .DATA 4
  .SDATA "RROT"
  .DATA DUP
RROT_WA: .DATA RROT_CA
RROT_CA:
  POPD R3
  POPD R2
  POPD R1
  PUSHD R3
  PUSHD R1
  PUSHD R2
  JP NEXT

; ( xn .. x0 u -- xn .. x0 xu)
PICK:
  .DATA 4
  .SDATA "PICK"
  .DATA RROT
PICK_WA: .DATA PICK_CA
PICK_CA:
  POPD R0
  MOV R1,SP
  ADD R1,R0
  ADD R1,R0 ; *2 to word-align
  ADDI R1,2 ; 
  LD R2,R1
  PUSHD R2
  JP NEXT

SWAP:
  .DATA 4
  .SDATA "SWAP"
  .DATA PICK
SWAP_WA: .DATA SWAP_CA
SWAP_CA:
  POPD R0
  POPD R1
  PUSHD R0
  PUSHD R1
  JP NEXT

DROP:
  .DATA 4
  .SDATA "DROP"
  .DATA SWAP
DROP_WA:
  .DATA DROP_CA
DROP_CA:
  POPD A
  JP NEXT

OVER:
  .DATA 4
  .SDATA "OVER"
  .DATA DROP
OVER_WA: .DATA OVER_CA
OVER_CA:
  POPD A
  POPD B
  PUSHD B
  PUSHD A
  PUSHD B
  JP NEXT

COMMA:
  .DATA 1
  .SDATA ","
  .DATA OVER
COMMA_WA: .DATA COMMA_CA
COMMA_CA:
  POPD A
  MOVIL B,%DICTIONARY_POINTER
  LD B,B
  ST B,A
  ADDI B,2
  MOVIL A,%DICTIONARY_POINTER
  ST A,B
  JP NEXT

FLUSH:  ; Flush stdout
  .DATA 5
  .SDATA "FLUSH"
  .DATA COMMA
FLUSH_WA:
    .DATA FLUSH_CA
FLUSH_CA:
    SYSCALL #SYSCALL_FLUSH
    JP NEXT



ASPACE:           ; Forth word to push a space char to the stack
  .DATA 6
  .SDATA "ASPACE"
  .DATA FLUSH
ASPACE_WA:
  .DATA ASPACE_CA
ASPACE_CA:
  MOVAI 0x20
  PUSHD A
  JP NEXT

SCODE:
  .DATA 5
  .SDATA "SCODE"
  .DATA ASPACE
SCODE_WA:
  .DATA COLON
SCODE_CA:
  .DATA RFROM_WA
  .DATA CASTORE_WA
  .DATA SEMI

RFROM:
  .DATA 2
  .SDATA "R>"
  .DATA SCODE
RFROM_WA:
  .DATA RFROM_CA
RFROM_CA:
  POPR A
  PUSHD A
  JP NEXT

ENTRY:
  .DATA 5
  .SDATA "ENTRY"
  .DATA RFROM
ENTRY_WA:
  .DATA COLON
ENTRY_CA:
  .DATA CURRENT_WA
  .DATA AT_WA
  .DATA AT_WA
  .DATA SEMI

DP:
  .DATA 2
  .SDATA "DP"
  .DATA ENTRY
DP_WA: .DATA DP_CA
DP_CA:
  MOVIL A,%DICTIONARY_POINTER
  PUSHD A
  JP NEXT

DP_STORE:
  .DATA 3
  .SDATA "DP!"
  .DATA DP
DP_STORE_WA: .DATA DP_STORE_CA
DP_STORE_CA:
  MOVIL A,%DICTIONARY_POINTER
  POPD B
  ST A,B
  JP NEXT

TWO_PLUS:
  .DATA 2
  .SDATA "2+"
  .DATA DP_STORE
TWO_PLUS_WA: .DATA TWO_PLUS_CA
TWO_PLUS_CA:
  POPD A
  ADDI A,2
  PUSHD A
  JP NEXT

WA_TO_LA:
  .DATA 5
  .SDATA "WA>LA"
  .DATA TWO_PLUS
WA_TO_LA_WA: .DATA WA_TO_LA_CA
WA_TO_LA_CA:
  ; ( wa -- ca )
  POPD A
  MOV B,A
  LD A,B ; Length to A
  ADD A,B
  ADDI A,3
  CLRI A,0 ; Word-align
  PUSHD A
  JP NEXT

WA_TO_CA:
  .DATA 5
  .SDATA "WA>CA"
  .DATA WA_TO_LA
WA_TO_CA_WA: .DATA WA_TO_CA_CA
WA_TO_CA_CA:
  ; ( wa -- ca )
  POPD A
  MOV B,A
  LD A,B ; Length to A
  ADD A,B
  ADDI A,5
  CLRI A,0 ; Word-align
  PUSHD A
  JP NEXT

WA_TO_CB:
  .DATA 5
  .SDATA "WA>CB"
  .DATA WA_TO_CA
WA_TO_CB_WA: .DATA WA_TO_CB_CA
WA_TO_CB_CA:
  ; ( wa -- cb )
  POPD A
  MOV B,A
  LD A,B ; Length to A
  ADD A,B
  ADDI A,7
  CLRI A,0 ; Word-align
  PUSHD A
  JP NEXT


CREATE:
  .DATA 6
  .SDATA "CREATE"
  .DATA WA_TO_CB
CREATE_WA: .DATA COLON
CREATE_CA:
  .DATA ENTRY_WA      ;( -- WA of last entry )
  .DATA ASPACE_WA
  .DATA TOKEN_WA      ;( .. -- WA LEN )
  .DATA HERE_WA       ;( .. -- WA LEN HERE )
  .DATA CURRENT_WA    
  .DATA AT_WA         ;( .. -- WA LEN HERE [ADDR OF CURRENT VOCAB] )
  .DATA STORE_WA      ;( .. -- WA LEN )
  ; Bump the DP by the length of the token plus two for the length itself
  .DATA HERE_WA
  .DATA PLUS_WA       ; Add the length
  .DATA STAR_HASH_WA
  .DATA 2
  .DATA PLUS_WA
  .DATA ALIGN_WA      ; now points to link
  ; Point the DP at the new LA
  .DATA DP_WA
  .DATA STORE_WA      ; update the dictionary pointer
  ; Compile ENTRY
  .DATA COMMA_WA      ; point to the last entry
  .DATA HERE_WA
  .DATA TWO_PLUS_WA
  .DATA COMMA_WA      ; And point CA to here
  .DATA SEMI

SEMICOLON:
  .DATA 0xC001 ; This is an executive word
  .SDATA ";"
  .DATA CREATE
SEMICOLON_WA: .DATA COLON
SEMICOLON_CA:
  .DATA STAR_HASH_WA .DATA SEMI
  .DATA COMMA_WA

  .DATA STAR_HASH_WA
  .DATA 0
  .DATA MODE_WA
  .DATA STORE_WA
  .DATA SEMI

CURRENT:
  .DATA 7
  .SDATA "CURRENT"
  .DATA SEMICOLON
CURRENT_WA:
  .DATA CURRENT_CA
CURRENT_CA:
  MOVIL A,%CURRENT
  PUSHD A
  JP NEXT

CONTEXT:
  .DATA 7
  .SDATA "CONTEXT"
  .DATA CURRENT
CONTEXT_WA:
  .DATA CONTEXT_CA
CONTEXT_CA:
  MOVIL A,%CONTEXT
  PUSHD A
  JP NEXT

CASTORE:
  .DATA 3
  .SDATA "CA!"
  .DATA CONTEXT
CASTORE_WA:
  .DATA COLON
CASTORE_CA:
  .DATA ENTRY_WA
  .DATA WA_TO_CA_WA
  .DATA STORE_WA
  .DATA SEMI

HERE:
  .DATA 4
  .SDATA "HERE"
  .DATA CASTORE
HERE_WA: .DATA HERE_CA
HERE_CA:
  MOVIL A,%DICTIONARY_POINTER
  LD B,A
  PUSHD B
  JP NEXT

CONSTANT:
  .DATA 0x4008       ; This is a runtime only word
  .SDATA "CONSTANT"
  .DATA HERE
CONSTANT_WA:
  .DATA COLON
CONSTANT_CA:
  .DATA CREATE_WA
  .DATA COMMA_WA
  .DATA SCODE_WA



Q_EXECUTE:
  .DATA 8
  .SDATA "?EXECUTE"
  .DATA CONSTANT
Q_EXECUTE_WA:
  .DATA COLON

  ; This routine expects a NA on the stack from a successful SEARCH
  ; Words can be flagged with the immediate [15] and run-time [14] bits
  ; 00 - Normal word. Mode = 0 => Execute   Mode = 1 => Compile
  ; 01 - Runtime only Mode = 0 => Execute   Mode = 1 => Error
  ; 10 - Immediate    Mode = 0 => Error     Mode = 1 => Execute
  ; 11 - Executive    Mode = 0 => Execute   Mode = 1 => Execute
  ;
  ; DUP
  ; @ 13 >>
  ; MODE @
  ; OR
  ; ( addr flags -- )
  ; 
Q_EXECUTE_CA:
  .DATA DUP_WA           ; ( addr - addr addr )
  .DATA AT_WA            ; (addr addr -- addr len )
  .DATA STAR_HASH_WA
  .DATA 13
  .DATA SR_WA            ; Get the bits in 2,1 (addr len -- addr (len>>13))
  .DATA MODE_WA
  .DATA AT_WA            ; Get the mode bit
  .DATA OR_WA            ; AS bit 0

  .DATA DUP_WA           ; ( addr bits -- addr bits bits )
  .DATA STAR_HASH_WA 
  .DATA 0                ; ( . -- addr bits bits 0 )
  .DATA EQUALS_WA        ; ( . -- addr bits EQ )
  .DATA STAR_IF_WA       ; ( . -- addr bits )
  .DATA Q_EXECUTE_1
Q_EXECUTE_NORMAL:
    .DATA DROP_WA        ; ( . -- addr )
    .DATA EXECUTE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_1:  
  .DATA DUP_WA
  .DATA STAR_HASH_WA .DATA 1
  .DATA EQUALS_WA
  .DATA STAR_IF_WA 
  .DATA Q_EXECUTE_2
Q_EXECUTE_COMPILE:
    .DATA DROP_WA
    .DATA WA_TO_CA_WA
    .DATA COMMA_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_2:
  .DATA DUP_WA
  .DATA STAR_HASH_WA .DATA 2
  .DATA EQUALS_WA
  .DATA STAR_IF_WA 
  .DATA Q_EXECUTE_3
Q_EXECUTE_RUNTIME:
    .DATA DROP_WA
    .DATA EXECUTE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_3:
  .DATA DUP_WA
  .DATA STAR_HASH_WA .DATA 3
  .DATA EQUALS_WA
  .DATA STAR_IF_WA 
  .DATA Q_EXECUTE_4
Q_EXECUTE_RUNTIME_NOT_ALLOWED:
    .DATA STAR_HASH_WA
    .DATA MSG_RUNTIME_ONLY
    .DATA TYPE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_FAILED

Q_EXECUTE_4:
  .DATA DUP_WA
  .DATA STAR_HASH_WA .DATA 4
  .DATA EQUALS_WA
  .DATA STAR_IF_WA 
  .DATA Q_EXECUTE_EXECUTIVE
Q_EXECUTE_COMPILETIME_NOT_ALLOWED:
   .DATA STAR_HASH_WA
    .DATA MSG_COMPILE_ONLY
    .DATA TYPE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_FAILED

Q_EXECUTE_EXECUTIVE:
  .DATA DROP_WA
  .DATA EXECUTE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_ERROR:
  .DATA STAR_HASH_WA 
  .DATA MSG_SYSTEM_ERROR 
  .DATA TYPE_WA

Q_EXECUTE_FAILED:
  .DATA STAR_HASH_WA
  .DATA 0
  .DATA STAR_ELSE_WA
  .DATA Q_EXECUTE_EXIT

Q_EXECUTE_DONE:
  .DATA STAR_STACK_WA
  .DATA STAR_HASH_WA
  .DATA 1
Q_EXECUTE_EXIT:
  .DATA SEMI

; ( dp -- num true ) | ( dp -- false )
Q_NUMBER:
  .DATA 7
  .SDATA "?NUMBER"
  .DATA Q_EXECUTE
Q_NUMBER_WA: .DATA Q_NUMBER_CA
Q_NUMBER_CA:
  MOVIL A,%BASE
  PUSHD A
  SYSCALL #SYSCALL_NUMBER
  JP NEXT

EXECUTE:
  .DATA 7
  .SDATA "EXECUTE"
  .DATA Q_NUMBER
EXECUTE_WA:
  .DATA EXECUTE_CA
EXECUTE_CA:
  POPD WA ; points to the NA
  MOV A,WA
  LD A,A ; Get the length
  MOVIL B,0x3fff ; flags
  AND A,B
  ADD WA,A
  ADDI WA,5 ; Skip over the name and link
  CLRI WA,0 ; WOrd-align
  JP RUN

; ( sep - tokenLength )
TOKEN:
  .DATA 5
  .SDATA "TOKEN"
  .DATA EXECUTE
TOKEN_WA:
  .DATA TOKEN_CA
TOKEN_CA:
  ; Scans for tokens from the input line buffer
  ; The buffer is set up by INLINE
  ; LEN PTR CHARS
  ; Register usage:
  ; A - a character
  ; B - The separator
  ; R0 - Current pointer
  ; R1 - Length of this token
  ; R2 - End of the buffer
  ; R3 - scratch
  ; 

  ; Get the separator
  POPD B

  MOVI R1,0 ; Token length
  MOVI R3,0

  MOVIL R0,%LBUF_IDX
  LD R0,R0

  MOVIL R2,%LBUF_END
  LD R2,R2

  CMP R2,R0
  ; at or past the end of the buffer already?
  JR[Z] TOKEN_END
  JR[C] TOKEN_END

  MOVAI 0x20   ; Put a space in A
  CMP B,A     ; Is the separator a space?
  JR[NZ] TOKEN_TOK  ; No, so start seeking

TOKEN_SKIP:         ; Skip leading spaces
  CMP R2,R0         ; At the end of the buffer?
  JR[Z] TOKEN_DONE
  LD_B A,R0          ; Get the next char
  CMP A,B           ; Is it the separator?
  JR[NZ] TOKEN_TOK
  ADDI R0,1
  JR TOKEN_SKIP  ;

TOKEN_TOK:          ; Start searching for the end token here
  CMP R2,R0
  JR[Z] TOKEN_DONE  ; Did we get to the end of the buffer?
  LD_B A,R0
  CMP A,B
  JR[Z] TOKEN_DONE
  CMPAI 0x0A        ; Or is this a carriage return?
  JR[Z] TOKEN_DONE
  ADDI R0,1         ; idx++
  ADDI R1,1         ; len++
  JR TOKEN_TOK

TOKEN_DONE:
  CMPI R1,0
  JR[Z] TOKEN_END
  ; Move the token to the end of the dictionary
  ; So ?SEARCH can access it
  MOVIL A,%DICTIONARY_POINTER
  LD B,A    ; B Points to the dictionary
  ST B,R1   ; Save the length
  MOV R3,R1 ; Save for later
  ADDI B,2  ; Bump the dictionary pointer
  SUB R0,R1 ; Reset the buffer pointer

TOKEN_MOVE:
  LD_B A,R0
  ST_B B,A
  ADDI R0,1 ; Bump the buffer pointer
  ADDI B,1 ; Bump the dictionary pointer
  ADDI R1,-1 ; Decrement the length
  JR[NZ] TOKEN_MOVE
  ;
  ; And we're done
  ; Save the current pointer
  MOVIL A,%LBUF_IDX
  ADDI R0,1 ; Point past the terminator
  ST A,R0

TOKEN_END:
  ; Leave the token length on the stack
  PUSHD R3
  JP NEXT

; INLINE - Read a line from the terminal
; Move it to the line buffer
; We'll just code this as a syscall for now 
; Returns a flag on the stack - false if no characters were read
; ( bufferAddress -- flag )
INLINE:
    .DATA 6
    .SDATA "INLINE"
    .DATA TOKEN
INLINE_WA:
    .DATA INLINE_CA
INLINE_CA:
    ; Put the address of the line buffer on the stack
    MOVIL A,%LBUF_IDX
    PUSHD A
    SYSCALL #SYSCALL_INLINE
    JP NEXT

; Push the literal following this word to the stack
; and jump over it
STAR_HASH:
    .DATA 2
    .SDATA "*#"
    .DATA INLINE ; Link to INLINE
STAR_HASH_WA:
    .DATA STAR_HASH_CA
STAR_HASH_CA:
    LD A,I
    PUSHD A
    ADDI I,2
    JP NEXT

BEGIN:
  .DATA 0x8005
  .SDATA "BEGIN"
  .DATA STAR_HASH
BEGIN_WA: .DATA BEGIN_CA
BEGIN_CA:
  MOVIL A,%DICTIONARY_POINTER
  LD A,A ; current DP
  PUSHD A
  JP NEXT

UNTIL:
  .DATA 0x8005
  .SDATA "UNTIL"
  .DATA BEGIN
UNTIL_WA: .DATA UNTIL_CA
UNTIL_CA:
  POPD R0 ; loop address
  MOVIL R1,STAR_UNTIL_WA
  MOVIL R2,%DICTIONARY_POINTER
  LD R3,R2
  ; Compile *UNTIL
  ST R3,R1
  ADDI R3,2
  ST R3,R0
  ADDI R3,2
  ST R2,R3
  JP NEXT

STAR_UNTIL:
  .DATA 0x4006
  .SDATA "*UNTIL"
  .DATA UNTIL
STAR_UNTIL_WA: .DATA STAR_UNTIL_CA
STAR_UNTIL_CA:
  POPD A ; get the flag
  CMPI A,0
  JR[NZ] STAR_UNTIL_DONE
  LD I,I ; Jump back to the begin
STAR_UNTIL_DONE:
  JP NEXT


IF:
  .DATA 0x8002 ; Compile-time only
  .SDATA "IF"
  .DATA STAR_UNTIL
IF_WA:
  .DATA IF_CA
IF_CA:
; R0 - Address of *IF
; R1 - Address of DP
; R2 - value of DP
  MOVIL R0,STAR_IF_WA
  MOVIL R1,%DICTIONARY_POINTER
  LD R2,R1
  ST R2,R0 ; Compile *IF
  ADDI R2,2 ; point to location of jump word
  PUSHD R2  ; save the location
  ADDI R2,2 ; next location for the definition
  ST R1,R2
  JP NEXT

ELSE:
  .DATA 0x8004
  .SDATA "ELSE"
  .DATA IF
ELSE_WA:
  .DATA ELSE_CA
ELSE_CA:
  ; TOS contains the location of the word to use for the IF relative jump
  ; R0 - Address of DP
  ; R1 - Value of DP (HERE)
  ; R2 - location of jump address
  POPD R2
  MOVIL R0,%DICTIONARY_POINTER
  LD R1,R0
  ADDI R1,4
  ST R2,R1   ; Save it in the jump location
  MOVIL A,STAR_ELSE_WA
  ADDI R1,-4
  ST R1,A  ; Compile *ELSE to the current definition
  ADDI R1,2
  PUSHD R1
  ADDI R1,2
  ST R0,R1
  JP NEXT

THEN:
  .DATA 0x8004
  .SDATA "THEN"
  .DATA ELSE
THEN_WA:
  .DATA THEN_CA
THEN_CA:
  ; TOS contains the location of the word to use for the IF  jump
  POPD A
  MOVIL B,%DICTIONARY_POINTER ; Current dictionary location to B
  LD B,B
  ST A,B   ; Save it in the jump location
  JP NEXT  

STAR_IF:
  .DATA 0x4003 ; run-time only
  .SDATA "*IF"
  .DATA THEN
STAR_IF_WA:
  .DATA STAR_IF_CA
STAR_IF_CA:
  POPD A ; get the flag
  CMPI A,0
  JR[Z] STAR_ELSE_CA
  ADDI I,2
  JP NEXT

STAR_ELSE:
  .DATA 0x4005
  .SDATA "*ELSE"
  .DATA STAR_IF
STAR_ELSE_WA:
  .DATA STAR_ELSE_CA
STAR_ELSE_CA:
  LD I,I
  JP NEXT


; The REPL
; START_RESTART
; HELLO
; OUTER_LOOP: BEGIN
  ; OUTER_INLINE_LOOP:
  ; BEGIN
  ;   INLINE
  ; UNTIL - Loop till we have tokens
  ;
  ; BEGIN
  ;   ASPACE
  ;   TOKEN
  ;   WHILE        - token to process?
  ;     ?SEARCH
  ;     IF
  ;       ?EXECUTE
  ;     ELSE
  ;       ?NUMBER
  ;       NOT IF
  ;         QUESTION
  ;       ENDIF
  ;     ENDIF
  ;   END
; END

Q_SP:
  .DATA 3
  .SDATA "?SP"
  .DATA STAR_ELSE
Q_SP_WA: .DATA Q_SP_CA
Q_SP_CA:
  PUSHD SP
  JP NEXT

; Patch - fixup the dictionary to remove any half-compiled words
PATCH:
  .DATA 5
  .SDATA "PATCH"
  .DATA Q_SP
PATCH_WA: .DATA COLON
PATCH_CA:
  .DATA MODE_WA
  .DATA AT_WA
  .DATA STAR_IF_WA
  .DATA PATCH_DONE
  .DATA CURRENT_WA
  .DATA AT_WA .DATA AT_WA
  .DATA DUP_WA
  .DATA DP_STORE_WA ; Reset the dictionary pointer
  .DATA WA_TO_LA_WA .DATA AT_WA
  .DATA CURRENT_WA .DATA AT_WA .DATA STORE_WA
PATCH_DONE:
  .DATA RESTART_WA
  .DATA SEMI

OUTER:
    .DATA 5
    .SDATA "OUTER"
    .DATA PATCH
OUTER_WA:
    .DATA COLON
OUTER_CA:
    .DATA STAR_HASH_WA
    .DATA MSG_HELLO
    .DATA TYPE_WA

OUTER_LOOP:
    .DATA STAR_HASH_WA
    .DATA MSG_PROMPT
    .DATA TYPE_WA
    .DATA INLINE_WA
    .DATA STAR_IF_WA
    .DATA OUTER_LOOP

OUTER_TOKEN_LOOP:
    .DATA ASPACE_WA
    .DATA TOKEN_WA
    .DATA STAR_IF_WA
    .DATA OUTER_LOOP ; Just jump back to get another line if no more tokens
    .DATA SEARCH_WA
    .DATA STAR_IF_WA
    .DATA OUTER_NOT_A_WORD
    .DATA Q_EXECUTE_WA ; If the execute failed, discard the rest of the line
    .DATA NOT_WA
    .DATA STAR_IF_WA
    .DATA OUTER_TOKEN_EXECUTED
    .DATA PATCH_WA
    .DATA STAR_ELSE_WA
    .DATA OUTER_LOOP

OUTER_TOKEN_EXECUTED:
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP

OUTER_NOT_A_WORD:
    .DATA Q_NUMBER_WA
    .DATA STAR_IF_WA
    .DATA OUTER_NOT_A_NUMBER
    ; Number is on the stack. 
    ; If we're in compile mode, enclose it
    .DATA MODE_WA
    .DATA AT_WA
    .DATA STAR_IF_WA
    .DATA OUTER_NUM_TO_STACK
    .DATA STAR_HASH_WA
    .DATA STAR_HASH_WA ; push the literal handler to the stack
    .DATA COMMA_WA     ; enclose it
    .DATA COMMA_WA     ; and he number

OUTER_NUM_TO_STACK:
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP ; Leave the number on the stack and loop

OUTER_NOT_A_NUMBER:
    .DATA STAR_HASH_WA
    .DATA MSG_UNKNOWN_TOKEN
    .DATA TYPE_WA
    .DATA DP_WA
    .DATA AT_WA
    .DATA TYPE_WA
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP

    .DATA SEMI

TRAP:               ; Used for debugging. Dumps the contents of the registers
                    ; But for now it just says Hello World!
  .DATA 4
  .SDATA "TRAP"
  .DATA OUTER
TRAP_WA:
  .DATA TRAP_CA
TRAP_CA:
  MOV A,PC
  ADDAI 4
  JR TRAP1
  .DATA 14
  .SDATA "Hello World!"
TRAP1:
  PUSHD A            ; Push the address of the string
  SYSCALL #SYSCALL_TYPE
  HALT
  JP NEXT

DOT_WORD:
  .DATA 5
  .SDATA ".WORD"
  .DATA TRAP
DOT_WORD_WA: .DATA DOT_WORD_CA
DOT_WORD_CA:
  ; ( wordAddr -- )
  ; R0 - addr -> idx
  ; R1 - len
  POPD R0
  LD R1,R0 ; Len to R1
  CLRI R1,15
  CLRI R1,14
  ADDI R0,2 ; point to word
DOT_WORD_LOOP:
  LD_B R2,R0
  PUSHD R2
  SYSCALL #SYSCALL_PUTC
  ADDI R0,1
  ADDI R1,-1
  JR[NZ] DOT_WORD_LOOP
  JP NEXT

NEXT_WORD:
  .DATA 9
  .SDATA "NEXT-WORD"
  .DATA DOT_WORD
NEXT_WORD_WA: .DATA COLON
NEXT_WORD_CA:
  ; ( addr -- addr )
  ; DUP @ + 2+ ALIGN @
  .DATA DUP_WA
  .DATA AT_WA
  .DATA STAR_HASH_WA
  .DATA 0x3fff
  .DATA AND_WA
  .DATA PLUS_WA
  .DATA TWO_PLUS_WA
  .DATA ALIGN_WA
  .DATA AT_WA
  .DATA SEMI


CRET:
  .DATA 4
  .SDATA "CRET"
  .DATA NEXT_WORD
CRET_WA: .DATA CRET_CA
CRET_CA:
  MOVIL A,0x0a
  SYSCALL #SYSCALL_PUTC
  JP NEXT

WORDS:
; List all the defined words
  .DATA 5
  .SDATA "WORDS"
  .DATA CRET
WORDS_WA: .DATA COLON
WORDS_CA:
  .DATA CURRENT_WA
  .DATA AT_WA
  .DATA AT_WA
WORDS_LOOP:
  .DATA DUP_WA ; ( addr -- addr addr)
  .DATA DOT_WORD_WA ; ( .. -- addr len )
  .DATA ASPACE_WA
  .DATA EMIT_WA
  ; Now skip up to the next word
  .DATA NEXT_WORD_WA
  .DATA DUP_WA
  .DATA NOT_WA
  .DATA STAR_IF_WA
  .DATA WORDS_LOOP
  .DATA DROP_WA
  .DATA SEMI


; Search the dictionary for the current token
; Searches the vocabulary pointed to by CURRENT
; Return the WA on the stack if a match is found, or zero
SEARCH:
    .DATA 6
    .SDATA "SEARCH"
    .DATA WORDS
SEARCH_WA:
    .DATA SEARCH_CA
SEARCH_CA:

  ; DP in I R0
  MOVIL A,%DICTIONARY_POINTER
  LD R0,A

  ; Token length in WA R1
  LD R1,R0
  ; Bump I to point to first char
  ADDI R0,2
  MOV R6,R0

  ; Use CA R2 to point to the current word
  MOVIL R2,%CURRENT
  LD R2,R2
  LD R2,R2

SEARCH_NEXT:
  MOV R0,R6
  LD A,R2  ; get the length of the word
  CLRI A,15 ; Clear the immediate bit if it's set
  CLRI A,14 ; Clear the run-time bit if it's set
  CMP A,R1 ; Same as the token?
  JR[Z] SEARCH_COMPARE
  ; Not the same, move to the next word
  ADD R2,A
  ADDI R2,3 ; CA now points to the link address
  CLRI R2,0  ; make sure it's a word address
  LD R2,R2 ; Now point to the next word
  AND R2,R2 ; Is it zero?
  JR[NZ] SEARCH_NEXT

SEARCH_EXIT:
  ; Not found so leave the bufferr address zero on the stack and exit
  MOVIL A,%DICTIONARY_POINTER
  LD R0,A
  PUSHD R0
  MOVI A,0
  PUSHD A
  JP NEXT

SEARCH_COMPARE:
  ; Test the strings for equality
  MOV R3,R2 ; Save the current word location
  ADDI R2,2 ; Point to the string
  MOV R4,R1 ; Length to R4 for the counter

SEARCH_COMPARE_LOOP:
  LD_B A,R0
  LD_B B,R2
  CMP A,B
  JR[NZ] SEARCH_COMPARE_FAIL
  ADDI R2,1
  ADDI R0,1
  ADDI R4,-1
  JR[NZ] SEARCH_COMPARE_LOOP
SEARCH_COMPARE_FOUND:
  PUSHD R3 ; Name address to the stack
  MOVI A,1
  PUSHD A   ; And a true flag
  JP NEXT

SEARCH_COMPARE_FAIL:
  LD A,R3  ; get the length of the word
  CLRI A,15 ; Clear the immediate bit if it's set
  CLRI A,14 ; Clear the run-time bit if it's set
  ; Move to the next word
  ADD R3,A
  ADDI R3,3 ; CA now points to the link address
  CLRI R3,0  ; make sure it's a word address
  LD R2,R3 ; Now point to the next word
  AND R2,R2 ; Is it zero?
  JR[NZ] SEARCH_NEXT
  JR SEARCH_EXIT

TWO_DOTS: ; This is the public COLON routine
  .DATA 1
  .SDATA ":"
  .DATA SEARCH
TWO_DOTS_WA: .DATA COLON
TWO_DOTS_CA:
  .DATA CURRENT_WA
  .DATA AT_WA
  .DATA CONTEXT_WA
  .DATA STORE_WA
  .DATA CREATE_WA
  .DATA STAR_HASH_WA
  .DATA COLON
  .DATA CASTORE_WA
  .DATA STAR_HASH_WA
  .DATA 1
  .DATA MODE_WA
  .DATA STORE_WA
  .DATA SEMI


LIT:
  .DATA 0x8002 ; IMMEDIATE
  .SDATA ".\""
  .DATA TWO_DOTS
LIT_WA: .DATA COLON
LIT_CA:
  .DATA BREAKPOINT_WA
  .DATA STAR_HASH_WA .DATA STAR_LIT_WA 
  .DATA COMMA_WA
  .DATA STAR_HASH_WA .SDATA "\""
  .DATA TOKEN_WA
  .DATA TWO_PLUS_WA
  .DATA ALIGN_WA
  .DATA DP_WA
  .DATA PLUS_STORE_WA
  .DATA SEMI

CORE_VOCABULARY:
STATE:
  .DATA 5
  .SDATA "STATE"
  .DATA LIT
STATE_WA:
  .DATA STATE_CA
STATE_CA:
  .ALIAS A,STATE
  MOVIL STATE,%STATE
  PUSHD STATE
  JP NEXT

DICTIONARY_END:
    .DATA 00
