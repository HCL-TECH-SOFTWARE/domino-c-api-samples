Please refer to the following steps if you want to compile and run the samples on Linux platform :

1. Download the latest samples from the opensource git hub repo
   https://github.com/HCL-TECH-SOFTWARE/domino-c-api-samples
2. Take the latest Domino version CAPI toolkit required to run the CAPI samples
3. Set the environment variables for Linux as :
   
   export NOTES_CAPI=<location of your notes CAPI samples directory>
   export LOTUS=</opt/hcl/domino>
   export Notes_ExecDirectory=<location of domino executable program directory>
   export LD_LIBRARY_PATH=<location of the platorm specific libraries directory>
   export CPLUS_INCLUDE_PATH=<location of the include header files directory>
   
   e.g.
   
   export NOTES_CAPI=/opt/hcl/domino/notes-capi-14.5
   export LOTUS=/opt/hcl/domino
   export Notes_ExecDirectory=/opt/hcl/domino/notes/latest/linux
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/hcl/domino/notes/latest/linux:/opt/hcl/domino/notes-capi-14.5/lib/linux64
   export CPLUS_INCLUDE_PATH=/opt/hcl/domino/notes-capi-14.5/include:/opt/hcl/domino/notes-capi-14.5
   
   Please note following points,
   (a.)All the "libs" and "includes" are part of the CAPI toolkit releases.
   (b.)Please try to keep the lib path and include path inside the notes CAPI directory.
   (c.)There is "printLog.h" which is inside "samples" directory and is used for various error handling
       and printing logs inside samples. The path to this file has also to be included as above.
   (d.)There is a common "makeEnvLinux.mak" file which is included in all individual samples makefile.
       This contains all env and options that are common to the all linux makefiles

4. Go to individual samples and compile the program using make -f linux64.mak. 

5. If the compilation works. Please refer the section "Running HCL C API programs" in this opensource link
   https://opensource.hcltechsw.com/domino-c-api-docs/howto/user_guide/Building_UNIX_Applications/
   to run the programs.In case compilation fails, recheck if all the environment variables are properly set
   and all the initial setup are in place as above.
