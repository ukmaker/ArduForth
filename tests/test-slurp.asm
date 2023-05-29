 ; Test file
; Length of this file is 880


#HNAME: 0xFA90 ; A constant definition
#DNAME: 3900 ; A constant definition
#BNAME: 0b10010110 ; A constant definition

.ORG 1000 ; Start of variables
%VNAME: 0x30 ; A variable definition. Reserves space and sets initial value
$SNAME: "This is a string" ; Stored as a Forth-string <len><str>

.ORG 0 ; Start of code
START:  LD A,B ; Register to register move
        NOP ; Do nothing
LABEL1: JR[NZ] 3 ; Jump with literal
LABEL2: JR #HNAME ; Jump with literal value defined as #CONST:
LABEL3: JR[NC] %VAR ; Jump to address of VAR. Emits a warning
LABEL4: JR[P] LABEL ; Relative jump to the label. Error if the destination is too far

LDI[Z] A,#BNAME ; Load B with the value 0b10010010 if the Z flag is set
LDI B,%VAR   ; Load A with the address of the variable VAR
ST B,A     ; Store the contents of A in the address pointed to by BNAME

