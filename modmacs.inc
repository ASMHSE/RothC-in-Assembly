    DefineDD macro nombre , valor:=<0>
        @CatStr(  DD35_ , &nombre , <  dword >) &valor
       % &nombre textequ <[regpointer].DD35_&nombre>
    endm
    DefineXD macro nombre , valor:=<0>
        @CatStr( XD_ , &nombre, <  xword > ) &valor
       % &nombre textequ <[regpointer].XD_&nombre>
    endm
    
    DefineR8 macro nombre , valor:=<0.0>
        @CatStr(R8_ , &nombre, < REAL8 > ) &valor
       % &nombre textequ <[regpointer].R8_&nombre>
    endm
    DefinemDD macro nombre , valor:=<0>
        DefineVariable    @CatStr(  < >,DD35_ , &nombre ) , dword , &valor
       % &nombre textequ <[regpointer].DD35_&nombre>
    endm
    DefinemXD macro nombre , valor:=<0>
        DefineVariable    @CatStr(  < >,XD_ , &nombre ) , xword , &valor
       % &nombre textequ <[regpointer].XD_&nombre>
    endm
    
    DefinemR8 macro nombre , valor:=<0.0>
        DefineVariable    @CatStr(  < >,R8_ , &nombre ) , REAL8 , &valor
       % &nombre textequ <[regpointer].R8_&nombre>
    endm
    DefinemR8s macro args:vararg
        for arg, <args>
             DefinemR8 <arg>
        endm 
    endm

