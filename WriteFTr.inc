; ==================================================================================================
; Title:      WriteFTr.inc
; Author:     G. Friedrich, Héctor S. Enrique
; Version:    C.1.0
; Purpose:    ObjAsm support of formated string handling macros.
; Notes:      Version C.1.0, October 2017
;               - First WriteFT.inc release.
; Notes:      Version C.1.1, June 2025
;               - Adaptation to generate fixed with rigth aligned strings.
; ==================================================================================================


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Code:       Wrappers to functions that convert numbers into decimal representations.
; Purpose:    Write formated fixed with rigth aligned strings to memory using current ObjMem functions.

        Num2DecR_Tr macro FUNC_STR_TYPE, FUNC_NAME, NUMBER_TYPE
            
            ifidn <NUMBER_TYPE>,<real8>
                FUNC_NAME&FUNC_STR_TYPE&r proc uses xbx dest: POINTER, prepoint:dword, postpoint:dword, dFlags:dword
            else
                FUNC_NAME&FUNC_STR_TYPE&r proc uses xbx dest: POINTER, number:NUMBER_TYPE, minsize:dword
            endif
                local buffer[32] :CHR
                local buffer2[32]:CHR
                local pointer1   : POINTER
                local len1       : dword, len2 : dword    
                ifidn <NUMBER_TYPE>,<real8>
                    local minsize: dword
                endif            
            
                mov   xbx, dest
                ifidn <NUMBER_TYPE>,<real8>
                    mov   eax, prepoint
                    add   eax, postpoint
                    inc   eax
                    mov   minsize, eax
                    dec   prepoint
                    invoke FUNC_NAME&FUNC_STR_TYPE, addr buffer, prepoint, postpoint,dFlags
                    invoke StrSize&FUNC_STR_TYPE, addr buffer
                else
                    invoke FUNC_NAME&FUNC_STR_TYPE, addr buffer, number
                endif
                mov   len1, eax
                
                mov   eax, minsize
                inc   eax
                .if len1 < eax
                    mov   __ecx, len1
                    mov   len1, eax
                    sub   eax, __ecx
                    mov   len2, eax
                    invoke StrFillChr&FUNC_STR_TYPE, addr buffer2, len2, 32
                    mov   ecx, len2
                    ifidn <FUNC_STR_TYPE>,<W> 
                        shl   xcx, 1
                    endif  
                    lea   xax, buffer2
                    add   xcx, xax
                    mov   byte ptr [xcx], 0
                    invoke StrCopy&FUNC_STR_TYPE, xbx, addr buffer2
                    mov   eax, len2
                    ifidn <FUNC_STR_TYPE>,<W> 
                        shl   xax, 1
                    endif  
                    add   xbx, xax
                .endif
                invoke StrCopy&FUNC_STR_TYPE, xbx, addr buffer
                mov   eax, len1
                ret
               
            FUNC_NAME&FUNC_STR_TYPE&r endp
        endm

    WrapperNum2DecR macro FUNC_NAME, NUMBER_TYPE
        
        Num2DecR_Tr A, FUNC_NAME, NUMBER_TYPE     
        ; Not tested W 
        Num2DecR_Tr W, FUNC_NAME, NUMBER_TYPE     
    endm    

.code
WrapperNum2DecR sdword2dec, dword
WrapperNum2DecR sqword2dec, qword
WrapperNum2DecR sxword2dec, xword
WrapperNum2DecR udword2dec, dword
WrapperNum2DecR uqword2dec, qword

WrapperNum2DecR uxword2dec, xword
WrapperNum2DecR byte2hex, byte
WrapperNum2DecR dword2hex, dword
WrapperNum2DecR word2hex, word
WrapperNum2DecR qword2hex, qword
WrapperNum2DecR xword2hex, xword
WrapperNum2DecR St0ToStr, real8

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:      WriteFr
; Purpose:    Write a formated string to memory. If format includes format specifiers (subsequences
;             beginning with ¦), the additional arguments following format are formatted & inserted
;             in the resulting string replacing their respective specifiers.
;             Escape sequences are supported to overcome MASM limitations. Escape sequences begin
;             with a \ character.
; Arguments:  Arg1: Non-volatile register pointing to the beginning of the output buffer.
;                   It is used internally and its value is restored before return.
;             Arg2: Single or double Quoted format string. Only ANSI characters are allowed.
;                   format specifiers start with the "¦" character, followed by 2 characters
;                   specifying the type (and the decimal places for floating-point numbers) of the
;                   subsequent arguments.
;             Arg3-N: Additional arguments. Must be non-volatile registers, FPU registers or
;                   memory symbols.
; Return:     On return Arg1 points to the ZTC, which helps appending other strings
;             or calculate the number of written bytes.
; Notes:      - Format specifiers:
;                 SB signed BYTE as decimal
;                 SW signed WORD as decimal
;                 SD signed DWORD as decimal
;                 SQ signed QWORD as decimal
;                 SX signed DWORD/QWORD as decimal
;                 UB unsigned BYTE as decimal
;                 UW unsigned WORD as decimal
;                 UD unsigned DWORD as decimal
;                 UQ unsigned QWORD as decimal
;                 UX unsigned DWORD/QWORD as decimal
;                 Fn Floating Point with n decimals, regular notation. n ranges from 0 to F
;                 En Floating Point with n decimals, scientific notation. n ranges from 0 to F
;                 HB BYTE as hexadecimal
;                 HW WORD as hexadecimal
;                 HD DWORD as hexadecimal
;                 HQ QWORD as hexadecimal
;                 HX DWORD/QWORD as hexadecimal
;                 SA ANSI string
;                 SU WIDE string (Unicode)
;                 ST ANSI/WIDE string
;                 GD GUID
;                 WE Windows API Error as description string
;                 CE COM Error as description string
;                 UE UEFI Error as description string
;                 MT Move to character position
;                 AT fill with spaces up to character position
;                 V4 IPv4 address in the format xxx.xxx.xxx.xxx, input must be in NBO
;                 V6 IPv6 address in the format xxxx:xxxx:... , input must be in NBO
;                 CA ANSI Character
;                 CW WIDE Character
;                 CT ANSI/WIDE Character
;                 BI Build in a macro. Use <<...>> to define the included code. Arg1 -> dest. buffer
;                 BN Build in a macro that returns in eax the nr of bytes written, excluding the ZTC
;                 BZ Build in a macro that returns in eax the nr of bytes written, including the ZTC
;
;             - Escape sequences:
;                 n  Carriage Return and Line Feed
;                 r  Carriage Return
;                 l  Line Feed
;                 0  ZTC
;                 t  Horizontal Tab
;                 [  "<" character
;                 ]  ">" character
;                 :  "!" character
;                 ´  "'" character
;                 `  """ character
;                 \  "\" character
;
;             - Format specifiers and escape sequences are case sensitive.
;             - To use V4/V6 add the following lines in the asm file:
;                 % includelib &LibPath&Windows\Ntdll.lib
;                 % include &IncPath&Windows\ip2string.inc
;
; Example:    WriteFr xdi, "The value is: ¦BZ ¦SU", <<<invoke udword2dec, xdi, 15>>>, addr cUnit
;
; WriteFr version
;            Just some crude modificacations to obtain right aligned strings
;
;             - String width is controlled by a number after the specifier, or 2 in REAL numbers.      
;                A value minor that minimum width is ignored, and allow to show minimum width
;  
; Example:    WriteFr xdi, "Memory at ¦HX0h: ¦F34", xsi, REAL4 ptr [xsi]
 
WriteFTr macro DstReg:req, Format:req, Args:vararg
  local pBegin
  
  if TARGET_STR_TYPE eq STR_TYPE_ANSI
    $$CHR textequ <CHRA>
    $$WF_T textequ <A>
  else
    $$CHR textequ <CHRW>
    $$WF_T textequ <W>
  endif

  ;;Scan for ¦AT, ¦MT or ¦BI. They require pBegin
  $$WF_Replace = 0                                      ;;Replace flag
  $$WF_CharCount = 0
  $$WF_ReplType textequ <>
  $$BeginRequired = 0
  forc $$WF_Character, <Format>
    if $$BeginRequired eq 0
      $$WF_CharCount = $$WF_CharCount + 1
      if $$WF_CharCount eq 1                            ;;Ignore first character = "
      elseif $$WF_CharCount ne @SizeStr(Format)         ;;Ignore $$WF_LastChar character = "
        $$WF_LastChar Catstr <$$WF_Character>
        if $$WF_Replace
          if @SizeStr(%$$WF_ReplType) eq 0
            $$WF_ReplType Catstr <$$WF_Character>       ;;Get first char
          else
            $$WF_ReplType Catstr $$WF_ReplType, <$$WF_Character>   ;;Append second char
            ifidn $$WF_ReplType, <AT>
              $$BeginRequired = 1
            elseifidn $$WF_ReplType, <MT>
              $$BeginRequired = 1
            elseifidn $$WF_ReplType, <BI>
              $$BeginRequired = 1
            endif
            $$WF_ReplType textequ <>                    ;;Reset replace type
            $$WF_Replace = 0
          endif
        else
          ifidn <&$$WF_Character>, <¦>
            $$WF_Replace = 1                            ;;Set replace flag
          endif
        endif
      endif
    endif
  endm

  if $$BeginRequired eq 1
    .data?
      pBegin  POINTER   ?

    .code
    mov pBegin, DstReg
  endif

  ;;Start doing the job
  .code
  $$Count = 0
  for Arg, <Args>
    @CatStr(<$$WF_Arg>, %$$Count) textequ <Arg>
    $$Count = $$Count + 1
  endm

  $$WF_ArgCount = 0
  $$WF_Text textequ <>
  $$WF_Replace = 0                                      ;;Replace flag
  $$WF_CharCount = 0
  $$WF_ReplType textequ <>
  $$WF_Escape = 0                                       ;;Escape flag

    forc $$WF_Character, <Format>
        $$WF_CharCount = $$WF_CharCount + 1
        if $$WF_CharCount eq 1                              ;;Ignore first character = "
        elseif $$WF_CharCount ne @SizeStr(Format)           ;;Ignore $$WF_LastChar character = "
            $$WF_LastChar Catstr <$$WF_Character>
            if $$WF_Replace
                if @SizeStr(%$$WF_ReplType) eq 0
                    $$WF_ReplType Catstr <$$WF_Character>         ;;Get first char
                elseif @SizeStr(%$$WF_ReplType) eq 1
                    $$WF_ReplType Catstr $$WF_ReplType, <$$WF_Character>   ;;Append second char
                else
                  $$WF_ReplType Catstr $$WF_ReplType, <$$WF_Character>   ;;Append third char
                  ifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SB>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    movsx eax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<sdword2dec>,%$$WF_T,<r>), DstReg, eax, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SW>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    movsx eax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<sdword2dec>,%$$WF_T,<r>), DstReg, eax, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SD>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<sdword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SQ>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<sqword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SX>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<sxword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UB>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    movzx eax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<udword2dec>,%$$WF_T,<r>), DstReg, eax, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UW>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    movzx eax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<udword2dec>,%$$WF_T,<r>), DstReg, eax, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UD>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<udword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UQ>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<uqword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UX> ;-----------------------------------------------
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<uxword2dec>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <HB>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<byte2hex>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    add DstReg, (BYTE2HEX_MAX_CHARS - 1)*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <HW>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                      invoke @CatStr(word2hex>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    add DstReg, (WORD2HEX_MAX_CHARS - 1)*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <HD>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                     invoke @CatStr(<dword2hex>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    add DstReg, (DWORD2HEX_MAX_CHARS - 1)*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <HQ>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<qword2hex>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    add DstReg, (QWORD2HEX_MAX_CHARS - 1)*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <HX>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                     invoke  @CatStr(<xword2hex>,%$$WF_T,<r>), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>)
                    add DstReg, 2*@WordSize*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 1), <F> ;---------------------------------------------------------- Floating Point
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    fld @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<St0ToStr>,%$$WF_T,<r>), DstReg, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 2, 1), <h>), @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 3, 1), <h>), f_NOR
                    fUnload
                    invoke @CatStr(<StrSize>,%$$WF_T), DstReg
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 1), <E>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    fld @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<St0ToStr>,%$$WF_T), DstReg, 0, @CatStr(<0>, @SubStr(<%$$WF_ReplType>, 2, 1), <h>), f_SCI
                    fUnload
                    invoke @CatStr(<StrSize>,%$$WF_T), DstReg
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SA>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    if TARGET_STR_TYPE eq STR_TYPE_WIDE
                      invoke StrA2StrW, DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      invoke StrSizeW, DstReg
                      lea DstReg, [DstReg + xax - Sizeof(CHRW)]
                      $$WF_ArgCount = $$WF_ArgCount + 1
                    else
                      invoke StrCopyA, DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      lea DstReg, [DstReg + xax - sizeof(CHRA)]
                      $$WF_ArgCount = $$WF_ArgCount + 1
                    endif
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <SU>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    if TARGET_STR_TYPE eq STR_TYPE_WIDE
                      invoke StrCopyW, DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      lea DstReg, [DstReg + xax - sizeof(CHRW)]
                      $$WF_ArgCount = $$WF_ArgCount + 1
                    else
                      invoke StrW2StrA, DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      invoke StrSizeA, DstReg
                      lea DstReg, [DstReg + xax - sizeof(CHRA)]
                      $$WF_ArgCount = $$WF_ArgCount + 1
                    endif
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <ST>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke StrCopy, DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <CA>
                    m2m CHRA ptr [DstReg], @CatStr(<$$WF_Arg>, %$$WF_ArgCount), eax
                    add DstReg, sizeof(CHRA)
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <CW>
                    m2m CHRW ptr [DstReg], @CatStr(<$$WF_Arg>, %$$WF_ArgCount), eax
                    add DstReg, sizeof(CHRW)
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <CT>
                    m2m $$CHR ptr [DstReg], @CatStr(<$$WF_Arg>, %$$WF_ArgCount), eax
                    add DstReg, sizeof($$CHR)
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <GD>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<GUID2Str>,%$$WF_T), DstReg, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    add DstReg, (GUID2STR_MAX_CHARS - 1)*sizeof($$CHR)
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <WE>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    if TARGET_PLATFORM eq PLATFORM_WINDOWS
                      invoke @CatStr(<FormatMessage>,%$$WF_T), FORMAT_MESSAGE_FROM_SYSTEM, 0, @CatStr(<$$WF_Arg>, %$$WF_ArgCount), 0, DstReg, 256, 0
                      .if eax != 0
                        lea DstReg, [DstReg + xax*sizeof($$CHR) - 2*sizeof($$CHR)]
                        $$WF_ArgCount = $$WF_ArgCount + 1
                      .endif
                    else
                      .err <Invalid format specifier>
                    endif
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <CE>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    if TARGET_PLATFORM eq PLATFORM_WINDOWS
                      invoke @CatStr(<ComGetErrStr>,%$$WF_T), @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      .if xax != NULL
                        mov DstReg, $invoke(StrECopyA, DstReg, xax)
                        $$WF_ArgCount = $$WF_ArgCount + 1
                      .endif
                    else
                      .err <Invalid format specifier>
                    endif
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <UE>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    if TARGET_PLATFORM eq PLATFORM_UEFI
                      invoke @CatStr(<UefiGetErrStr>,%$$WF_T), @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      .if xax != NULL
                        mov DstReg, $invoke(StrECopyA, DstReg, xax)
                        $$WF_ArgCount = $$WF_ArgCount + 1
                      .endif
                    else
                      .err <Invalid format specifier>
                    endif
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <V4>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<RtlIpv4AddressToString>,%$$WF_T), @CatStr(<$$WF_Arg>, %$$WF_ArgCount), DstReg
                    mov DstReg, xax
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <V6>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    invoke @CatStr(<RtlIpv6AddressToString>,%$$WF_T), @CatStr(<$$WF_Arg>, %$$WF_ArgCount), DstReg
                    mov DstReg, xax
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <BI>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    invoke @CatStr(<StrEnd>,%$$WF_T), pBegin
                    mov DstReg, xax
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <BN>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    @CatStr(<$$WF_Arg>, %$$WF_ArgCount)                               ;Call the macro
                    add DstReg, xax
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <BZ>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    @CatStr(<$$WF_Arg>, %$$WF_ArgCount)                               ;Call the macro
                    lea DstReg, [DstReg + xax - sizeof($$CHR)]
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <AT>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    mov xax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    mov xcx, DstReg
                    sub xcx, pBegin
                    if TARGET_STR_TYPE eq STR_TYPE_WIDE
                      shr xcx, 1
                    endif
                    sub xax, xcx
                    if TARGET_STR_TYPE eq STR_TYPE_WIDE
                      .while SDWORD ptr eax > 1               ;;Fill with spaces
                        mov DCHRW ptr [DstReg], SPC shl 16 + SPC
                        add DstReg, sizeof(DCHRW)
                        sub eax, 2
                      .endw
                    else
                      .while SDWORD ptr eax > 3               ;;Fill with spaces
                        mov QCHRA ptr [DstReg], "    "
                        add DstReg, sizeof(QCHRA)
                        sub eax, 4
                      .endw
                      .if SDWORD ptr eax > 1
                        mov DCHRA ptr [DstReg], "  "
                        add DstReg, sizeof(DCHRA)
                        sub eax, 2
                        cmp eax, 1
                      .endif
                    endif
                    .if ZERO?
                      mov $$CHR ptr [DstReg], SPC
                      add DstReg, sizeof($$CHR)
                    .endif
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  elseifidn @SubStr(<%$$WF_ReplType>, 1, 2), <MT>
                    $$FillText [DstReg], <%$$WF_Text>
                    $$WF_Text textequ <>
                    if ??StrLen ne 0
                      add DstReg, ??StrLen*sizeof($$CHR)
                    endif
                    mov xax, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                    mov xcx, DstReg
                    sub xcx, pBegin
                    if TARGET_STR_TYPE eq STR_TYPE_WIDE
                      shr xcx, 1
                    endif
                    sub xax, xcx
                    .if SDWORD ptr eax < 0                      ;;If negative, move pointer
                      mov DstReg, pBegin
                      mov xcx, @CatStr(<$$WF_Arg>, %$$WF_ArgCount)
                      if TARGET_STR_TYPE eq STR_TYPE_WIDE
                        shl xcx, 1
                      endif
                      add DstReg, xcx
                      m2z $$CHR ptr [DstReg]                      ;;Set ZTC
                    .else
                      if TARGET_STR_TYPE eq STR_TYPE_WIDE
                        .while SDWORD ptr eax > 1               ;;Fill with spaces
                          mov DCHRW ptr [DstReg], SPC shl 16 + SPC
                          add DstReg, sizeof(DCHRW)
                          sub eax, 2
                        .endw
                      else
                        .while SDWORD ptr eax > 3               ;;Fill with spaces
                          mov QCHRA ptr [DstReg], "    "
                          add DstReg, sizeof(QCHRA)
                          sub eax, 4
                        .endw
                        .if SDWORD ptr eax > 1
                          mov DCHRA ptr [DstReg], "  "
                          add DstReg, sizeof(DCHRA)
                          sub eax, 2
                          cmp eax, 1
                        .endif
                      endif
                      .if ZERO?
                        mov $$CHR ptr [DstReg], SPC
                        add DstReg, sizeof($$CHR)
                      .endif
                    .endif
                    $$WF_ArgCount = $$WF_ArgCount + 1
        
                  else                                          ;;No matching format specifiers found
                    $$WF_Text CatStr $$WF_Text, <¦>, $$WF_ReplType  ;;Emit the format specifiers
        
                  endif
                  $$WF_Replace = 0                              ;;Reset replace flag
                  $$WF_ReplType textequ <>                      ;;Reset replace type
                endif
        
            elseif $$WF_Escape
                $$WF_Escape = 0                                 ;;Reset escape flag
                ifidn <$$WF_Character>, <n>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$DCHR ptr [DstReg], 10 shl (8*sizeof($$CHR)) + 13
                  add DstReg, 2*sizeof($$CHR)
                elseifidn <$$WF_Character>, <r>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], 13
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <l>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], 10
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <0>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], 0
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <t>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], 9
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <[>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], '<'
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <]>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], '>'
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <:>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], '!'
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <´>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], "'"
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <`>
                  $$FillText [DstReg], <%$$WF_Text>
                  $$WF_Text textequ <>
                  if ??StrLen ne 0
                    add DstReg, ??StrLen*sizeof($$CHR)
                  endif
                  mov $$CHR ptr [DstReg], '"'
                  add DstReg, sizeof($$CHR)
                elseifidn <$$WF_Character>, <\>
                  $$WF_Text CatStr $$WF_Text, <\>
                else
                  $$WF_Text CatStr $$WF_Text, <\>;, <&$$WF_Character>
                  ifidn <&$$WF_Character>, <¦>
                    $$WF_Replace = 1                            ;;Set replace flag
                  else
                    $$WF_Text CatStr $$WF_Text, <&$$WF_Character>
                  endif
                endif
            else
                ifidn <&$$WF_Character>, <¦>
                  $$WF_Replace = 1                              ;;Set replace flag
                elseifidn <&$$WF_Character>, <\>
                  $$WF_Escape = 1                               ;;Set escape flag
                else
                  $$WF_Text CatStr $$WF_Text, <$$WF_Character>
                endif
            endif
        endif
    endm

  $$FillString [DstReg], %$$WF_Text                     ;;If there is something in the buffer
  if ??StrLen ne 0
    add DstReg, ??StrLen*sizeof($$CHR)
  endif
endm


if TARGET_STR_TYPE eq STR_TYPE_ANSI
  WriteFr textequ <WriteFAr>
else
  WriteFr textequ <WriteFWr>
endif

WriteFAr macro DstReg:req, Format:req, Args:vararg
  $$TARGET_STR_TYPE = TARGET_STR_TYPE
  TARGET_STR_TYPE = STR_TYPE_ANSI
  $$CHR textequ <CHRA>
  $$DCHR textequ <DCHRA>
  $$FillText textequ <FillTextA>
  $$FillString textequ <FillStringA>
  WriteFTr DstReg, Format, Args
  TARGET_STR_TYPE = $$TARGET_STR_TYPE
endm

WriteFWr macro DstReg:req, Format:req, Args:vararg
  $$TARGET_STR_TYPE = TARGET_STR_TYPE
  TARGET_STR_TYPE = STR_TYPE_WIDE
  $$CHR textequ <CHRW>
  $$DCHR textequ <DCHRW>
  $$FillText textequ <FillTextW>
  $$FillString textequ <FillStringW>
  WriteFTr DstReg, Format, Args
  TARGET_STR_TYPE = $$TARGET_STR_TYPE
endm

