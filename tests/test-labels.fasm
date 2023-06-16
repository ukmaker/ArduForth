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