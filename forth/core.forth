( Forth source for core words )

: CA ENTRY WA>CA ;

: AT-FIRST
    CA 2+ HERE  =
;


: VALLOT 
( size-in-words -- start-addr )
    2 *
    %USER-TOP @ SWAP ( size -- current-address size )
    %USER-TOP @ + %USER-TOP  !
;

: ARRAY 
    ( size-in-words -- ) 
    ( -- addr size-in-words )

    <BUILDS DUP VALLOT , , DOES> DUP @ SWAP 2+ @
    
;

: ARRAY! ( value addr len index -- )
    SWAP OVER > 
    IF
        2 * + !
    ELSE
        ." Index out of bounds "
        DROP DROP DROP
    THEN
;


: ARRAY@ ( addr len index -- value )
    SWAP OVER > 
    IF
        2 * + @
    ELSE
        ." Index out of bounds "
        DROP DROP 0
    THEN
;

: .ARRAY 
    DUP 0 
    ?DO 
        OVER OVER I ARRAY@ I . ASPACE EMIT . CRET 
    LOOP 
    DROP DROP 
;

: STACK 1+ ARRAY 0 CA 4 + @ ! ;

: STACK! 1+ ARRAY! ;
: STACK@ 1+ ARRAY@ ;

: PUSH ( val array-addr array-size  -- )
    ( use the given array as a stack, growing upwards)
    ( the first word in the array is the stack pointer )
    OVER @     ( val addr size sp ) ( get the SP )
    OVER 1 - < ( val addr size < ) ( SP below the end of the stack? )
    IF
        ( OK to push the value )
        DROP 
        DUP @ 2 * OVER + 2+ ( val addr size+addr )
        2 PICK SWAP ! ( val addr size+addr val  )
        DUP @ 1+ SWAP ! DROP
    ELSE
        ." Stack overflow"
        DROP DROP DROP
    THEN
;

: POP ( addr size -- val )
    DROP DUP @ IF ( non-zero SP, OK to pop )
    DUP @ 1- OVER !
    DUP @ 2 * + 2+ @
    ELSE
        ." Stack underflow"
    THEN
;

16 STACK __LOCALS__
0 VARIABLE __LOCALDEFS__

: ?} ( addr -- flag) 
    ( Is the string pointed to a closing brace? )
    S" }" COMPARE
;

: LOCALS-JP ENTRY WA>CB 2+ ;

: LOCALS-NVARS ENTRY WA>CB 4 + ;

: LOCALS-FIRST ENTRY WA>CB 6 + ;

: LOCALS-NEXT DUP @ 2+ + ALIGN ;


: DONE ( Leave a DO LOOP )
    R> R> R> SWAP DROP DUP >R >R >R
;

: *$ ( get the offset for the local name ) ( -- offset )
    ASPACE TOKEN ( get the variable name to DP )
    DP@

    LOCALS-FIRST
    0 ( found-flag) 
    
    LOCALS-NVARS
    0 DO
        ( tok-addr var-addr found-flag -- tok-addr var-addr tok-addr var-addr)
        DROP
        OVER OVER COMPARE 0=
        IF ( found it )
            DROP DROP I 1 DONE
        ELSE
            LOCALS-NEXT 0
        THEN
    LOOP

    0= IF ( not found )
     ." Local variable not defined: " TYPE CRET RESTART
    THEN
;

: >$ ['] __LOCALS__ , ['] *# , *$ , ['] STACK! , ; IMMEDIATE

: $> ['] __LOCALS__ , ['] *# , *$ , ['] STACK@ , ; IMMEDIATE



: ALLOT 0 DO 0 , LOOP ;

: ENCLOSE DP @ @ 2+ DP @ + ALIGN DP ! ;

: *{ 
    ( construct a {} block at the start of the word being defined )
    ( *ELSE Relative-jump )
    ( num-vars )
    ( vardef, vardef, ... )
    ['] *ELSE ,
    2 ALLOT ( reserve space for jump and num-vars )
;

: *} DP @ LOCALS-JP !
;

: *INIT-LOCALS
    0 ?DO 0 __LOCALS__ PUSH LOOP
;


: { ( Start local variable definitions )
    AT-FIRST NOT IF ." { Must be the first word " RESTART THEN
    *{ ( compile the starting block )
        BEGIN
            ASPACE TOKEN S" }" DP@ COMPARE 0=
            IF ( got the closing brace )
                1 ( to exit the loop )
            ELSE ( compile the variable name )
                ENCLOSE
                ( update the count of variables )
                LOCALS-NVARS @ 1+ LOCALS-NVARS !
                0 ( don't exit the loop )
            THEN
        UNTIL
    *} ( compile the jump address )
    
    LOCALS-NVARS @ ['] *# , , ( number of locals )
    ['] *INIT-LOCALS ,

; IMMEDIATE

: *FREE-LOCALS
    0 ?DO __LOCALS__ POP DROP LOOP
;

: ;{} ( end a word which has local variables - pop the __LOCALS__ )
    LOCALS-NVARS @ ['] *# , ,
    ['] *FREE-LOCALS ,
    ['] *; , 0 MODE !
; IMMEDIATE

: TEST { a b }
    >$ a >$ b

    $> a $> b + >$ a
    $> a 2 +
;{}



