; ==================================================================================================
; Title:      RothC.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Procedures for RothC model
;
; Adapted from "RothC model" by Kevin Coleman, February 2024
;
; Link:       https://www.rothamsted.ac.uk/rothamsted-carbon-model-rothc
;
; Notes:      Version C.1.0.0, June 2025
;               - First release.
; ==================================================================================================

Method RothC.Calculation, uses xsi xdi, xinput : POINTER
    SetObject xsi  
    
    mov   xdi, xinput
    assume xdi : ptr RothC_dynamic_inputs
     
    ; Calc RMF's i've create a subroutine for each to make it easier to add others or replace existing with new.     
    OCall xsi.RMF_Tmp       ;(TEMP, RM_TMP)
    OCall xsi.RMF_Moist     ;(RAIN, PEVAP, clay, depth, PC, SWC, RM_Moist)
    OCall xsi.RMF_PC        ;(PC,RM_PC)

    ; combine RMF's into one.      
    fSlv8 RateM = RM_TMP * RM_Moist * RM_PC
     
    OCall xsi.Decomp
MethodEnd

;**********************************************************************
;    Calculates the temperature modifying factor   
;**********************************************************************     

Method RothC.RMF_Tmp, uses xsi
    local fSlvTLS()
    
    SetObject xsi
        
    .if fLT(Temp,-5.0)
        fldz
    .else
        fSlv8 = 47.91/(exp(106.06/(Temp+18.27))+1.0)
    .endif
    fstp  RM_TMP

MethodEnd

;**********************************************************************
;       Calculates the moisture modifying factor  
;**********************************************************************     

Method RothC.RMF_Moist, uses xsi
    local fSlvTLS()

    SetObject xsi     

    ; calc soil water functions properties
    fSlv8 SMDMax = -(20 + 1.3*clay - 0.01*(clay*clay))
    fSlv8 SMDMaxAdj = SMDMax * depth / 23.0
    fSlv8 SMD1bar = 0.444 * SMDMaxAdj
    fSlv8 SMDBare = 0.556 * SMDMaxAdj
      
    fSlv8 DF1 = Rain - 0.75 * Evap
      
    .if fEQ(PC,1)
        fSlv8 = max(SMDMaxAdj,min(0.0,SWC+DF1))
    .else
        fSlv8 = max(min(SMDBare,SWC),min(0.0,SWC+DF1))
    .endif
    fstp  SWC  
      
    .if fGT(SWC,SMD1bar)
        fld1
    .else
        fSlv8 = (RMFMin + (RMFMax - RMFMin) \ 
                  * (SMDMaxAdj - SWC) / (SMDMaxAdj - SMD1bar) )   
    .endif
    fstp  RM_Moist
MethodEnd

;**********************************************************************
;      Calculates the plant retainment modifying factor  
;**********************************************************************     
      
Method RothC.RMF_PC, uses xsi
    local fSlvTLS()
     
    SetObject xsi

    .IF fEQ(PC,0)
        fld1
    .ELSE
        fSlv8  = 0.6
    .ENDIF
    fstp  RM_PC
MethodEnd

;**********************************************************************
;      calculates the decomposition and radiocarbon 
;**********************************************************************     

Method RothC.Decomp, uses xsi
    local rmodernC : REAL8
    local x : real8, prop1:real8,prop2:real8,prop3:real8
    local fSlvTLS()
 
    SetObject xsi
   
    fSlv8 tstep = 1.0/[xsi].timeFact    ; monthly 1/12, or daily 1/365  
      
    fSlv8 exc = exp(-conr*tstep) 
      
    fSlv8 IOM_Rage = 50000.0
    
    fSlv8 rmodernC = modernC/100  
    ; C decomposition
    fSlv8 DPM1 = DPM * exp(-RateM*DPM_k*tstep)
    fSlv8 RPM1 = RPM * exp(-RateM*RPM_k*tstep)      
    fSlv8 Bio1 = Bio * exp(-RateM*Bio_k*tstep)      
    fSlv8 Hum1 = Hum * exp(-RateM*Hum_k*tstep) 

      
    fSlv8 DPM_d = DPM - DPM1
    fSlv8 RPM_d = RPM - RPM1      
    fSlv8 Bio_d = Bio - Bio1
    fSlv8 Hum_d = Hum - Hum1 
      
    ; proportion C from each pool into CO2, Bio and Hum      
            fSlv8 x = 1.67*(1.85+1.60*exp(-0.0786*clay))
            
        fSlv8 prop1 = x / (x+1)
        fSlv8 prop2 = 0.46 / (x+1)
        fSlv8 prop3 = 0.54 / (x+1)
    
    fSlv8 DPM_co2 = DPM_d * prop1
    fSlv8 DPM_bio = DPM_d * prop2
    fSlv8 DPM_hum = DPM_d * prop3
      
    fSlv8 RPM_co2 = RPM_d * prop1
    fSlv8 RPM_bio = RPM_d * prop2
    fSlv8 RPM_hum = RPM_d * prop3    
      
    fSlv8 Bio_co2 = Bio_d * prop1
    fSlv8 Bio_bio = Bio_d * prop2
    fSlv8 Bio_hum = Bio_d * prop3
      
    fSlv8 Hum_co2 = Hum_d * prop1
    fSlv8 Hum_bio = Hum_d * prop2
    fSlv8 Hum_hum = Hum_d * prop3  
      
    ; update C pools  
    fSlv8 DPM = DPM1
    fSlv8 RPM = RPM1
    fSlv8 Bio = Bio1 + DPM_bio + RPM_bio + Bio_bio + Hum_bio
    fSlv8 Hum = Hum1 + DPM_hum + RPM_hum + Bio_hum + Hum_hum    
      
    ; split plant C to DPM and RPM 
    fSlv8 PI_C_DPM = DPM_RPM / (DPM_RPM + 1.0) * C_inp
    fSlv8 PI_C_RPM =     1.0 / (DPM_RPM + 1.0) * C_inp

    ; split FYM C to DPM, RPM and Hum 
    fSlv8 FYM_C_DPM = 0.49*FYM_inp
    fSlv8 FYM_C_RPM = 0.49*FYM_inp      
    fSlv8 FYM_C_Hum = 0.02*FYM_inp   
      
    ; add Plant C and FYM_C to DPM, RPM and Hum   
    fSlv8 DPM = DPM + PI_C_DPM + FYM_C_DPM
    fSlv8 RPM = RPM + PI_C_RPM + FYM_C_RPM  
    fSlv8 Hum = Hum + FYM_C_Hum
      
    ; calc new ract of each pool
    fSlv8 DPM_Ract = DPM1 *exp(-conr*DPM_Rage)
    fSlv8 RPM_Ract = RPM1 *exp(-conr*RPM_Rage) 
      
    fSlv8 Bio_Ract = Bio1 * exp(-conr*Bio_Rage)
    fSlv8 DPM_Bio_Ract = DPM_bio * exp(-conr*DPM_Rage)
    fSlv8 RPM_Bio_Ract = RPM_bio * exp(-conr*RPM_Rage)
    fSlv8 Bio_Bio_Ract = Bio_bio * exp(-conr*Bio_Rage)
    fSlv8 Hum_Bio_Ract = Hum_bio * exp(-conr*Hum_Rage)
      
    fSlv8 Hum_Ract = Hum1 * exp(-conr*Hum_Rage)   
    fSlv8 DPM_Hum_Ract = DPM_hum * exp(-conr*DPM_Rage)
    fSlv8 RPM_Hum_Ract = RPM_hum * exp(-conr*RPM_Rage)
    fSlv8 Bio_Hum_Ract = Bio_hum * exp(-conr*Bio_Rage)
    fSlv8 Hum_Hum_Ract = Hum_hum * exp(-conr*Hum_Rage)
      
    fSlv8 IOM_Ract = IOM *exp(-conr*IOM_Rage) 
      
    ; assign new C from plant and FYM the correct age
       
    fSlv8 PI_DPM_Ract = rmodernC * PI_C_DPM
    fSlv8 PI_RPM_Ract = rmodernC * PI_C_RPM
      
    fSlv8 FYM_DPM_Ract = rmodernC * FYM_C_DPM
    fSlv8 FYM_RPM_Ract = rmodernC * FYM_C_RPM
    fSlv8 FYM_Hum_Ract = rmodernC * FYM_C_Hum          
      
    ; update ract for each pool
            
    fSlv8 DPM_Ract_new = FYM_DPM_Ract + PI_DPM_Ract + DPM_Ract*exc
    fSlv8 RPM_Ract_new = FYM_RPM_Ract + PI_RPM_Ract + RPM_Ract*exc    
      
    fSlv8 Bio_Ract_new = (Bio_Ract + DPM_Bio_Ract + RPM_Bio_Ract \ 
                    + Bio_Bio_Ract + Hum_Bio_Ract )*exc
      
    fSlv8 Hum_Ract_new = FYM_Hum_Ract + (Hum_Ract + DPM_Hum_Ract \
                    + RPM_Hum_Ract + Bio_Hum_Ract + Hum_Hum_Ract )*exc  
      
    fSlv8 SOC = DPM + RPM + Bio + Hum + IOM      
      
    fSlv8 Total_Ract = DPM_Ract_new + RPM_Ract_new \
                + Bio_Ract_new + Hum_Ract_new + IOM_Ract

    ; calculate rage of each pool.      
    .if fLE(DPM,0)
        fldz
    .else
        fSlv8 = ln(DPM/DPM_Ract_new) / conr
    .endif
    fstp DPM_Rage   
      
    .if fLE(RPM,0)
        fldz
    .else
        fSlv8 = ln(RPM/RPM_Ract_new) / conr 
    .endif
    fstp RPM_Rage  
      
    .if fLE(Bio,0)
        fldz
    .else
        fSlv8 = ln(Bio/Bio_Ract_new) / conr
    .endif
    fstp Bio_Rage
      
    .if fLE(Hum,0)
        fldz
    .else
        fSlv8 = ln(Hum/Hum_Ract_new) / conr
    .endif
    fstp Hum_Rage
        
    .if fLE(SOC,0)
        fldz
    .else
        fSlv8 = ln(SOC/Total_Ract) / conr    
    .endif
    fstp Total_Rage  
      
MethodEnd
