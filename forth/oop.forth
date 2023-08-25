( The big one - Object Oriented Forth )
( No. this doesn't remotely work yet )

class: Widget ( this is a base class )
    { context width height } 

    (): Widget ( here is the constructor )
        to width
        to height
        0 open !
    ;

    (): open #WIN-OPEN #GUI SYSCALL context ! ;

;



class: Window extends Widget
    { title }

    (): Window 
        to title
        Widget
    ;

    (): open ( defines a method open() )

        super open()
        title context #WIN-SET-TITLE #GUI SYSCALL

    ; ( end the definition )
;

: TEST

    20 40 S" test window" Window w

    w open()


;

