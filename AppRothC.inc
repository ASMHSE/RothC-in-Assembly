; ==================================================================================================
; Title:      AppRothC.inc
; Author:     Héctor S. Enrique
; Version:    1.0.0
; Purpose:    Simulation Manager and Model objects for RothC model
;
; Adapted from "RothC model" by Kevin Coleman, February 2024
; Adapted from "Wrapper for RothC model" by Kevin Coleman, February 2024
;
; Link:       https://www.rothamsted.ac.uk/rothamsted-carbon-model-rothc
;
; Notes:      Version C.1.0.0, June 2025
;               - First release.
; ==================================================================================================

RothC_controls struct
    co_nsteps    qword   1   ; number of timesteps
    co_year      qword   0   ; year
    co_timeFact  qword  12   ; monthly 1/12, or daily 1/365    
RothC_controls ends

RothC_stable_inputs struct
    si_clay     real8  0.0     ; clay content of the soil (units: %)
    si_depth    real8  0.0     ; depth of topsoil (units: cm)
    si_IOM      real8  0.0     ; inert organic matter (t C /ha)
RothC_stable_inputs ends

RothC_dynamic_inputs struct
    regpointer textequ <xdi>
    DefineXD year
    DefineXD month
    DefineR8 modernC  ;  %modern 
    DefineR8 Temp     ;  Air temperature (C)
    DefineR8 Rain     ;  Rainfall (mm)
    DefineR8 Evap     ;  open pan evaporation (mm)
    DefineR8 C_inp    ;  Carbon input to the soil each month (units: t C /ha)
    DefineR8 FYM_inp  ;  Farmyard manure input to the soil each month (units: t C /ha)
    DefineXD PC       ;  Plant cover (0 = no cover, 1 = covered by a crop)
    DefineR8 DPM_RPM  ;  Ratio of DPM to RPM for carbon additions to the soil (units: none)
RothC_dynamic_inputs ends
 
Object RothC,, Primer

    VirtualMethod  Calculation, POINTER
    VirtualMethod  Decomp

    VirtualMethod  RMF_Tmp
    VirtualMethod  RMF_Moist
    VirtualMethod  RMF_PC

    regpointer textequ <xsi>
    DefinemR8 clay
    DefinemR8 depth

    DefineVariable timeFact, DWORD, 0 
     
    ; All pools are carbon and not organic matter 

    DefinemR8 DPM        ;  Decomposable Plant Material (units: t C /ha)
    DefinemR8 RPM        ;  Resistant Plant Material    (units: t C /ha)
    DefinemR8 Bio        ;  Microbial Biomass           (units: t C /ha)
    DefinemR8 Hum        ;  Humified Organic Matter     (units: t C /ha)
    DefinemR8 IOM        ;  Inert Organic Matter        (units: t C /ha)
    DefinemR8 SOC        ;  Soil Organic Matter / Total organic Matter (units: t C / ha)

    DefinemR8 DPM_Rage   ;  radiocarbon age of DPM
    DefinemR8 RPM_Rage   ;  radiocarbon age of RPM
    DefinemR8 Bio_Rage   ;  radiocarbon age of Bio
    DefinemR8 Hum_Rage   ;  radiocarbon age of Hum
    DefinemR8 IOM_Rage   ;  radiocarbon age of IOM ??
    DefinemR8 Total_Rage ;  radiocarbon age of SOC (/ TOC)

    DefinemR8 SWC        ;  soil moisture deficit (mm per soil depth)
    DefinemR8 RM_TMP     ;  rate modifying fator for temperature (0.0 - ~5.0)
    DefinemR8 RM_Moist   ;  rate modifying fator for moisture (0.0 - 1.0)
    DefinemR8 RM_PC      ;  rate modifying fator for plant retainment (0.6 or 1.0)
  
    DefinemR8s DPM_Delta, RPM_Delta, Bio_Delta, Hum_Delta, IOM_Delta, Total_Delta

    ; Intermediate variables
  
    DefinemR8 RateM

    DefinemR8s SMDMax, SMDMaxAdj, SMD1bar, SMDBare 
    
    DefinemR8s DF1, tstep, exc 
    
    DefinemR8s PI_C_DPM, PI_C_RPM   
    DefinemR8s FYM_C_DPM, FYM_C_RPM, FYM_C_Hum

    ; C that remains in each pool       
    DefinemR8s DPM1,RPM1,Bio1,Hum1 
           
    ; C amount that will become CO2, Bio and Hum (_d = delta change)
    DefinemR8s DPM_d,RPM_d,Bio_d,Hum_d      
      
    ; _co2: amount of pool that becomes CO2 
    DefinemR8s DPM_co2, RPM_co2, Bio_co2, Hum_co2
      
    ; _bio: amount of pool that becomes bio 
    DefinemR8s DPM_bio, RPM_bio, Bio_bio, Hum_bio
      
    ; _hum amount of pool that becomes hum 
    DefinemR8s DPM_hum, RPM_hum, Bio_hum, Hum_hum 
      
    DefinemR8s DPM_Ract,RPM_Ract,Bio_Ract,Hum_Ract,IOM_Ract,Total_Ract
      
    DefinemR8s PI_DPM_Ract, PI_RPM_Ract
      
    DefinemR8s FYM_DPM_Ract, FYM_RPM_Ract, FYM_Hum_Ract      
      
    DefinemR8s DPM_Ract_new, RPM_Ract_new, Bio_Ract_new, Hum_Ract_new
      
    DefinemR8s DPM_Bio_Ract, RPM_Bio_Ract, Bio_Bio_Ract, Hum_Bio_Ract 
    DefinemR8s DPM_Hum_Ract, RPM_Hum_Ract, Bio_Hum_Ract, Hum_Hum_Ract
 
    ; Constantes  
    DefinemR8 RMFMax, 1.0
    DefinemR8 RMFMin, 0.2

    DefinemR8 DPM_k, 10.0
    DefinemR8 RPM_k, 0.3
    DefinemR8 Bio_k, 0.66
    DefinemR8 Hum_k, 0.02 
    
    DefinemR8 conr, 1.2448764018677178689246266549177e-4 ; ln(2.0) / 5568.0
 
ObjectEnd 

Object AppRothC,,ConsoleApp
    
    RedefineMethod Init
    RedefineMethod Done
    
    VirtualMethod  Initial
    VirtualMethod  LeeLineaCSV, POINTER, DWORD
    VirtualMethod  ReadInputData
    RedefineMethod Run
    VirtualMethod  Equilibrium
    VirtualMethod  RunFromMonth13
    
    Embed   inputs, Collection
    
    DefineVariable file_Input,       $ObjPtr(DiskStream), NULL             
    DefineVariable file_MonthResult, $ObjPtr(DiskStream), NULL             
    DefineVariable file_YearResult,  $ObjPtr(DiskStream), NULL
    
    DefineVariable nsteps,     DWORD, 0
    DefineVariable checksteps, DWORD, 0

    DefineVariable model, $ObjPtr(RothC), NULL                 
ObjectEnd

include RothC.inc

include RunRothC.inc
