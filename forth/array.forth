
: ARRAY 
    ( size-in-words -- ) 
    ( -- addr )
    <BUILDS $USER-TOP @ OVER VALLOT , , DOES>
;

: ARRAY-SIZE 2+ @ ;

: ARRAY-BOUNDS? 
    ( addr index -- addr index flag )
    ( is the index within the bounds of the array ? )
    DDUP SWAP ARRAY-SIZE <
    DUP NOT IF ." Index out of bounds " THEN
;

: ARRAY! 
    ( value array-addr index -- )
    ARRAY-BOUNDS?
    IF
        2* SWAP @ + !
    ELSE
        2DROP DROP
    THEN
;


: ARRAY@ 
    ( addr index -- value )
    ARRAY-BOUNDS?
    IF
        2* SWAP @ + @
    ELSE
        2DROP 0
    THEN
;

: .ARRAY 
    DUP ARRAY-SIZE  0
    ?DO 
        DUP I ARRAY@ I . &sp; EMIT . CRET 
    LOOP 
    DROP 
;
