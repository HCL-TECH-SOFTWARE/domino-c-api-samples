/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *

    PROGRAM:    ConvertHtmlToNSF

    FILE:       ConvertHtmlToNSF.c

    PURPOSE:    Shows how to convert .html to .NSF conversion.

    SYNTAX:     ConvertHtmlToNSF <database name> <DLL name> <source path> <view name>

**************************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

/* OS includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(UNIX)
#include <sys/io.h>
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>

/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <ods.h>
#include <names.h>
#include <editods.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <ixport.h>
#include <ixedit.h>
#include <osmisc.h>
#include <dirent.h>

#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <osmisc.h>
#include <oserr.h>
#include <odserr.h>
#include <names.h>
#include <editods.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <ixport.h>
#include <ixedit.h>
#include <ods.h>
#include <printLog.h>
#include <dirent.h>
#include <miscerr.h>
#include <printLog.h>
#include <nsfnote.h>

/* local includes  */

#include "CDRecord.h"

#if !defined(ND64) 
#define DHANDLE HANDLE 
#endif

/* Function specifications  */

STATUS LNPUBLIC ImportCD(char* szModulePath,
    char* szFileName,
    char* szTempName,
    char* szDLL);

STATUS LNPUBLIC AppendImportItem(NOTEHANDLE hNote,
    char* pszImportFile,
    char* pszItemName);

STATUS LNPUBLIC removeFirstLine(char* cdFileName);

/* function specification for Domino and Notes DLLs    */

STATUS(LNCALLBACKPTR ProcAddress)(VOID* IXContext, WORD Flags,
    HMODULE hModule, char* AltLibraryName, char* PathName);

static const char base64encdec[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


#define ITEM_NAME_Body    "Body"
#define TARGET_FOLDER_PATH  "C:\\HyperLinks\\"
#define MODIFIED_IMAGE_FOLDER_PATH  "C:\\OutPutHtmlFile\\"
#define LINEOFTEXT   256


//struct key_value* kv;
// this structure we used for storing (key = htmlfile name, value = notesURL)

struct key_value
{
    char* key;
    char value[1000];
};


void ModifyBodyFieldInEachDocument(DBHANDLE hDB1, NOTEID viewNoteID1,char * filePath1) {

    STATUS error = NOERROR;
    DBHANDLE hDB;
    HCOLLECTION hCollection;
    NOTEID viewNoteID;
    BLOCKID hBlock;
    BLOCKID bhItem;
    BLOCKID bhItemCopy;
    int nFieldLen;
    NOTEID* id_list;
    DWORD  note_count = 0;
    char* htmlFileName;

    /* Open the collection of notes in the view at the current time.
       Return a handle to the collection to the variable hCollection. */
    if (error = NIFOpenCollection(hDB1, hDB1, viewNoteID1,
        0,
        NULLHANDLE,
        &hCollection,
        NULL, NULL, NULL, NULL))
    {
        goto NIFErr;
    }

    {
        COLLECTIONPOSITION indexPos;
        DWORD entriesReturned;
        DHANDLE hBuffer;
        WORD   signalFlag;             /* signal and share warning flags */

        indexPos.Level = 0;
        indexPos.Tumbler[0] = 0;
        //indexPos.Tumbler[0] = 1;

        do
        {
            if (error = NIFReadEntries(hCollection,
                &indexPos,
                NAVIGATE_NEXT, 1L,
                NAVIGATE_NEXT, MAXDWORD,
                READ_MASK_NOTEID,
                &hBuffer, NULL,
                NULL, &entriesReturned, &signalFlag))
            {
                goto ColErr;
            }

            /********************************************************************/

            if (hBuffer != NULLHANDLE)
            {
                char string[512];
                const char szItemname[LINEOFTEXT] = "Body";
                const char szItem[LINEOFTEXT] = "Name";
                char szActValue[LINEOFTEXT] = "";
                //WORD wItemtype = TYPE_TEXT;
                WORD wItemtype = TYPE_HTML;
                FILE* fp;
                struct dirent* de;  // Pointer for directory entry 
                printf("PRINT PATH :%s", filePath1);
                DIR* dr = opendir(filePath1);

                if (dr == NULL)  // opendir returns NULL if couldn't open directory 
                {
                    printf("Could not open current directory");
                    return 0;
                }
                int count = 0;
                int i = 0;
                NOTEHANDLE hNoteFirstDoc, hNoteSecDoc;
                NOTEID* noteIDCpy;
                NOTEID* entry = OSLock(NOTEID, hBuffer);
                while ((de = readdir(dr)) != NULL) {
                    
                    htmlFileName = malloc(sizeof(de->d_name));
                    memset(htmlFileName, '\0', sizeof(de->d_name));
                    strncpy(htmlFileName, de->d_name, sizeof(de->d_name));

                    if (strstr(htmlFileName,".html")) {
       
                        printf("\n***********************************::Count::%d\n", ++count);
                        //NOTEID* entry = OSLock(NOTEID, hBuffer);
                        printf("\nentriesReturned :%d\n", entriesReturned);
                        /* process each individual noteID       */

                        /* skip this noteID if it is for a category entry  */
                        if (*entry & NOTEID_CATEGORY)
                        {
                            entry++;
                        }

                        if (!(NOTEID_CATEGORY & entry[i]))
                            PRINTLOG("Note ID number %lu is: %lX\n", ++note_count, entry[i]);

                        if (error = NSFNoteOpen(hDB1, entry[i], 0, &hNoteFirstDoc))
                        {
                            OSLoadString(NULLHANDLE, ERR(error), string, sizeof(string) - 1);

                            printf("Error '%s' reading docment %#lX -- %s\n",
                                string, entry[i], " skipping it");

                            /* Since the error has been reported, we will
                               reset the error status and continue */
                            error = NOERROR;
                            continue;
                        }

                        noteIDCpy = entry[i]; /* Preserve it for later use */

                        /* get item Name info */
                        if (error = NSFItemInfo(hNoteFirstDoc,
                            szItem,
                            strlen(szItem),
                            &bhItem,
                            NULL,
                            NULL,
                            NULL))
                        {
                            goto DocErr;
                        }

                        nFieldLen = NSFItemGetText(hNoteFirstDoc, szItem, szActValue, LINEOFTEXT);

                        printf("\n DocumentName: %s : %s\n", szActValue, htmlFileName);

                        /* get item info */
                        if (error = NSFItemInfo(hNoteFirstDoc,
                            szItemname,
                            strlen(szItemname),
                            &bhItem,
                            NULL,
                            NULL,
                            NULL))
                        {
                            goto DocErr;
                        }

                        nFieldLen = NSFItemGetText(hNoteFirstDoc, szItemname, szActValue, LINEOFTEXT);
                        bhItemCopy = bhItem;

                        /* This API is used to delete the item value(Body) */
                        if (error = NSFItemDelete(hNoteFirstDoc,
                            ITEM_NAME_Body,
                            (WORD)strlen(ITEM_NAME_Body)))
                        {
                            PRINTLOG("Error: unable to delete item '%s' from note.\n", ITEM_NAME_Body);
                            NSFNoteClose(hNoteFirstDoc);
                            goto DocErr;
                        }

                        if (error = AppendImportItem(hNoteFirstDoc, htmlFileName, "Body"))
                        {
                            PRINTLOG("\nError while adding rich text.  Terminating...\n");
                            NSFNoteClose(hNoteFirstDoc);    
                            NSFDbClose(hDB1);    
                            PRINTERROR(error, "AppendImportItem");
                            goto DocErr;
                        } 

                        /* Update the Notes handler to get the changes reflected in nsf */
                        if (error = NSFNoteUpdate(hNoteFirstDoc, UPDATE_FORCE))
                        {
                            goto DocErr;
                        }
                        i++;
                    }
                    free(htmlFileName);
                }

            DocErr:
                /* close each note   */
                NSFNoteClose(hNoteFirstDoc);
                if (error)
                {
                    OSLoadString(NULLHANDLE, ERR(error), string,
                        sizeof(string) - 1);
                    printf("Error '%s' writing document %#lX -- %s\n",
                        string, entry[i], "skipping it");
                    /* Since the error has been reported, we will
                       reset the error status and continue */
                    error = NOERROR;
                    continue;
                }

                /* finished with noteIDs, unlock memory and free it   */
                OSUnlockObject(hBuffer);
                OSMemFree(hBuffer);
            }   /* if (hBuffer != NULLHANDLE) */

        } while (signalFlag & SIGNAL_MORE_TO_DO);

        NIFCloseCollection(hCollection);

        goto ColSucc;
    }

ColErr:
    NIFCloseCollection(hCollection);

ColSucc:
NIFErr:
    /********************************************************************/
    /*
    Close the Domino database.
    */
    /********************************************************************/

DBErr:
    if (error)
    {
        PRINTERROR(error, "NIFReadEntries");
    }

    return(error);
}


void concatenate_string(char* s, char* szTemp)
{
    int i;

    int j = strlen(s);

    for (i = 0; szTemp[i] != '\0'; i++) {
        s[i + j] = szTemp[i];
    }

    s[i + j] = '\0';

    return;
}


char* replaceWord(const char* s, const char* oldW, const char* newW)
{
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;

            // Jumping to index after the old word. 
            i += oldWlen - 1;
        }
    }

    // Making new string of enough length 
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s) {
        // compare the substring with the result 
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}


char* ReplaceHyperLinksToNotesURL(char* filePath, int fileCount, struct key_value* filename_notesURL) {

    DIR* dir;
    struct dirent* entry;
    char* fileName;
    dir = opendir(filePath);
    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }

    // Read files from the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories
        if (entry->d_type == DT_DIR) continue;
        char* fileName = malloc(strlen(filePath) + strlen(entry->d_name) + 1);
        strcpy(fileName, entry->d_name);
        if (strstr(fileName, ".html")) {

            char* szRead;
            char* szOutPutValue;
            char urlname[MAXPATH];
            char* fullPathFile[MAXPATH];
            char tempPath[MAXPATH];
            strcpy(fullPathFile, filePath);
            strcat(fullPathFile, "\\");
            strcat(fullPathFile, fileName);  //prepared input  file full path
            strcpy(tempPath, TARGET_FOLDER_PATH);
            strcat(tempPath, fileName);  //prepared target file full path

            //printf("\n*file name*:%s\n", fullPathFile);
            FILE* fp;
            FILE* fp1;
            fp = fopen(fullPathFile, "r+");
            fp1 = fopen(tempPath, "w+");

            if (fp == NULL) {
                printf("Can't open file.");
                exit(0);
            }

            fseek(fp, 0L, SEEK_END);
            long int fileSize = ftell(fp);
            szRead = (char*)malloc((size_t)fileSize + 1);
            fseek(fp, 0L, SEEK_SET);

            if (fp != NULL) {

                // comparing word with file
                rewind(fp);
                while (!feof(fp)) {
                    fscanf(fp, "%s", szRead);
                    if (strstr(szRead, "href=")) {
                         char tempValue[1000];
                         strcpy(tempValue, szRead);
                         char delimiter1[] = "=";
                         char* token1 = strtok(tempValue, delimiter1);
                         while (token1 != NULL) {
                             strcpy(urlname, token1);
                             token1 = strtok(NULL, delimiter1);
                         }
                         if (strstr(urlname, ".html")) {
                             char notesURL[MAXPATH];
                             if ((strstr(urlname, "<"))) {
                                 char delimiter2[] = "<";
                                 char* token2 = strtok(urlname, delimiter2);
                                 strcpy(urlname, token2);
                             }
                             if ((strstr(urlname, ">"))) {
                                 char delimiter3[] = ">";
                                 char* token3 = strtok(urlname, delimiter3);
                                 strcpy(urlname, token3);
                                 
                             }
                             char temp[1000] = { 0 };
                             int j = 0;
                             for (int i = 1; i < strlen(urlname) - 1; i++) {
                                temp[j++] = urlname[i];
                             }
                             //printf("%s\n", temp);
                             for (int i = 0; i < fileCount; i++) {
                                 char tempFileName[MAXPATH];
                                 char tempNotesURL[MAXPATH +1];
                                 strcpy(tempFileName, filename_notesURL[i].key);
                                 if (strcmp(temp, tempFileName) == 0) {
                                      char* result = NULL;
                                      strcpy(tempNotesURL, filename_notesURL[i].value);
                                      result = replaceWord(szRead, temp, tempNotesURL);
                                      printf("New String: %s\n", result);
                                      strcpy(szRead, result);
                                      free(result);

                                 }

                             }
                            
                         }
                    }
                    fprintf(fp1, "%s ", szRead);
                }
                fclose(fp1);
                fclose(fp);
                free(szRead);
            }
            strcpy(tempPath, " ");
        }
        else {
            printf(" ");
        }
        
    }
    return 1;
}


void ReplaceImageToBase64image(char* szFullPathFileName, char* szBase64, char* szimageName,int szImageTagCount, char* szFileName) {
    FILE* ifp, * ofp;
    char word[MAXPATH], ch, replace[MAXPATH];
    char* szRead;
    char* szReadExtFile;
    int word_len, i, p = 0;
    //char szOutFolderPath[MAXPATH] = "C:\\OutPutHtmlFile\\"; //local variable 
    char szOutFolderPath[MAXPATH];
    strcpy(szOutFolderPath, MODIFIED_IMAGE_FOLDER_PATH);
    strcat(szOutFolderPath, szFileName);

    char lowerCaseTitle[MAXPATH]; //local varible
    strcpy(lowerCaseTitle, szimageName);
    for (int m = 0; m < strlen(lowerCaseTitle); m++) {
        lowerCaseTitle[m] = tolower(lowerCaseTitle[m]);
    }

    if (szImageTagCount > 1) {
        FILE* fp1, *fp2;
        char tempFile[MAXPATH];
        strcpy(tempFile, MODIFIED_IMAGE_FOLDER_PATH);
        strcat(tempFile, "testdata111.html");
        fp1 = fopen(szOutFolderPath, "r+");
        fp2 = fopen(tempFile, "w+");    // temp file once all image's are repalced to base64 it got delete.
        fseek(fp1, 0L, SEEK_END);
        long int fileSizeExt = ftell(fp1);
        szReadExtFile = (char*)malloc((size_t)fileSizeExt + 1);
        fseek(fp1, 0L, SEEK_SET);
        printf("\nsize of :%d\n", fileSizeExt);
        if (fp1 == NULL) {
            printf("Can't open file.");
            exit(0);
        }
        rewind(fp1);
        while (!feof(fp1)) {
            fscanf(fp1, "%s", szReadExtFile);
            if(strcmp(szReadExtFile, szimageName) == 0){
                 if ((strstr(lowerCaseTitle, ".jpg"))) {
                      fprintf(fp2, "src=\"data:image/jpg;base64,");
                 }
                 if ((strstr(lowerCaseTitle, ".png"))) {
                      fprintf(fp2, "src=\"data:image/png;base64,");
                 }
                 if ((strstr(lowerCaseTitle, ".gif"))) {
                      fprintf(fp2, "src=\"data:image/gif;base64,");
                 }
                 fprintf(fp2, szBase64);
                 fprintf(fp2, "\"");
            }

            // In last loop it runs twice
            fprintf(fp2, "%s ", szReadExtFile);

        }
        fclose(fp2);
        fclose(fp1);
        remove(szOutFolderPath);
        rename(tempFile, szOutFolderPath);
        free(szReadExtFile);
    }
    else {

        ofp = fopen(szOutFolderPath, "w+");
        ifp = fopen(szFullPathFileName, "r+");
        fseek(ifp, 0L, SEEK_END);
        long int fileSize = ftell(ifp);
        szRead = (char*)malloc((size_t)fileSize + 1);
        // Move the file pointer to START read file from begining.
        fseek(ifp, 0L, SEEK_SET);
        if (ifp == NULL | ofp == NULL) {
            printf("Can't open file.");
            exit(0);
        }
        if (ifp != NULL) {

            if (ofp != NULL) {
                // comparing word with file
                rewind(ifp);
                while (!feof(ifp)) {
                    fscanf(ifp, "%s", szRead);
                    if (strcmp(szRead, szimageName) == 0) {

                        if ((strstr(lowerCaseTitle, ".jpg"))) {
                            fprintf(ofp, "src=\"data:image/jpg;base64,");
                        }
                        if ((strstr(lowerCaseTitle, ".png"))) {
                            fprintf(ofp, "src=\"data:image/png;base64,");
                        }
                        if ((strstr(lowerCaseTitle, ".gif"))) {
                            fprintf(ofp, "src=\"data:image/gif;base64,");
                        }
                        fprintf(ofp, szBase64);
                        fprintf(ofp, "\"");
                    }
                    // In last loop it runs twice
                    fprintf(ofp, "%s ", szRead);
                }
                fclose(ifp);
                fclose(ofp);
                free(szRead);
            }

        }
    }
    return 1;
}


void FindImageTag(char* FileName, char* Path) {
    char stringDup[MAXPATH];
    char szImgTag[MAXPATH];
    char fileFullPath[MAXPATH];
    int imageTagCount = 0;
    FILE* fp;
    FILE* fp1;

    strcpy(fileFullPath, Path);
    strcat(fileFullPath, FileName);
    printf("\n File Name :%s\n", fileFullPath);
    fp = fopen(fileFullPath, "r");
    if (fp == NULL) {
        printf("Can't open file.");
        exit(0);
    }
    memset(stringDup, '\0', sizeof(stringDup));
    while (!feof(fp)) {
        fscanf(fp, "%s", stringDup);
        if (strstr(stringDup, "src=")) {
            strcpy(szImgTag, stringDup);
            if (strstr(szImgTag, "\n")) {    // remove new line from image name if exist.
                char new_line[] = "\n";
                char* new_token = strtok(szImgTag, new_line);
                strcpy(szImgTag, new_token);
            }
            char delimiter1[] = "=";
            char* token1 = strtok(stringDup, delimiter1);
            char lowerCaseTitle[MAXPATH];
            strcpy(lowerCaseTitle, token1);
            for (int m = 0; m < strlen(lowerCaseTitle); m++) {
                lowerCaseTitle[m] = tolower(lowerCaseTitle[m]);
            }
            while (token1 != NULL)
            {
                if ((strstr(lowerCaseTitle, ".png")) || (strstr(lowerCaseTitle, ".jpg")) || (strstr(lowerCaseTitle, ".gif"))) {
                    char src_value[MAXPATH] = { 0 };
                    strcpy(src_value, token1);
                    int i = 0; int j = 0;
                    char temp[MAXPATH] = { 0 };
                    char fullImagePath[MAXPATH] = { 0 };
                    for (i = 1; i < strlen(src_value) - 1; i++) {
                        temp[j++] = src_value[i];
                    }
                    imageTagCount++;
                    if (strstr(temp, "\"")) {
                        char delimiter2[] = "\"";
                        char* token2 = strtok(temp, delimiter2);
                        strcpy(temp, token2);
                    }
                    strcpy(fullImagePath, Path);
                    strcat(fullImagePath, temp);
                    char* szOutPutValue;
                    fp1 = fopen(fullImagePath, "rb");
                    // Move the file pointer to EOF and get the size of the file.
                    fseek(fp1, 0L, SEEK_END);
                    long int fileSize = ftell(fp1);
                    szOutPutValue = (char*)malloc((size_t)fileSize + 1);
                    // Move the file pointer to START read file from begining.
                    fseek(fp1, 0L, SEEK_SET);
                    fread(szOutPutValue, fileSize, 1, fp1);
                    fclose(fp1);
                    int result = 0;
                    char* base64;
                    size_t base64len;
                    result = base64_encode(base64encdec, szOutPutValue, (size_t)fileSize, &base64, &base64len);
                    ReplaceImageToBase64image(fileFullPath, base64, szImgTag, imageTagCount, FileName);
                    free(szOutPutValue);
                }
                token1 = strtok(NULL, delimiter1);

            }
        }
        else
        {
            printf("");
        }

    }
    fclose(fp);
    return 1;
}

static int base64_encode(const char* table64,
    const char* inputbuff, size_t insize,
    char** outptr, size_t* outlen)
{
    char* output;
    char* base64data;
    const unsigned char* in = (unsigned char*)inputbuff;
    const char* padstr = &table64[64]; /* Point to padding string*/
    *outptr = NULL;
    *outlen = 0;

    if (!insize)
        insize = strlen(inputbuff);

#if SIZEOF_SIZE_T == 4
    if (insize > UINT_MAX / 4)
        return 0;
#endif

    base64data = output = malloc((insize + 2) / 3 * 4 + 1);
    if (!output)
        return 0;
    while (insize >= 3)
    {
        *output++ = table64[in[0] >> 2];
        *output++ = table64[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *output++ = table64[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)];
        *output++ = table64[in[2] & 0x3F];
        insize -= 3;
        in += 3;
    }
    if (insize)
    {
        /* this is only one or two bytes now */
        *output++ = table64[in[0] >> 2];
        if (insize == 1)
        {
            *output++ = table64[((in[0] & 0x03) << 4)];
            if (*padstr)
            {
                *output++ = *padstr;
                *output++ = *padstr;
            }
        }
        else
        {
            /* insize == 2 */
            *output++ = table64[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
            *output++ = table64[((in[1] & 0x0F) << 2)];
            if (*padstr)
                *output++ = *padstr;
        }
    }
    /* Zero terminate */
    *output = '\0';

    /* Return the pointer to the new data (allocated memory) */
    *outptr = base64data;

    /* Return the length of the new data */
    *outlen = output - base64data;

    return 1;
}

// This mehtod takes input as html folder path and 
//iterate the all files and stored as arraylist.
int ListAllFileInFolder(char* dirPath) {

    DIR* dir;
    struct dirent* entry;
    // Open the directory
    dir = opendir(dirPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }

    int countFiles = 0;
    // Read files from the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories
        if (entry->d_type == DT_DIR) continue;
        char* fileName = malloc(strlen(dirPath) + strlen(entry->d_name) + 1);
        strcpy(fileName, entry->d_name);
        countFiles++;
        if (strstr(fileName, ".html")) {

            FindImageTag(fileName, dirPath);
        }
        free(fileName);  //free memory
    }

    // Close the directory
    closedir(dir);
    return countFiles;
}

int main(int argc, char* argv[]) {

    
    char        achTempName[MAXPATH];
    char* szModPath, * szFilePath, * szNSFFile, * szviewName , * szDLL;
    NOTEID viewNoteID;
    DHANDLE     hNewNote;
    DBHANDLE    hDbNSFFile;
    HCOLLECTION hCollection;
    DBREPLICAINFO  Replicainfo;
    DBID        input_dbid;         /* dbid of input database */
    DWORD  note_count = 0;
    STATUS      usError = 0;
    int        duplicate = 0;
    char* htmlFileName;
    char        szTimedate[MAXALPHATIMEDATE + 1];
    WORD        wLen;
    UNID            Note_UNID;
    ORIGINATORID    Note_OID;
    int totalCount = 0;

    /* Start by calling Notes Init. */
    usError = NotesInitExtended(argc, argv);
    if (usError)
    {
        PRINTLOG("Error: Unable to initialize Notes.\n");
        return (1);
    }

    //Sleep(30000);
    if (argc != 5 && argc != 6)
    {
        PRINTLOG("\nUsage: \n\tlgimport <NSF Filename>");
        PRINTLOG(" <Import/Export DLL Name> <Import File Path>");
        PRINTLOG(" <Second DLL Name> (if necessary)\n");
        return(0);
    }

    szNSFFile = argv[1];
    szModPath = argv[2];
    szFilePath = argv[3];
    szviewName = argv[4];


    /* If a 2nd DLL name was entered, set szDLL to it.  Otherwise, set */
    /* szDLL to null.                                                  */

    if (argc == 6)
        szDLL = argv[5];
    else
        szDLL = 0;

    int totalFileCount = ListAllFileInFolder(szFilePath);

    char workingDir[MAXPATH];
    char source[MAXPATH];

    getcwd(workingDir, sizeof(workingDir)); // get current working directory 
    printf("Current working directory: %s\n", szFilePath);
    for (int i = 0; szFilePath[i]; i++) {
        if (szFilePath[i] == "\"") {
            szFilePath[i] = "\\";
        }
    }
    sprintf(source, "cp -r %s %s", szFilePath, workingDir);  //copy files from source current sample folder
    system(source);
    
    struct key_value* kv = malloc(sizeof(struct key_value) * totalFileCount);

    /* The code copy the files from modified image folder to sample folder. */
    char modifiedImageFile[MAXPATH];
    sprintf(modifiedImageFile, "cp -r %s %s", MODIFIED_IMAGE_FOLDER_PATH, workingDir);
    system(modifiedImageFile);

    struct dirent* de;  // Pointer for directory entry 
    DIR* dr = opendir(workingDir);
    
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    {
        printf("Could not open current directory");
        return 0;
    }

    /* Open the Database, assuming it's been created with the   */
    /* template shipped with this demo.                         */

    if (usError = NSFDbOpen(szNSFFile, &hDbNSFFile))
    {
        PRINTLOG("\nCannot open database '%s'. Terminating...\n", szNSFFile);
        PRINTERROR(usError, "NSFDbOpen");
        goto Done;
    }

    
    if (usError = NSFDbReplicaInfoGet(hDbNSFFile, &Replicainfo)) {
        goto Done;
    }

    char szReplicaID[MAXPATH];
    char szReplicaSecOrd[MAXPATH];
    sprintf(szReplicaID, "%lx", Replicainfo.ID.Innards[1]);
    int rplicalength = (8 - strlen(szReplicaID));
    if (rplicalength == 1) {
        char s[2] = "0";
        // function concatenate_string 
        concatenate_string(s, szReplicaID);
        strcpy(szReplicaID, s);

    }
    if (rplicalength == 2) {
        char s[4] = "00";
        // function concatenate_string 
        concatenate_string(s, szReplicaID);
        strcpy(szReplicaID, s);

    }
    if (rplicalength == 3) {
        char s[8] = "000";
        // function concatenate_string 
        concatenate_string(s, szReplicaID);
        strcpy(szReplicaID, s);

    }

    sprintf(szReplicaSecOrd, "%lx", Replicainfo.ID.Innards[0]);
    int replicaSeclength = (8 - strlen(szReplicaSecOrd));
    if (replicaSeclength == 1) {
        char s[2] = "0";
        // function concatenate_string 
        concatenate_string(s, szReplicaSecOrd);
        strcpy(szReplicaSecOrd, s);

    }
    if (replicaSeclength == 2) {
        char s[4] = "00";
        // function concatenate_string 
        concatenate_string(s, szReplicaSecOrd);
        strcpy(szReplicaSecOrd, s);

    }
    if (replicaSeclength == 3) {
        char s[8] = "000";
        // function concatenate_string 
        concatenate_string(s, szReplicaSecOrd);
        strcpy(szReplicaSecOrd, s);

    }
    strcat(szReplicaID, szReplicaSecOrd);  //concat two ords

    printf("\nReplica ID: %s\n", szReplicaID);

    printf("\n Wait now all .html files are Importing to the .Nsf ..........\n");
    
    if (usError = NIFFindView(hDbNSFFile, szviewName, &viewNoteID))
    {
        if (usError == ERR_NOT_FOUND)
        {
            printf("View '%s' cannot be found\n", szviewName);
            goto Done;
        }
        goto Done;
    }


    while ((de = readdir(dr)) != NULL) {
        htmlFileName = malloc(sizeof(de->d_name));
        memset(htmlFileName, '\0', sizeof(de->d_name));
        strncpy(htmlFileName, de->d_name, sizeof(de->d_name));
        if (strlen(htmlFileName) >= 3) {
            if (strstr(htmlFileName, ".html")) {

                duplicate++;

                /* source file to create a char buffer containing the data      */
                /* that will be appended to a CD buffer.                        */

                if (usError = ImportCD(szModPath, htmlFileName, achTempName, szDLL))
                {
                    PRINTLOG("\nUnable to create temporary file. Terminating...\n");
                    goto Done;
                }

                /* Create a Note */
                if (usError = NSFNoteCreate(hDbNSFFile, &hNewNote))
                {
                    PRINTLOG("\nCannot create new note. Terminating...\n");
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "NSFNoteCreate");
                    goto Done;
                }

                /* Add the Form name */
                if (usError = NSFItemSetText(hNewNote,
                    "Form",
                    "Simple Form",
                    (WORD)strlen("Simple Form")))
                {
                    PRINTLOG("\nError while setting Form name. Terminating...\n");
                    NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "NSFItemSetText");
                    goto Done;
                }

                // Remove the .html file extension in FileTitle  

                int i = 0, j = 0, k;
                char word[MAXPATH] = { ".html" };
                for (i = 0; i < strlen(htmlFileName); i++)
                {
                    while (htmlFileName[i] == word[j] && word[j] != '\0') //To search the word in string
                    {
                        for (k = i; htmlFileName[k] != '\0'; k++)
                        {
                            htmlFileName[k] = htmlFileName[k + 1]; //To override the word by next words
                        }
                        j++;

                    }
                }

                //Add the Title Name
                if (usError = NSFItemSetText(hNewNote,
                    "Name",
                    htmlFileName,
                    (WORD)strlen(htmlFileName)))
                {
                    PRINTLOG("\nError while setting Form name. Terminating...\n");
                    NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "NSFItemSetText");
                    goto Done;
                }

                if (usError = removeFirstLine(achTempName))
                {
                    PRINTLOG("\nError while deleting garbage values.  Terminating...\n");
                    NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "removeFirstLine");
                    goto Done;
                }

                if (usError = AppendImportItem(hNewNote, achTempName, "Body"))
                {
                    PRINTLOG("\nError while adding rich text.  Terminating...\n");
                    NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "AppendImportItem");
                    goto Done;
                }

                /* write the note */

                if (usError = NSFNoteUpdate(hNewNote, 0))
                {
                    PRINTLOG("\nError writing new Note. Terminating..\n");
                    NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
                    NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
                    PRINTERROR(usError, "NSFNoteUpdate");
                    goto Done;
                }

                /* Start cleaning up. First, Close the Note */

                if (usError = NSFNoteClose(hNewNote))
                {
                    PRINTLOG("\nUnable to close Note.  Terminating..\n");
                    PRINTERROR(usError, "NSFNoteClose");
                    goto Done;
                }
            }
        }
    }
    /* Open the collection of notes in the view at the current time.
       Return a handle to the collection to the variable hCollection. */
    if (usError = NIFOpenCollection(hDbNSFFile, hDbNSFFile, viewNoteID,
        0,
        NULLHANDLE,
        &hCollection,
        NULL, NULL, NULL, NULL))
    {
        goto Done;
    }

    {
        COLLECTIONPOSITION indexPos;
        DWORD entriesReturned;
        DHANDLE hBuffer;
        WORD   signalFlag;             /* signal and share warning flags */

        indexPos.Level = 0;
        indexPos.Tumbler[0] = 0;
        //indexPos.Tumbler[0] = 1;

        do
        {
            if (usError = NIFReadEntries(hCollection,
                &indexPos,
                NAVIGATE_NEXT, 1L,
                NAVIGATE_NEXT, MAXDWORD,
                READ_MASK_NOTEID,
                &hBuffer, NULL,
                NULL, &entriesReturned, &signalFlag))
            {
                goto Done;
            }

            /********************************************************************/

            if (hBuffer != NULLHANDLE)
            {
                char string[512];
                WORD wItemtype = TYPE_HTML;
                FILE* fp;
                struct dirent* de;  // Pointer for directory entry 
                DIR* dr = opendir(workingDir);

                if (dr == NULL)  // opendir returns NULL if couldn't open directory 
                {
                    printf("Could not open current directory");
                    fflush(stdout);
                    return 0;
                }
                int count = 0;
                int i = 0;
                NOTEHANDLE hNoteFirstDoc, hNoteSecDoc;
                NOTEID* entry = OSLock(NOTEID, hBuffer);
                while ((de = readdir(dr)) != NULL) {
       
                    htmlFileName = malloc(sizeof(de->d_name));
                    memset(htmlFileName, '\0', sizeof(de->d_name));
                    strncpy(htmlFileName, de->d_name, sizeof(de->d_name));
                    if (strlen(htmlFileName) >= 3) {
                        if (strstr(htmlFileName, ".html")) {
                            
                            /* process each individual noteID       */
                            /* skip this noteID if it is for a category entry  */
                            if (*entry & NOTEID_CATEGORY)
                            {
                                entry++;
                            }
                            
                            char docUNID[1000];
                            char lpszURL[MAXPATH];
                            if (!(NOTEID_CATEGORY & entry[i]))

                                PRINTLOG("Note ID number %lu is: %lX\n", ++note_count, entry[i]);
                            if (usError = NSFNoteOpen(hDbNSFFile, entry[i], 0, &hNoteFirstDoc))
                            {
                                
                                printf("Error '%s' reading docment %#lX -- %s\n", string, entry[i], " skipping it");
                                goto Done;
                            }

                            NSFNoteGetInfo(hNoteFirstDoc, _NOTE_OID, &Note_OID);
                            char szReplicaFile[MAXPATH];
                            char szReplicaNote[MAXPATH];
                            char szfirstTemp[MAXPATH];
                            char szsecondTemp[MAXPATH];

                            sprintf(szReplicaFile, "%lx", Note_OID.File.Innards[1]);

                            int length = (8 - strlen(szfirstTemp));
                            if (length == 1) {
                                char s[2] = "0";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaFile);
                                strcpy(szReplicaFile, s);

                            }
                            if (length == 2) {
                                char s[4] = "00";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaFile);
                                strcpy(szReplicaFile, s);

                            }
                            if (length == 3) {
                                char s[8] = "000";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaFile);
                                strcpy(szReplicaFile, s);

                            }

                            sprintf(szfirstTemp, "%lx", Note_OID.File.Innards[0]);
                            
                            int szfirstlength = (8 - strlen(szfirstTemp));
                            if (szfirstlength == 1) {
                                char s[2] = "0";
                                // function concatenate_string 
                                concatenate_string(s, szfirstTemp);
                                strcpy(szfirstTemp, s);

                            }
                            if (szfirstlength == 2) {
                                char s[4] = "00";
                                // function concatenate_string 
                                concatenate_string(s, szfirstTemp);
                                strcpy(szfirstTemp, s);

                            }
                            if (szfirstlength == 3) {
                                char s[8] = "000";
                                // function concatenate_string 
                                concatenate_string(s, szfirstTemp);
                                strcpy(szfirstTemp, s);

                            }
                            strcat(szReplicaFile, szfirstTemp);

                            sprintf(szReplicaNote, "%lx", Note_OID.Note.Innards[1]);

                            int szlength = (8 - strlen(szReplicaNote));
                            if (szlength == 1) {
                                char s[2] = "0";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaNote);
                                strcpy(szReplicaNote, s);

                            }
                            if (szlength == 2) {
                                char s[4] = "00";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaNote);
                                strcpy(szReplicaNote, s);

                            }
                            if (szlength == 3) {
                                char s[8] = "000";
                                // function concatenate_string 
                                concatenate_string(s, szReplicaNote);
                                strcpy(szReplicaNote, s);

                            }

                            sprintf(szsecondTemp, "%x", Note_OID.Note.Innards[0]);

                            int szsecondlength = (8 - strlen(szsecondTemp));
                            if (szsecondlength == 1) {
                                char s[2] = "0";
                                // function concatenate_string 
                                concatenate_string(s, szsecondTemp);
                                strcpy(szsecondTemp, s);

                            }
                            if (szsecondlength == 2) {
                                char s[4] = "00";
                                // function concatenate_string 
                                concatenate_string(s, szsecondTemp);
                                strcpy(szsecondTemp, s);

                            }
                            if (szsecondlength == 3) {
                                char s[8] = "000";
                                // function concatenate_string 
                                concatenate_string(s, szsecondTemp);
                                strcpy(szsecondTemp, s);

                            }
                            strcat(szReplicaNote, szsecondTemp);
                            strcat(szReplicaFile, szReplicaNote);

                            //preparing the doc url for each document 
                            char szDocURL[1000];
                            strcpy(szDocURL, "notes:///__");
                            strcat(szDocURL, szReplicaID);
                            strcat(szDocURL, ".nsf/0/");
                            strcat(szDocURL, szReplicaFile);
                            strcat(szDocURL, "?OpenDocument");

                           // store key and values in structure(key =filename, Value =notesURL);
                            kv[i].key = htmlFileName;
                            strcpy(kv[i].value, szDocURL);
                            i++;
                            totalCount = i;
                        }
                    }
                }
            }   /* if (hBuffer != NULLHANDLE) */

        } while (signalFlag & SIGNAL_MORE_TO_DO);
 
        NIFCloseCollection(hCollection);

        //goto Done;
    }

    /* This fuction replace the hyperlinks to notesURL. */

    ReplaceHyperLinksToNotesURL(workingDir, totalCount, kv);

    /* This code copy the files from modified hyper folder to sample folder. */

    char sourceFile[MAXPATH];
    sprintf(sourceFile, "cp -r %s %s", TARGET_FOLDER_PATH, workingDir);
    system(sourceFile);

    /* This method modified the body field in each document in current database. */
     
    ModifyBodyFieldInEachDocument(hDbNSFFile, viewNoteID , workingDir);

    free(kv); // free memory

    /* Now close the database. */
    if (usError = NSFDbClose(hDbNSFFile))
    {
        PRINTLOG("\nError closing database '%s'. Terminating...\n, szNSFFile");
        PRINTERROR(usError, "NSFDbClose");
        goto Done;
    }

    PRINTLOG("\nAll Done!\n");
    
    NotesTerm();

    PRINTLOG("\nProgram completed successfully.\n");

    return(0);

Done:
    NotesTerm();
    return(1);
}

/**********************************************************************

   FUNCTION:   ImportCD

   Inputs:    szModulePath - name of import DLL
              szFileName   - name of file to be imported
   Outputs:   szTempName   - Name of temp file containing imported data

************************************************************************/

STATUS LNPUBLIC ImportCD(char* szModulePath, char* szFileName,
    char* szTempName, char* szDLL)
{
    EDITIMPORTDATA EditImportData;            /* Import DLL data structure                 */
    HMODULE        hmod;                      /* module handle                             */
    STATUS         Error;                     /* Return status from Domino and Notes calls */
    char* FileName;                           /* File name to be imported                  */
    char           TempName[] = "DEFAULT.CD"; /* Temp Filename for import.                 */
    char* ModuleName;                         /* pointer to DLL module name                */
    char* SecondDLLName;

    ModuleName = szModulePath;
    FileName = szFileName;
    SecondDLLName = szDLL;

    /* Use OSLoadLibrary to load the import DLL and return a pointer to */
    /* the main entry point.                                             */

    if (Error = OSLoadLibrary(ModuleName, (DWORD)0, &hmod, &ProcAddress))
    {
        PRINTLOG("OSLoadLibrary failed.\n");
        goto Done;
    }

    strcpy(EditImportData.OutputFileName, TempName);
    //PRINTLOG("\nTemp filename is %s.\n", EditImportData.OutputFileName);

    /* Assign the default fontid */

    EditImportData.FontID = DEFAULT_FONT_ID;

    if (Error = (*ProcAddress) (&EditImportData,
        IXFLAG_FIRST | IXFLAG_LAST,         /* Both 1st and last import */
        0,                                  /* Use default hmodule      */
        SecondDLLName,                      /* 2nd DLL, if needed.      */
        FileName))                          /* File to import.          */
    {
        PRINTLOG("Call to DLL Entry point failed.\n");
        goto Done;
    }

    /* return the temp filename to calling routine */

    strcpy(szTempName, EditImportData.OutputFileName);

Done:
    /* Free the DLL and return */
    OSFreeLibrary(hmod);
    return Error;

}

/*********************************************************************
FUNCTION : removeFirstLine

*********************************************************************/

STATUS LNPUBLIC removeFirstLine(char* cdFileName) {
    /* Now add the imported data. */
    FILE* fp1, * fp2;
    char d;
    int del_line = 1, temp = 1;
    char* filename = malloc(strlen(cdFileName) + 1);

    strcpy(filename, cdFileName);
 
    //open file in read mode
    fp1 = fopen(filename, "r");

    //rewind
    rewind(fp1);
    //open new file in write mode
    fp2 = fopen("copy.txt", "w");
    d = fgetc(fp1);
    putc("-", fp2);
    putc("-", fp2);
    while (d != EOF) {
        d = fgetc(fp1);
        if ((d == '\n') || (d == '<'))
        {
            temp++;
        }
        //except the line to be deleted
        if (temp != del_line)
        {
            //copy all lines in file copy.txt
            putc(d, fp2);
        }
    }
    //close both the files.
    fclose(fp1);
    fclose(fp2);

    //remove original file
    remove(filename);

    //rename the file copy.txt to original name
    rename("copy.txt", filename);
    return (NOERROR);
}


/************************************************************************

    FUNCTION:   AppendImportItem

    PURPOSE:    Adds a Rich Text item to the note that is being
                  created.
    INPUTS:
      hNote       - handle to the new note being created.
      pszCDFile   - pointer to name of temporary file.
      pszItemName - pointer to name of richtext item.

*************************************************************************/

STATUS LNPUBLIC AppendImportItem(NOTEHANDLE hNote,
    char* pszCDFile,
    char* pszItemName)
{
    STATUS sError;           /* Domino and Notes error status                 */
    BOOL   bError;           /* Returncode from PutPara, PutPabDef, PutPabRef */
    char* pCDBuffer;        /* Rich Text memory buffer                       */
    char* pCurrent;         /* Pointer to current location of buffer         */

    BYTE* pbSig;            /* Byte pointer to start of new CD record        */
    WORD  wSig;              /* Signature of current CD record                */
    WORD  wCDRecordLength;   /* Length of current CD record                   */

    long  lCombinedLength;

    WORD  wCDBufferLength = CD_BUFFER_LENGTH; /* Length of current CD buffer */
    long  lLength;                            /* Length of current read buffer*/
    WORD  wReadLength;                        /* Length of current read buffer*/

    WORD  dwPos = sizeof(WORD);    /* Index for buffer manipulation           */
    long  longpos = sizeof(WORD);  /* Initialy seek past TYPE_COMPOSITE       */
    /*      at start of file                   */
    long  ltmpItemLength;
    WORD  wItemLength;         /* Index for buffer manipulation           */
    BOOL  bFlag = FALSE;       /* termination flag                        */
    int   CDFileFD;


    // CDFileFD = open (pszCDFile, O_RDWR|O_BINARY);
    CDFileFD = open(pszCDFile, 0x0002 | 0x8000);
    if (CDFileFD <= 0)
    {
        /* Leave if error returned... */

        unlink(pszCDFile);
        return (ERR_APPEND_RICHTEXT_ERROR);
    }

    /* Allocate a buffer to read file into  */

    pCDBuffer = (char*)malloc(wCDBufferLength);
    memset(pCDBuffer, '\0', wCDBufferLength);
    if (pCDBuffer == (char*)NULL)
    {
        /* Leave if error returned... */

        close(CDFileFD);
        unlink(pszCDFile);
        return (ERR_APPEND_RICHTEXT_ERROR);
    }

    /* Keep a pointer to our current position in the buffer. */

    pCurrent = pCDBuffer;
    
    /* Set start length to zero */

    wItemLength = 0;

    /*  Put a paragraph record in buffer. */

    bError = PutPara(&pCurrent,
        (WORD)(wCDBufferLength - (pCurrent - pCDBuffer)));

    if (bError == FALSE)
    {
        PRINTLOG("Error adding paragraph record.  Terminating...");
        free(pCDBuffer);
        close(CDFileFD);
        unlink(pszCDFile);
        return (ERR_APPEND_RICHTEXT_ERROR);

    }

    /* Setup a pabdef */

    bError = PutPabDef(&pCurrent,
        (WORD)1,
        (WORD)(wCDBufferLength - (pCurrent - pCDBuffer)),
        JUSTIFY_LEFT);

    /* Leave if error returned... */

    if (bError == FALSE)
    {
        PRINTLOG("Error adding paragraph definition record.  Terminating...");
        free(pCDBuffer);
        close(CDFileFD);
        unlink(pszCDFile);
        return (ERR_APPEND_RICHTEXT_ERROR);
    }


    /* Now add a pabref */

    bError = PutPabRef(&pCurrent,
        (WORD)1,
        (WORD)(wCDBufferLength - (pCurrent - pCDBuffer)));

    wItemLength = pCurrent - pCDBuffer;

    /* Leave if error returned... */

    if (bError == FALSE)

    {
        PRINTLOG("Error adding paragraph reference record.  Terminating...");
        free(pCDBuffer);
        close(CDFileFD);
        unlink(pszCDFile);
        return (ERR_APPEND_RICHTEXT_ERROR);
    }

    /* Keep on writing items until entire cd file hase been appended */
    while (bFlag == FALSE)
    {
        /* Seek file to end of previous CD record */
        memset(pCDBuffer, '\0', wCDBufferLength);
        if (lseek(CDFileFD, longpos, SEEK_SET) != longpos)
        {
            /* Leave if error returned... */

            free(pCDBuffer);
            close(CDFileFD);
            unlink(pszCDFile);
            return (ERR_APPEND_RICHTEXT_ERROR);
        }

        /* Read the contents of the file into memory */
        wReadLength = read(CDFileFD,
            &pCDBuffer[wItemLength],
            (WORD)(wCDBufferLength - wItemLength));




        /* check for error */
     
        if (wReadLength == 0xffff)
        {
            /* Leave if error returned... */

            free(pCDBuffer);
            close(CDFileFD);
            unlink(pszCDFile);
            return (ERR_APPEND_RICHTEXT_ERROR);
        }
        /* See whether the contents will fit in current item....  */
        if (wReadLength < CD_HIGH_WATER_MARK)
        {
            /* we can fit what is left in a single buffer and leave  */
            
            bFlag = TRUE;
            wItemLength += wReadLength;
        }
        else
        {
            wCDRecordLength = 0;
            lLength = 0;
            lCombinedLength = wItemLength + wCDRecordLength;
            while (lCombinedLength < CD_HIGH_WATER_MARK)
            {
                pbSig = (BYTE*)&pCDBuffer[wItemLength];
                wSig = *(WORD*)pbSig;
                pbSig++;

                /* find length of CD record. */

                if (*pbSig == 0)    				/* record length is a DWORD  */
                {
                    pbSig++;
                    wCDRecordLength = (WORD) * (DWORD*)pbSig;
                }

                else if (*pbSig == 0xFF)        	/* record length is a WORD  */
                {
                    pbSig++;
                    wCDRecordLength = *(WORD*)pbSig;
                }

                else	                            /* record length is the BYTE */
                    wCDRecordLength = (WORD)*pbSig;

                if (wCDRecordLength % 2)
                    wCDRecordLength++;

                lLength = lLength + (long)wCDRecordLength;
                ltmpItemLength = (long)wItemLength + (long)wCDRecordLength;
                if (ltmpItemLength < CD_BUFFER_LENGTH)
                    wItemLength += wCDRecordLength;
                lCombinedLength = (long)ltmpItemLength + (long)wCDRecordLength;
                
            }
        }
        /* Copy the imported item to the note if so  */

        if (wItemLength > 0)
        {
            if (sError = NSFItemAppend(hNote,
                (WORD)0,
                pszItemName,
                (WORD)strlen(pszItemName),
                TYPE_HTML,
                pCDBuffer,
                (DWORD)wItemLength))
            {
                /* Leave if error returned... */

                free(pCDBuffer);
                close(CDFileFD);
                unlink(pszCDFile);
                return (sError);
            }
            //free(pCDBuffer);
        }

        longpos += lLength;
        wItemLength = 0;
    }

    /* close up and leave ..*/

    free(pCDBuffer);

    close(CDFileFD);

    unlink(pszCDFile);

    return (NOERROR);
}
