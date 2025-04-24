Please refer to the following steps if you want to compile and run the samples on Windows platform :

1. Download the latest samples from the opensource git hub repo
   https://github.com/HCL-TECH-SOFTWARE/domino-c-api-samples
2. Take the latest Domino version CAPI toolkit required to run the CAPI samples
3. Make sure you have Microsoft Visual Studio 2017/2022.
   Note: Before setting up the environment variables, Please run the below command from your command prompt
         For example:
             cd C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build
         And then run below command in your command prompt.
             vcvars64  
4. Set the environment variables for Windows as :
   
   set INCLUDE = <include path>;%INCLUDE%;
   set LIB = <lib path>;%LIB%
   
   "include path" should contain CAPI include files directory and Microsoft C include file directory
   "lib path" should contain CAPI libraries directory and Microsoft C libraries directory
   
   Please note the following points,
   (a.)All the "libs" and "includes" are part of the CAPI toolkit releases.
   (b.)Please try to keep the lib path and include path inside the notes CAPI directory.
   (c.)There is "printLog.h" which is inside "samples" directory and is used for various error handling
       and printing logs inside samples. The path to this file has also to be included as above.
   (d.)There is a common "makeEnvWin.mak" file which is included in all individual sample makefiles.
       This contains all env and options that are common to the all Windows makefiles.
   (e.)In cas of header file conflicts for certain files please refer to "Header File Conflict" section
       in the opensource link https://opensource.hcltechsw.com/domino-c-api-docs/howto/user_guide/Building_Windows_Applications/

5. Go to individual samples and compile the program using nmake /f mswin64.mak. 

6. If the compilation works. Please refer the section "Testing the Installation" in this opensource link
   https://opensource.hcltechsw.com/domino-c-api-docs/howto/user_guide/Building_Windows_Applications/
   to run one sample program and follow same for others.
