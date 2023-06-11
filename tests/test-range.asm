 ; Test that the assembler correctly detects out of range values
#VARSTART: 1000

.ORG #VARSTART
%BIGONE: 2

.ORG 0
MOVI A,%BIGONE
MOVAI 2000 
