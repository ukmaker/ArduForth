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
#BASE_DEC: 0
#BASE_HEX: 1
#BASE_BIN: 2

; The predefined SYSCALLs
#SYSCALL_PRINTC: 0
#SYSCALL_TYPE: 1
#SYSCALL_DOT: 2
#SYSCALL_GETC: 3
#SYSCALL_PUTC: 4
#SYSCALL_INLINE: 5
#SYSCALL_FLUSH: 6
#SYSCALL_NUMBER: 7

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
  MOVIL SP,#SPTOP
  MOVIL RS,#RSTOP
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
  LD CA,WA
  ADDI WA,2
  MOV PC,CA

MESSAGES: ; SYSTEM MESSAGES LIVE HERE
MSG_HELLO:          .DATA 22 .SDATA "Hello! I'm a TIL :-) >"
MSG_UNKNOWN_TOKEN:  .DATA 13 .SDATA "Unknown token"
MSG_RUNTIME_ONLY:   .DATA 39 .SDATA "Compile-time words forbidden at runtime"
MSG_COMPILE_ONLY:   .DATA 39 .SDATA "Runtime words forbidden at compile-time"
MSG_SYSTEM_ERROR:   .DATA 12 .SDATA "System error"
MSG_WORD_NOT_FOUND: .DATA 14 .SDATA "Word not found"
MSG_PROMPT:         .DATA  6 .SDATA " OK >>"

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

VOCABULARY:
  .DATA 10
  .SDATA "VOCABULARY"
  .DATA DOES
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


TYPE:
  .DATA 4           ; Length of TYPE
  $TYPE: "TYPE"     ; And the string's characters
  .DATA CORE   
TYPE_WA:
  .DATA TYPE_CA    ; This is the word address of TYPE
TYPE_CA:             ; The Code address
  SYSCALL #SYSCALL_TYPE
  JP NEXT

MESSAGE: ; Print a system message ( n -- )
  .DATA 7
  .SDATA "MESSAGE"
  .DATA TYPE
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

STATE:
  .DATA 5
  .SDATA "STATE"
  .DATA MODE
STATE_WA:
  .DATA STATE_CA
STATE_CA:
  MOVIL A,%STATE
  PUSHD A
  JP NEXT

AT:
  .DATA 1
  .SDATA "@"
  .DATA STATE
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

MINUS:
  .DATA 1
  .SDATA "-"
  .DATA PLUS
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
  NOT A
  PUSHD A
  JP NEXT

EQUALS:
  .DATA 1
  .SDATA "="
  .DATA NOT
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
  .DATA EQUALS
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
  .DATA SL
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

ROT:
  .DATA 3
  .SDATA "ROT"
  .DATA DUP
ROT_WA: .DATA ROT_CA
ROT_CA:
  POPD R0
  POPD R1
  POPD R2
  PUSHD R1
  PUSHD R0
  PUSHD R2
  JP NEXT

SWAP:
  .DATA 4
  .SDATA "SWAP"
  .DATA ROT
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

COMMA:
  .DATA 1
  .SDATA ","
  .DATA DROP
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

TWO_PLUS:
  .DATA 2
  .SDATA "2+"
  .DATA DP
TWO_PLUS_WA: .DATA TWO_PLUS_CA
TWO_PLUS_CA:
  POPD A
  ADDI A,2
  PUSHD A
  JP NEXT

WA_TO_CA:
  .DATA 5
  .SDATA "WA>CA"
  .DATA TWO_PLUS
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
    .DATA STAR_HASH_WA
    .DATA MSG_RUNTIME_ONLY
    .DATA TYPE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_4:
  .DATA DUP_WA
  .DATA STAR_HASH_WA .DATA 4
  .DATA EQUALS_WA
  .DATA STAR_IF_WA 
  .DATA Q_EXECUTE_5
    .DATA STAR_HASH_WA
    .DATA MSG_COMPILE_ONLY
    .DATA TYPE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_5:
  .DATA DROP_WA
  .DATA EXECUTE_WA
  .DATA STAR_ELSE_WA 
  .DATA Q_EXECUTE_DONE

Q_EXECUTE_ERROR:
  .DATA STAR_HASH_WA 
  .DATA MSG_SYSTEM_ERROR 
  .DATA TYPE_WA

Q_EXECUTE_DONE:
  .DATA SEMI

Q_NUMBER:
  .DATA 7
  .SDATA "?NUMBER"
  .DATA Q_EXECUTE
Q_NUMBER_WA: .DATA Q_NUMBER_CA
Q_NUMBER_CA:
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
  ; I - Current pointer
  ; CA - Length of this token
  ; WA - End of the buffer
  ; 
  PUSHR I ; Save I
  PUSHR WA
  PUSHR CA

  MOVI CA,0 ; Token length
  MOV R0,CA

  MOVIL A,%LBUF_IDX
  LD I,A

  MOVIL A,%LBUF_END
  LD WA,A

  CMP I,WA
  ; at the end of the buffer already?
  JR[Z] TOKEN_END

  ; Get the separator
  POPD B
  MOVAI 0x20   ; Put a space in A
  CMP B,A     ; Is the separator a space?
  JR[NZ] TOKEN_TOK  ; No, so start seeking

TOKEN_SKIP:         ; Skip leading spaces
  CMP I,WA    ; At the end of the buffer?
  JR[Z] TOKEN_DONE
  LD_B A,I ;  Get the next char
  CMP A,B      ; Is it the separator?
  JR[NZ] TOKEN_TOK
  ADDI I,1
  JR TOKEN_SKIP  ;

TOKEN_TOK:          ; Start searching for the end token here
  CMP I,WA
  JR[Z] TOKEN_DONE  ; Did we get to the end of the buffer?
  LD_B A,I
  CMP A,B
  JR[Z] TOKEN_DONE
  CMPAI 0x0A        ; Or is this a carriage return?
  JR[Z] TOKEN_DONE
  ADDI I,1
  ADDI CA,1
  JR TOKEN_TOK

TOKEN_DONE:
  CMPI CA,0
  JR[Z] TOKEN_END
  ; Move the token to the end of the dictionary
  ; So ?SEARCH can access it
  MOVIL A,%DICTIONARY_POINTER
  LD B,A ; B Points to the dictionary
  ST B,CA  ; Save the length
  MOV R0,CA ; Save for later
  ADDI B,2 ; Bump the dictionary pointer
  SUB I,CA ; Reset the buffer pointer

TOKEN_MOVE:
  LD_B A,I
  ST_B B,A
  ADDI I,1 ; Bump the buffer pointer
  ADDI B,1 ; Bump the dictionary pointer
  ADDI CA,-1 ; Decrement the length
  JR[NZ] TOKEN_MOVE
  ;
  ; And we're done
  ; Save the current pointer
  MOVIL A,%LBUF_IDX
  ST A,I

TOKEN_END:
  ; retrieve the registers
  POPR CA
  POPR WA
  POPR I
  ; Leave the token length on the stack
  PUSHD R0
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

IF:
  .DATA 0x8002 ; Compile-time only
  .SDATA "IF"
  .DATA STAR_HASH
IF_WA:
  .DATA IF_CA
IF_CA:
  MOVIL A,STAR_IF_WA
  MOVIL B,%DICTIONARY_POINTER
  ST B,A
  LD A,B
  ADDI A,2 ; point to location of jump word
  PUSHD A  ; save the location
  ADDI A,2 ; next location for the definition
  ST B,A
  JP NEXT

ELSE:
  .DATA 0x8004
  .SDATA "ELSE"
  .DATA IF
ELSE_WA:
  .DATA ELSE_CA
ELSE_CA:
  ; TOS contains the location of the word to use for the IF relative jump
  POPD A
  MOVIL B,%DICTIONARY_POINTER
  LD R0,B ; Current dictionary location to B
  SUB R0,A  ; Calculate the word offset
  ST A,R0   ; Save it in the jump location
  MOVIL A,STAR_ELSE_WA
  ST R0,A  ; Compile *ELSE to the current definition
  ADDI R0,2
  ST B,R0
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

OUTER:
    .DATA 5
    .SDATA "OUTER"
    .DATA STAR_ELSE
OUTER_WA:
    .DATA COLON
OUTER_CA:
    .DATA STAR_HASH_WA
    .DATA MSG_HELLO
    .DATA TYPE_WA

OUTER_LOOP:
    .DATA MODE_WA
    .DATA AT_WA
    .DATA DOT_WA
    .DATA STAR_HASH_WA
    .DATA MSG_PROMPT
    .DATA TYPE_WA
    .DATA INLINE_WA
    .DATA NOT_WA
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
    .DATA Q_EXECUTE_WA
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP
OUTER_NOT_A_WORD:
    .DATA Q_NUMBER_WA
    .DATA NOT_WA
    .DATA STAR_IF_WA
    .DATA OUTER_NOT_A_NUMBER
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP ; Leave the number on the stack and loop

OUTER_NOT_A_NUMBER:
    .DATA STAR_HASH_WA
    .DATA MSG_UNKNOWN_TOKEN
    .DATA TYPE_WA
    .DATA STAR_ELSE_WA
    .DATA OUTER_TOKEN_LOOP

    .DATA SEMI

TRAP:               ; Used for debugging. Dumps the contents of the registers
                    ; But for now it just says Hello World!
  .DATA 2
  .SDATA "TR"
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

; Search the dictionary for the current token
; Searches the vocabulary pointed to by CURRENT
; Return the WA on the stack if a match is found, or zero
SEARCH:
    .DATA 6
    .SDATA "SEARCH"
    .DATA TRAP
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

  ; Use CA R2 to point to the current word
  MOVIL R2,%CURRENT
  LD R2,R2
  LD R2,R2

SEARCH_NEXT:
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

SEARCH_COMPARE_LOOP:
  LD_B A,R0
  LD_B B,R2
  CMP A,B
  JR[NZ] SEARCH_COMPARE_FAIL
  ADDI R2,1
  ADDI R0,1
  ADDI R1,-1
  JR[NZ] SEARCH_COMPARE_LOOP
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

CORE_VOCABULARY:
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

DICTIONARY_END:
    .DATA 00
