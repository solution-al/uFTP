/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ftpServer.h"
#include "ftpCommandsElaborate.h"
#include "ftpData.h"
#include "fileManagement.h"

void cleanDynamicStringDataType(dynamicStringDataType *dynamicString, int init)
{
    if (init == 1)
    {
        dynamicString->text = 0;
        dynamicString->textLen = 0;
    }
    else
    {
        if (dynamicString->textLen != 0)
        {
            if (dynamicString->text != 0) {
                free(dynamicString->text);
            }
        }

        dynamicString->textLen = 0;
    }
}

void cleanLoginData(loginDataType *loginData, int init)
{
    cleanDynamicStringDataType(&loginData->homePath, init);
    cleanDynamicStringDataType(&loginData->ftpPath, init);
    cleanDynamicStringDataType(&loginData->name, init);
    cleanDynamicStringDataType(&loginData->password, init);
    cleanDynamicStringDataType(&loginData->absolutePath, init);
}

void setDynamicStringDataType(dynamicStringDataType *dynamicString, char *theString, int stringLen)
{
    if (dynamicString->textLen == 0)
    {
        dynamicString->text = (char *) malloc ((sizeof(char) * stringLen) + 1);
        memset(dynamicString->text, 0, stringLen + 1);
        memcpy(dynamicString->text, theString, stringLen);
        dynamicString->textLen = stringLen;
    }
    else
    {
        if(stringLen != dynamicString->textLen) {
            dynamicString->text = (char *) realloc (dynamicString->text, (sizeof(char) * stringLen) + 1);
        }

        memset(dynamicString->text, 0, stringLen + 1);
        memcpy(dynamicString->text, theString, stringLen);
        dynamicString->textLen = stringLen; 
    }
}

void setRandomicPort(ftpDataType *data, int socketPosition)
{
    int randomicPort = 0;
    time_t t;

   /* Intializes random number generator */
   srand((unsigned) time(&t));
   randomicPort = (rand() % (30000 - 50000)) + 30000;

   data->clients[socketPosition].pasvData.passivePort = randomicPort;
}

void getListDataInfo(char * thePath, DYNV_VectorGenericDataType *directoryInfo)
{
    int i;
    int fileAndFoldersCount = 0;
    ftpListDataType data;
    
    data.fileList = NULL;
    
    printf("\nThePath = %s", thePath);
    
    FILE_GetDirectoryInodeList(thePath, &data.fileList, &fileAndFoldersCount, 0);
    
    for (i = 0; i < fileAndFoldersCount; i++)
    {
        data.numberOfSubDirectories = 1; /* to Do*/
        
        if (FILE_IsDirectory(data.fileList[i]) == 1)
        {
            data.isDirectory = 1;
            data.isFile = 0;
            data.fileSize = 4096;
        }
        else if (FILE_IsFile(data.fileList[i]) == 1)
        {
            data.isDirectory = 0;
            data.isFile = 1;
            data.fileSize = FILE_GetFileSizeFromPath(data.fileList[i]);
        }

        data.owner = FILE_GetOwner(data.fileList[i]);
        data.groupOwner = FILE_GetGroupOwner(data.fileList[i]);
        data.fileNameWithPath = data.fileList[i];
        data.fileNameNoPath = FILE_GetFilenameFromPath(data.fileList[i]);
        data.inodePermissionString = FILE_GetListPermissionsString(data.fileList[i]);
        data.lastModifiedData = FILE_GetLastModifiedData(data.fileList[i]);

        memset(data.lastModifiedDataString, 0, LIST_DATA_TYPE_MODIFIED_DATA_STR_SIZE);       
        
        strftime(data.lastModifiedDataString, 80, "%b %d %Y", localtime(&data.lastModifiedData));

        /*
        %a	Abbreviated weekday name	Sun
        %A	Full weekday name	Sunday
        %b	Abbreviated month name	Mar
        %B	Full month name	March
        %c	Date and time representation	Sun Aug 19 02:56:02 2012
        %d	Day of the month (01-31)	19
        %H	Hour in 24h format (00-23)	14
        %I	Hour in 12h format (01-12)	05
        %j	Day of the year (001-366)	231
        %m	Month as a decimal number (01-12)	08
        %M	Minute (00-59)	55
        %p	AM or PM designation	PM
        %S	Second (00-61)	02
        %U	Week number with the first Sunday as the first day of week one (00-53)	33
        %w	Weekday as a decimal number with Sunday as 0 (0-6)	4
        %W	Week number with the first Monday as the first day of week one (00-53)	34
        %x	Date representation	08/19/12
        %X	Time representation	02:50:06
        %y	Year, last two digits (00-99)	01
        %Y	Year	2012
        %Z	Timezone name or abbreviation	CDT
        %%	A % sign	%
         */
        /*
        printf("\n\ndata.numberOfSubDirectories = %d", data.numberOfSubDirectories);
        printf("\ndata.isDirectory = %d", data.isDirectory);
        printf("\ndata.isFile = %d", data.isFile);
        printf("\ndata.fileSize = %d", data.fileSize);
        printf("\ndata.owner = %s", data.owner);
        printf("\ndata.groupOwner = %s", data.groupOwner);
        printf("\ndata.fileNameWithPath = %s", data.fileNameWithPath);
        printf("\ndata.fileNameNoPath = %s", data.fileNameNoPath);
        printf("\ndata.inodePermissionString = %s", data.inodePermissionString);
        printf("\ndata.lastModifiedDataString = %s", data.lastModifiedDataString);
        */
        directoryInfo->PushBack(directoryInfo, &data, sizeof(ftpListDataType));
    }
    
    /*
    printf("\n\ntotal %d", directoryInfo->Size);
    for (i = 0; i < directoryInfo->Size; i++)
    {
        printf("\n%s %d %s %s %d %s %s", ((ftpListDataType *)directoryInfo->Data[i])->inodePermissionString
        ,((ftpListDataType *)directoryInfo->Data[i])->numberOfSubDirectories
        ,((ftpListDataType *)directoryInfo->Data[i])->owner
        ,((ftpListDataType *)directoryInfo->Data[i])->groupOwner
        ,((ftpListDataType *)directoryInfo->Data[i])->fileSize
        ,((ftpListDataType *)directoryInfo->Data[i])->lastModifiedDataString
        ,((ftpListDataType *)directoryInfo->Data[i])->fileNameNoPath);
    }
    */
}

void deleteListDataInfoVector(void *TheElementToDelete)
{
    ftpListDataType * data = TheElementToDelete;

    free(data->permissions);
    free(data->owner);
    free(data->inodePermissionString);
    free(data->fileNameWithPath);
}