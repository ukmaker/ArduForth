\ Words to control the STM32

: PORTA 0 ;
: PORTB 1 ;
: PORTC 2 ;
: PORTD 3 ;
: PORTE 4 ;
: PORTF 5 ;
: PORTG 6 ;
: PORTH 7 ;

HEX : PORT-BASE ( port-number -- port-base-address-double ) 0x400 * 0x4002 ;

: PINMODE-INPUT 0 ;
: PINMODE-OUTPUT 1 ;
: PINMODE-ALT 2 ;
: PINMODE-ANALOG 3 ;

: REG-PINMODE 0 ;
: REG-ODR 0x14 ;
: REG-IDR 0x10 ;
: REG-BSRR 0x18 ;

: PORT-ADDR ( reg-number port-number -- Dport-address ) 
    PORT-BASE ROT 0x00 D+ 
;

: PIN-MODEMASK ( pin -- mask-double ) ( calculates the bitmask for the pin's mode bits )
    0x03 0x00 ROT 2 * D<<
;

: PINMODE ( PORT -- Dmode-register-address ) 
    REG-PINMODE SWAP PORT-ADDR  
;

: ODR ( port -- Daddress) REG-ODR SWAP PORT-ADDR ;
: BSRR REG-BSRR SWAP PORT-ADDR ;
: BIT! ( value bit port -- ) ROT IF SWAP 1 0 ROT D<< ROT BSRR H! ;

: PINMODE! ( mode pin-number port-number -- ) ( PINMODE-INPUT 12 PORTA MODE! )
    PINMODE ( get the address of the mode register) ( mode pin-number Dreg-addr )
    DDUP H@ ( get the current value of the register ) ( mode pin-number Dreg-addr Dvalue )
    4 PICK PIN-MODEMASK DINVERT DAND ( mask out the bits we're about to change ) ( mode pin-number Dreg-addr Dmasked-value )
    5 PICK 0x00 6 PICK ( mode pin-number Dreg-addr Dmasked-value Dmode pin-number )
    2 * D<< ( mode bits to the correct position ) ( mode pin-number Dreg-addr Dmasked-value Dmode-shifted )
    DOR DSWAP H!
    DROP DROP
;
