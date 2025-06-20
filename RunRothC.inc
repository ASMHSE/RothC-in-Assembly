; ==================================================================================================
; Title:      RunRothC.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Simulation Manager Procedures for RothC model
;
; Adapted from "Wrapper for RothC model" by Kevin Coleman, February 2024
;
; Link:       https://www.rothamsted.ac.uk/rothamsted-carbon-model-rothc
;
; Notes:      Version C.1.0.0, June 2025
;               - First release.
; ==================================================================================================

;  Note:
;  The shell reads in an example data set from RothC_input.dat, if your data 
; is in another format you can change the read statements.
;
;  This model uses the first 12 months of weather (temp, rain, and evap), and land 
; management information (C input, FYM input, and plant cover) to run to equilibrium    
       
    DStr inputfile, "RothC_input.dat"
    ;DStr inputfile, "RothC_input_Morocco.dat"

    DStr monthfile, "month_results.csv"
    DStr yearfile, "year_results.csv"

    DStr title1, "  obs    year   month  modern   Tmp  Rain  Evap   C_inp  FYM  PC  DPM_RPM"
    DStr points, "  ...    ....      ..    ...    ...   ...   ...     ...  ...  ..      ..."

    DStr fmtitle, "Year, Month, DPM_t_C_ha, RPM_t_C_ha, BIO_t_C_ha, HUM_t_C_ha, IOM_t_C_ha, SOC_t_C_ha,     deltaC"    
    
Method AppRothC.Init, uses xbx

    DbgText "AppRothC.Init"
    
    SetObject xbx
    ACall xbx.Init, NULL
    
    OCall [xbx].inputs::Collection.Init, xbx, 1024, 1024, COL_MAX_CAPACITY

    New RothC
    mov  [xbx].model, xax
    mov  xsi, xax
    assume xsi : $ObjPtr(RothC)
    
MethodEnd

Method AppRothC.Done, uses xbx
    SetObject xbx
    Destroy [xbx].model
MethodEnd

Method AppRothC.Initial, uses xbx xsi

    SetObject xbx

    mov   xsi, [xbx].model
    mov   [xsi].timeFact, 12  ; monthly
        
    fSlv8 IOM_Rage = 50000.0
    fSlv8 IOM_Delta = (exp(-50000/8035.0) - 1.0) * 1000.0
      
    fSlv8 DPM = 0.0
    fSlv8 RPM = 0.0
    fSlv8 Bio = 0.0
    fSlv8 Hum = 0.0
    fSlv8 IOM = 0.0
      
    fSlv8 DPM_Rage = 0.0
    fSlv8 RPM_Rage = 0.0
    fSlv8 Bio_Rage = 0.0
    fSlv8 Hum_Rage = 0.0
    ;fSlv8 IOM_Rage = 50000.0  

    ; set initial soil water content (deficit) 
    fSlv8 SWC = 0.0

MethodEnd      

.code 
NextToken proc
    local inicia: DWORD
    
    mov   inicia, 0
    mov   xax, xbx
  ini:  
    mov   cl, [xax]
    .if cl == ' ' || cl == 9
        .if inicia == 0
            inc   xbx
        .else
            mov byte ptr [xax],0
            inc   xax
            jmp   fin      
        .endif
    .elseif cl == 0
        jmp   fin
    .else
        mov   inicia, 1        
    .endif 
    inc   xax
    jmp ini
  fin:    
    ret
NextToken endp

        LoadTokenR8 macro var, whatnext
            invoke NextToken
            mov   pline, xax
            invoke StrToSt0A, xbx
           % fstp  &var
            ifb <whatnext>
                mov   xbx, pline
            endif
        endm 
        LoadTokenXD macro var, whatnext
            invoke NextToken
            mov   pline, xax
            invoke dec2xwordA, xbx
            mov   &var, xax
            ifb <whatnext>
                mov   xbx, pline
            endif
        endm 

Method AppRothC.LeeLineaCSV, uses xbx xsi xdi, pline:POINTER, linea:dword
    local buffer[1024]:CHR
    local check1 : xword
    local fregTLS()
    
    SetObject _ecx

    mov xbx, pline
    .if linea == 4

        _push _ecx
        mov xsi, [_ecx].model

        LoadTokenR8 clay
        LoadTokenR8 depth 
        LoadTokenR8 IOM 
        invoke NextToken
        mov   pline, xax
        invoke dec2dwordA, xbx
        _pop _ecx    
        mov [_ecx].nsteps, eax
    .elseif linea == 6    
        OCall _ecx.PrintLn, addr title1
    .elseif linea > 6
        mov   eax, [_ecx].checksteps
        inc   eax
        mov   [_ecx].checksteps, eax
        mov   check1, xax
        _push _ecx

        MemAlloc sizeof RothC_dynamic_inputs
        mov   xdi, xax

        LoadTokenXD year
        LoadTokenXD month 
        LoadTokenR8 modernC
        LoadTokenR8 Temp
        LoadTokenR8 Rain
        LoadTokenR8 Evap
        LoadTokenR8 C_inp
        LoadTokenR8 FYM_inp
        LoadTokenXD PC
        LoadTokenR8 DPM_RPM, fin
        _peek _ecx   
        mov edx, [_ecx].nsteps
        mov eax, linea
        .if eax < 12 || eax > edx
            ALIGN_1 = 9
            ALIGN_2 = 26
            ALIGN_3 = 38
 
            lea   xbx, buffer  
            formati textequ <"¦UX5   ¦UX5   ¦UX5   ¦F31 ¦F22 ¦F31 ¦F31 ¦F34 ¦F30  ¦UX2    ¦F22">
            variablesi textequ <check1, year, month, modernC, Temp, Rain, Evap,C_inp, FYM_inp, PC,DPM_RPM>
            %WriteFr xbx, formati, variablesi 
            _pop _ecx
            _push _ecx
            OCall _ecx.PrintLn, addr buffer
        .elseif linea == 12    
            OCall _ecx.PrintLn, addr points
        .endif

        _pop _ecx
        OCall [_ecx].inputs::Collection.Insert, xdi 
    .endif
    
MethodEnd

; read in RothC input data file: data will be passed from other programs at some point
Method AppRothC.ReadInputData, uses xsi xbx

    local flen: XWORD
    local pDisk : $ObjPtr(DiskStream)
    local pMemBlock : POINTER
    local fregTLS(,64)
    local BufferLinea[1024]:CHR
    local _time : XWORD
    local linea : DWORD
    
    SetObject xsi
    
    invoke GetTickCount
    mov   _time, xax

    mov   linea, 0  
    New DiskStream
    mov   pDisk, xax
    OCall pDisk::DiskStream.Init, NULL, ADDR inputfile, GENERIC_READ, \
                                        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
    .if xax != INVALID_HANDLE_VALUE
         mov xbx, $OCall(pDisk::DiskStream.GetSize)
        .if eax != -1
            mov   flen, xbx 
            MemAlloc ebx
            .if xax != NULL
                mov   pMemBlock, xax
                OCall pDisk::DiskStream.BinRead, pMemBlock, ebx
                .if xax
                    lea   xbx, BufferLinea 
                    mov   xcx , pMemBlock
                    mov   xdx , 0        ;    para la linea
                    .while flen > 0
                        mov   al, byte ptr [xcx]
                        .if al == 0Dh 
                        .elseif al == 0Ah
                            mov   byte ptr [xbx+xdx], 0
                            _pushad
                            OCall xsi.LeeLineaCSV, xbx, linea
                            _popad
                            mov   xdx, -1
                            inc   linea
                        .else
                            mov byte ptr [xbx+xdx], al     
                        .endif
                        dec   flen
                        inc   xdx
                        inc   xcx
                    .endw
                .endif
                MemFree pMemBlock
            .endif            
        .endif
    .endif
    Destroy pDisk 

    invoke GetTickCount
    mov   xcx, _time
    sub   xax, xcx
    mov   _time, xax
    lea   xbx, BufferLinea  
    WriteF xbx, "\nData reading time was ¦UX miliseconds", _time
    OCall xsi.PrintLn, addr BufferLinea
      
MethodEnd
    
Method AppRothC.Run, uses xsi xbx xdi
    local _time : XWORD
    local buffer[1024] : CHR 
    
    SetObject xbx

    .if [xbx].inputs.dCount > 0
            
        invoke GetTickCount
        mov   _time, xax
    
        New DiskStream                      ;71
        mov   [xbx].file_YearResult, xax
        OCall [xbx].file_YearResult::DiskStream.Init, NULL, ADDR yearfile, GENERIC_WRITE, \
                                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
        
        New DiskStream                      ;91
        mov   [xbx].file_MonthResult, xax
        OCall [xbx].file_MonthResult::DiskStream.Init, NULL, ADDR monthfile, GENERIC_WRITE, \
                                            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    
        OCall xbx.Equilibrium
    
        OCall xbx.RunFromMonth13
    
        Destroy [xbx].file_MonthResult
        Destroy [xbx].file_YearResult
    
        invoke GetTickCount
        mov   xcx, _time
        sub   xax, xcx
        mov   _time, xax
        lea   xsi, buffer  
        WriteF xsi, "\nTime of operation was ¦UX miliseconds", _time
        OCall xbx.PrintLn, addr buffer
    .else
        OCall xbx.PrintLn, $OfsCStr("No data was loaded")
    .endif

MethodEnd

; uses first 12 months of weather and carbon data 
Method AppRothC.Equilibrium, uses xbx xdi xsi
    local TOC0 : REAL8, TOC1 : REAL8, test1 : REAL8
    local buffer[1024] : CHR 
    local k : DWORD, j : DWORD
    local fSlvTLS()
    
    DbgText "AppRothC.Equilibrium"
    
    SetObject xbx

    OCall [xbx].file_YearResult::DiskStream.BinWrite, offset fmtitle, 95
    OCall [xbx].file_YearResult::DiskStream.BinWrite, addr cCRLF, 2

    mov   k, 0
    mov   j, -1

    assume xsi : $ObjPtr(RothC)
    fSlv8 SOC = DPM + RPM + Bio + Hum + IOM

    mov   xsi, [xbx].model         
    fSlv8 test1 = 100.0

    @dothis ; Run to equililibrium: cycles through the first 12 months
        fSlv8 Total_Rage = 0
    
        OCall [xbx].inputs::Collection.ItemAt, k
        mov   xdi, xax
        OCall xsi::RothC.Calculation, xax
        
        mov   xdx, 0
        mov   eax, k
        inc   xax
        mov   ecx, [xsi].timeFact  
        div   xcx
        cmp   xdx, 0
        jne   nozero
        fSlv8 TOC0 = TOC1
        fSlv8 TOC1 = DPM + RPM + Bio + Hum
        fSlv8 test1 = abs(TOC1-TOC0)
      nozero:     

        inc   k
        inc   j 
       
        mov   eax, [xsi].timeFact     ; 12 if monthly
        .if k == eax
            mov    k, 0   
        .endif

    @whilethis test1 fGE 1.0e-6

    fSlv8 Total_Delta = (exp(-Total_Rage/8035.0) - 1.0) * 1000.0

    formati textequ <"¦UX4, ¦UX5, ¦F18, ¦F18, ¦F18, ¦F27, ¦F27, ¦F27, ¦F54 ">          
    variablesi  textequ < year, j, DPM, RPM, Bio, Hum, IOM, SOC, Total_Delta>
    _push xbx  
    lea   xbx, buffer
   % WriteFr xbx, formati, variablesi
    _pop  xbx
    
    invoke StrSizeA, addr buffer
    dec eax
    OCall [xbx].file_YearResult::DiskStream.BinWrite, addr buffer, eax
   
MethodEnd
         
      
; run RothC for months 13 to the end
Method AppRothC.RunFromMonth13, uses xbx xdi xsi
    local buffer[1024] : CHR 
    local i : DWORD, j : DWORD

    SetObject xbx
    mov   xsi, [xbx].model         

    OCall [xbx].file_MonthResult::DiskStream.BinWrite, offset fmtitle, 95

    mov   j, 0
   
    ForLpD i, [xsi].timeFact, [xbx].nsteps, eax   ; 13 if monthly
        OCall [xbx].inputs::Collection.ItemAt, i
        mov   xdi, xax
        OCall xsi::RothC.Calculation, xax
         
        fSlv8 Total_Delta = (exp(-1*Total_Rage/8035.0) - 1.0) * 1000.0
         
        inc   j
     
        OCall [xbx].file_MonthResult::DiskStream.BinWrite, addr cCRLF, 2
        _push xbx  
        lea   xbx, buffer
       % WriteFr xbx, formati, variablesi
        _pop  xbx
        invoke StrSizeA, addr buffer
        dec   eax
        OCall [xbx].file_MonthResult::DiskStream.BinWrite, addr buffer, eax

        mov   eax, [xsi].timeFact     ; 12 if monthly
        .if j == eax
            OCall [xbx].file_YearResult::DiskStream.BinWrite, addr cCRLF, 2
            _push xbx  
            lea   xbx, buffer
           % WriteFr xbx, formati, variablesi
            _pop  xbx
            invoke StrSizeA, addr buffer
            dec   eax
            OCall [xbx].file_YearResult::DiskStream.BinWrite, addr buffer, eax
            mov   j, 0   
        .endif
 
    Next i  
MethodEnd   
      
            