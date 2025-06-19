; ==================================================================================================
; Title:      AppRothC.asm
; Authors:    Héctor S. Enrique
; Version     1.0.0
; Purpose:    ObjAsm RothC Simulator application.
; Notes:      Version 1.0.0, October 2025
;               - First release. 
; ==================================================================================================

NOMINMAX = 1
%include @Environ(OBJASM_PATH)\\Code\\Macros\\Model.inc
SysSetup OOP, CON64, ANSI_STRING;, DEBUG(WND, INFO);, RESGUARD)
;SILENT=TRUE

% include &MacPath&fMath.inc
% include &MacPath&BStrings.inc

% include &IncPath&Windows\ShellApi.inc
% include &IncPath&Windows\shlwapi.inc
% include &IncPath&Windows\WinConTypes.inc

% includelib &LibPath&Windows\shell32.lib
% includelib &LibPath&Windows\shlwapi.lib

% include &MacPath&LDLL.inc
% include &MacPath&LSLL.inc

MakeObjects Primer, Stream, Collection, SortedCollection, StrCollectionA
MakeObjects DiskStream
MakeObjects ConsoleApp

include \masm32\macros\SmplMath\math.inc
fSlvSelectBackEnd FPU

    @reg32_64 ecx, r10
    @reg32_64 edx, r11
    @reg32_64 eax, r12
  
include ModMacs.inc
include WriteFTr.inc

MakeObjects .\AppRothC

.code
  
start proc                                            ;Program entry point
    SysInit
    ifndef SILENT
        DbgClearAll
    endif

    OCall $ObjTmpl(AppRothC)::AppRothC.Init           ;Initialize application
    OCall $ObjTmpl(AppRothC)::AppRothC.Initial        ;Initialize model

    OCall $ObjTmpl(AppRothC)::AppRothC.ReadInputData  ;Execute data load
    OCall $ObjTmpl(AppRothC)::AppRothC.Run            ;Execute application

    OCall $ObjTmpl(AppRothC)::AppRothC.PrintLn, $OfsCStr(10,13,"Press any key to continue...")
    .repeat
        OCall $ObjTmpl(AppRothC)::AppRothC.GetChar
    .until eax != NULL

    OCall $ObjTmpl(AppRothC)::AppRothC.Done           ;Finalize application

    SysDone                                            ;Runtime finalization of the OOP model
    invoke ExitProcess, 0 
start endp

end
