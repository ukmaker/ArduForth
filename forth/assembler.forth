( An assembler for the ForthVM )
( This is WIP - labels support is not finished )

VOCABULARY ASSEMBLER
ASSEMBLER DEFINITIONS

8 CONSTANT OP-BITS
10 CONSTANT CC-BITS

: REG CONSTANT ;
: CC CC-BITS << CONSTANT ;

0 REG R0
1 REG R1
2 REG R2
3 REG R3
4 REG R4
5 REG R5
6 REG R6
7 REG R7
8 REG A
9 REG B
10 REG I
11 REG FP
12 REG WA
13 REG SP
14 REG RS
15 REG PC

0 CC C]
1 CC Z]
2 CC P]
3 CC M]
4 CC NC]
5 CC NZ]
6 CC NP]
7 CC NM]

1 13 << CONSTANT CC-APPLY-BIT

: 'REG ' DUP 0= IF ." Register name expected" 0 ELSE CA>WA EXECUTE THEN ;
: REGA 'REG 4 << OR ;
: REGB 'REG OR ;

: ARG-EXPECTED-ERROR ." Number expected " ;
: ARG-RANGE-ERROR ." Argument " . ."  is out of range "
    . .&sp; . CRET
;

: ARG &sp; TOKEN TOKEN-LEN 
    IF
        SEARCH 
        IF 
            ?EXECUTE
        ELSE
            ?NUMBER
            IF
                1
            ELSE 
                0
            THEN
        THEN
    ELSE
        0
    THEN
;

: ?ARG DDUP ARG 0= 
    IF 
        2DROP 2DROP
        ARG-EXPECTED-ERROR 0 
    ELSE 
        ( g l g l v )
        DUP >R
        LROT LROT >=<
        IF
            ( OK )
            2DROP R> 1
        ELSE
            ( Not a value )
            R> ARG-RANGE-ERROR
            DROP 0
        THEN
    THEN
;

: ARG4   7 -8 ?ARG IF OR ELSE 0 THEN ;
: ARG16  65535 0 ?ARG 0= IF 0 THEN ;
: ARG8   127 -128 ?ARG IF OR ELSE 0 THEN ;

: OP <BUILDS OP-BITS << , ;
: PLAIN    OP DOES> @ , ;
: I8       OP DOES> @ ARG8 , ;
: R        OP DOES> @ REGA , ;
: R-R      OP DOES> @ REGA REGB , ;
: R-I4     OP DOES> @ REGA ARG4 , ;
: R-I16    OP DOES> @ REGA , ARG16 , ;
: R-R-I4   OP DOES> @ REGA ARG4 , ;

: FLOW <BUILDS 8 << , ;
: FLOW-I16    FLOW DOES> @ , ARG16 , ;
: FLOW-I8     FLOW DOES> @ ARG8 , ;
: FLOW-R-I4   FLOW DOES> @ REGA ARG4 , ;
: FLOW-R-I16  FLOW DOES> @ REGA , ARG16 , ;

: 'CC ' DUP 0= IF ." Condition code expected" 0 ELSE CA>WA EXECUTE THEN ;
: CC 'CC OR CC-APPLY-BIT OR ;
: COND <BUILDS 0x20 OR 8 << , ;

: COND-I16    COND DOES> @ CC , ARG16 , ;
: COND-I8     COND DOES> @ CC ARG8 , ;
: COND-R-I4   COND DOES> @ CC REGA ARG4 , ;
: COND-R-I16  COND DOES> @ CC REGA , ARG16 , ;

1 R-R MOV
2 R-I4 MOVI
3 R-I16 MOVIL
4 I8 MOVAI
5 I8 MOVBI

6 R-R LD
7 R-R LD_B
8 R-R-I4 LDAX
9 R-R-I4 LDBX
10 R-R-I4 LDAX_B
11 R-R-I4 LDBX_B

12 R-I4 STI
13 I8 STAI
14 I8 STBI
15 R-I16 STIL
18 R-R-I4 STI_B
19 I8 STAI_B
20 I8 STBI_B

16 R-R ST
17 R-R ST_B
21 R-I4 STXA
22 R-I4 STXB
23 R-I4 STXA_B
24 R-I4 STXB_B

25 R PUSHD
26 R PUSHR
27 R POPD
28 R POPR

29 R-R ADD
30 R-I4 ADDI
31 R-I8 ADDAI
32 R-I8 ADDBI
33 R-R-I16 ADDIL

34 R-R SUB
35 R-I4 SUBI
36 R-I8 SUBAI
37 R-I8 SUBBI
38 R-R-I16 SUBIL

39 R-R MUL
40 R-R DIV
41 R-R AND
42 R-R OR
43 R-R NOT
44 R-R XOR

45 R-R SL
46 R-R SR
47 R-R RR
48 R-R RRC
49 R-R RL
50 R-R RLC

51 R-R BIT
52 R-R SET
53 R-R CLR

54 R-I4 SLI
55 R-I4 SRI
56 R-I4 RRI
57 R-I4 RRCI
58 R-I4 RLI
59 R-I4 RLCI

60 R-I4 BITI
61 R-I4 SETI
62 R-I4 CLRI

63 R-R CMP
64 R-I4 CMPI
65 I8 CMPAI
66 I8 CMPBI
67 R-I16 CMPIL

68 PLAIN RET
69 I8 SYSCALL
70 PLAIN HALT
71 PLAIN BRK

0x80 FLOW-I16 JP
0x81 FLOW-I8 JR
0x82 FLOW-R-I4 JX
0x83 FLOW-R-I16 JXL

0xc0 FLOW-I16 CALL
0xc1 FLOW-I8 CALLR
0xc2 FLOW-R-I4 CALLX
0xc3 FLOW-R-I16 CALLXL

0x80 COND-I16 JP[
0x81 COND-I8 JR[
0x82 COND-R-I4 JX[
0x83 COND-R-I16 JXL[

0xc0 COND-I16 CALL[
0xc1 COND-I8 CALLR[
0xc2 COND-R-I4 CALLX[
0xc3 COND-R-I16 CALLXL[


( Allot some space to store temporary labels )
8 CONSTANT #LABEL-LEN
8 CONSTANT #MAX-LABELS
0 VARIABLE $LABEL-TABLE
  #LABEL-LEN #MAX-LABELS *  4+ VEXTEND

: LABEL-PTR $LABEL-TABLE 2+ ;

#LABEL-LEN #MAX-LABELS *  $LABEL-TABLE !
$LABEL-TABLE 4+ LABEL-PTR !

: FIRST-LABEL $LABEL-TABLE 4+ ;

: LABEL-SPACE
   $LABEL-TABLE 4+ $LABEL-TABLE @ + ( end of the table )
   LABEL-PTR @ 
   -
;

: APPEND-LABEL 
( Append the token at DP to the symbol table )
    DP@ DP@ @ LABEL-PTR @ COPY
    DP@ @ LABEL-PTR @ 2+ + ALIGN LABEL-PTR ! 
;

( now install a token handler to deal with labels )

: ?LABEL 
( Is the current token a label? I.e. does it end with a colon?)
    DP@ WORD-LEN DP@ 1+ + C@ [CHAR] : =
;

: ?LABEL-EXISTS 
    ( Is the current token already defined? )
    ( -- addr 1 / 0 )
    LABEL-PTR @
    FIRST-LABEL
    BEGIN
        DDUP > IF ( OK to continue )
            DUP DP@ COMPARE 0=
            IF ( found it )
                1 1
            ELSE
                DUP @ 2+ + ALIGN
                0
            THEN
        ELSE
            0 1
        THEN
    UNTIL
    IF
        SWAP DROP 1
    ELSE 
        0
    THEN
;

: ?LABEL-FITS
( Does the label fit in the free space? )
    DP@ @ LABEL-SPACE <=
;

: LABEL-HANDLER 
    ?LABEL
    IF 
        ?LABEL-EXISTS
        IF
            .WORD ."  - label already defined "
        ELSE
            ?LABEL-FITS 
            IF
                APPEND-LABEL
            ELSE
                ." Label space exhausted "
            THEN
        THEN
        1 ( this handler has processed the token )
    ELSE   
        0
    THEN        
;

: .LABELS
( Print out all currently defined labels )
    LABEL-PTR @
    FIRST-LABEL
    BEGIN
        DDUP > IF ( OK to continue )
            DUP .WORD CRET
            DUP @ 2+ + ALIGN
            0
        ELSE
            1
        THEN
    UNTIL
    2DROP
;

