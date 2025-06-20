
# The Rothamsted Carbon Model (RothC) in Assembly

## Model

Roth C models the turnover of organic carbon in non-waterlogged top-soil.  It accounts for the effects of soil texture, temperature, moisture content and plant cover on the turnover process. It uses a monthly time step to calculate total organic carbon (t ha<sup>-1</sup>), microbial biomass carbon (t ha<sup>-1</sup>) and Δ<sup>14</sup>C (from which the equivalent radiocarbon age of the soil can be calculated). 

The model consist of 2 constants to define soil, and 5 variable states that represent forms of Carbon in soil (and 1 more for summatory or SOC). Model dinamycs is controlled by 8 input variables, and contain several transient variables and constants for processes representation.

Simulations are Timestep, wich is equivalent to Euler's numerical integration with n=1, and steps can be months or days.

Fortran or Python source code, alongside more information about RothC model, can be found at [ROTHAMSTED CARBON MODEL (ROTHC)](https://www.rothamsted.ac.uk/rothamsted-carbon-model-rothc) page. 
  
## Simulator

[![](https://img.shields.io/badge/Assembler-UASM%20v2.52-green.svg?style=flat-square&logo=visual-studio-code&logoColor=white&colorB=1CC887)](http://www.terraspace.co.uk/uasm.html) 

Framework : ObjAsm C.2  - masters : [Main Developer](https://github.com/ObjAsm/ObjAsm-C.2) , [Fork](https://github.com/ASMHSE/ObjAsm-C.2/tree/master)

Math syntax macros: [SmplMath](https://github.com/ASMHSE/SmplMath)

Current building are for Windows platform, 32 and 64 bits, and ANSI inputs and outputs.

### AppRothC

Programming is Object Oriented, and consist of RothC model object, and a ConsoleApp descendent object who run the simulations. 

This application is intended as an example of how to: 
1.	read in the input data
2.	run the model
3.	created monthly and yearly outputs
Details of the inputs required, pools modelled, and units are in the code.

This is Neutral Bitness Code (BNC) and assembled binary bitness can be changed by selecting SysSetup (AppRothC.asm line 12) CON64 or CON32 

### Inputs  

The file RothC_input.dat contains input variables example for the model. Current format contains information lines and space separated values. 

### Outputs

This example create files year_results.csv and month_results.csv with comma separated values. They contains the yearly or monthly values of the SOC (both the pools and Total) and the delta 14-carbon.
 
## License

RothC code is under Apache 2 License, and OOP code is under ObjAsm-C.2 EULA

