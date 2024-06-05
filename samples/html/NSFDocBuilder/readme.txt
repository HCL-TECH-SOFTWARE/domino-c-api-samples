Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
ConvertHtmlToNSF - This sample program shows how to convert HTML to a database.
                   1.Takes the source folder path and copies the entire.html file into the current working directory. (ex: sample folder).
                   2.Takes the current working directory path (sample folder) and iterates all.html files, finding image tags in each file. 
                     Then convert the image to a base64 image and replace it in each file, and then store those image files in one temp1 folder.
                     #temp folder                     
                          ex: #define MODIFIED_IMAGE_FOLDER_PATH  "C:\\OutPutHtmlFile\\"
                   3.Now we are copying the files from the temp1 folder to the current working directory (files are now overridden in the working directory).
                   4.Takes the current working directory, iterates all files, and then imports them into the database using the dynamic library (nihtnl.dll).
                   5.Now take the current working directory, iterate all files, find the href (hyperlink tag) in each file, and replace it with notesURL.
                     and place those files in another temp2 folder.
                     #temp folder                    
                          ex: #define TARGET_FOLDER_PATH  "C:\\HyperLinks\\"
                   6.Now we are copying the files from the temp2 folder to the current working directory (files are now overridden in the working directory).
                   7.Now iterate all files in the current working directory, read the content of each file, and open each document in the current database. 
                     and then delete the "Body" field in each document and recreate the "Body" field in each document. 


ENVIRONMENTS
------------
Windows (64-bit)

FILES
-----
readme.txt  -  This file specifies what is needed to use this example.
ConvertHtmlToNSF.c  -  Main program.
CDRecord.c    -  Contains the functions that create the various CD records.
CDRecord.h    -  Function prototypes and symbolic constants
mswin64.mak     - Make file for Windows 64-bit.


RUNNING ConvertHtmlToNSF 
-------------------------
Copy the sample database "TestFile.nsf", to the Domino data
directory on your system. 

- Under Windows:
  type the following at the system command prompt:

    ConvertHtmlToNSF <database name> <Dll name> <source path> <view name>
    where:
      <database> is the filename of the Domino database to be modified.
      <Dll name> is the dynamic library name along with fill path.
      <source path> is the html files path.
      <view name> is the view name of the database.
EXAMPLE
-------

 ConvertHtmlToNSF TestFile.nsf "C:\\Program Files (x86)\\HCL\\Notes\\nihtml.dll" "C:\\TestHtmlFiles\\" "Sample View"

 NOTE: Before running this sample you need to create two temp folders and then modify below two MACRO'S in 
       sample code and then run.
       #define TARGET_FOLDER_PATH  "C:\\HyperLinks\\"
       #define MODIFIED_IMAGE_FOLDER_PATH  "C:\\OutPutHtmlFile\\"


CHECKING THE RESULTS
--------------------
 Check database once completed program.


