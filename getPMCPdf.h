/*######################################################################
# Name: getPMCPdf.h
# Use:
#  - Holds structures and functions to downloads a pdf from pubmed
#    central (PMC) using curl
# Dependencies:
#  - <stdlib.h>
#  - <string.h>
#  - <stdio.h>
#  - <curl/curl.h>
#  - "cStrFun.h"
######################################################################*/

#ifndef GETPMCPDF_H
#define GETPMCPDF_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include "cStrFun.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' SOH: Start Of Header
'  - struct-01 buffStruct:
'     o Holds a buffer of characters
'  - struct-02 webCrawl:
'     o Holds a curl handle and a buffer for the handle
'  - struct-03 filetruct:
'     o Holds the name of a file & the pointer to it
'  - struct-04 webGetPdf:
'     o Holds curl handle & the file for it to write to
'  - fun-01 getPMCPdf:
'     o Driver function to download a pdf from pubmed centeral
'  - fun-02 writeDataToBuff:
'     o Function to write data output from curl
'  - fun-03 makeWebCrawlST:
'     o makes a webCrawl structer with a curl handle and a buffer
'  - fun-04 freeWebCrawlST:
'     o Frees a webCrawlST delared
'  - fun-05 setWebCrawlURL:
'     o Sets up changes the url for the webCrawl structure
'  - fun-06 findPdfLink:
'     o Find the pdf link for a pubmed centeral id (PMCID)
'  - fun-07 curlWriteToFile:
'     o writes the output from curl to a file
'  - fun-08 makeWebGetPdfST:
'     o makes a webGetPdf structure on the heap
'  - fun-09 freeWebGetPdfST:
'     o Frees a webGetPdf structer and its variables
'  - fun-10 downloadPdf:
'     o Downloads a pdf to the file in webGetPdfST
'  - fun-11 webGetChangeFile:
'     o Changes the output file for a webGetPdf structuer
'  - fun-12 setWebGetPdfURL:
'     o Sets up changes the url for the webGetPdf structure
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Struct-01: buffStruct: Holds a buffer of characters
\---------------------------------------------------------------------*/
typedef struct buffStruct
{ // buffStruct
    char *buffCStr;
    size_t lenBuff;
    size_t numElmInBuff;
}buffStruct;

/*---------------------------------------------------------------------\
| Struct-02: webCrawl: Holds a curl handle and a buffer for the handle
\---------------------------------------------------------------------*/
typedef struct webCrawl
{ // webCral
    CURL *curlHandle;          // Curl handle to do search with
    struct buffStruct *buffST; // Buffer holding curls output
}webCrawl;

/*---------------------------------------------------------------------\
| Struct-03: filetruct: Holds the name of a file & the pointer to it
\---------------------------------------------------------------------*/
typedef struct fileStruct
{ // fileStruct
    char fileNameCStr[256]; // Name of the file in the structure
    FILE *outFILE;          // FILE to output to
}fileStruct;

/*---------------------------------------------------------------------\
| Struct-04: webGetPdf: Holds curl handle & the file for it to write to
\---------------------------------------------------------------------*/
typedef struct webGetPdf
{ // webGetPdf
    struct CURL *curlHandle;   // Handle to webpage to get pdf from
    struct fileStruct *fileST; // fileStruct passed to curlHandle
}webGetPdf;

/*---------------------------------------------------------------------\
| Output:
|  - FILE:
|    o With the downloaded pdf (if succeded)
|  - Returns:
|    o 1 for success
|    o 2 if unable to find the link
|    o 4 if unable to access the webpage
|    o 8 if unable to download the pdf
\---------------------------------------------------------------------*/
unsigned char getPMCPdf(
    char *PMCIdCStr,     // PMC id of pdf to grab
    struct webCrawl *webCrawlST,
      // Structer to look for the pdf link with Has curl handle & buffer
      // Make with makeWebCrawlST() fun-03
      // Free with freWebCrawlST()  fun-04
    struct webGetPdf *webGetPdfST
      // Holds the curl handle for getting the pdf & the file/filename
      // to copy the pdf to. make with makeWebGetPdfST() fun-08
      // free with freeWebGetPdfST() fun-09
      // Modify file name with webGetPdfChangeFile() fun-11
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-1 Sub-1: getPMCPdf 
   '  - Driver function to download a pdf from pubmed centeral
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Allows curl to write data in buffer to buffInST->buffCStr
\---------------------------------------------------------------------*/
size_t writeDataToBuff(
    void *buffer,        // buffer with data to copy from curl
    size_t sizeOfElm,    // Size of a single read in element
    size_t numElmInBuff, // Number of elements in the buffer
    void *buffInST       // Buffer in a structer (so I have the size)
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: Sec-1 Sub-1: writeDataToBuff
   '  - Function to write data output from curl
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Returns a webCrawl structure
\---------------------------------------------------------------------*/
struct webCrawl * makeWebCrawlST(
    char *urlCStr // url to copy over for webCrawl to use
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: Sec-1 Sub-1: makeWebCrawlST
   '  - makes a webCrawl structer with a curl handle and a buffer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Frees a webCrawlST structer (you need to set it to 0)
\---------------------------------------------------------------------*/
void freeWebCrawlST(
    char stackBl,               // 1: strucuter on stack; 0 on heap
    struct webCrawl *webCrawlST // Structure to free
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-04 TOC: Sec-1 Sub-1: freeWebCrawlST
   '  - Frees a webCrawlST delared
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output:
|  - Modifies:
|    o webCrawlST->curlHandle to have the new url (sets to "" if no url)
\---------------------------------------------------------------------*/
void setWebCrawlURL(
    char *urlCStr,             // new url
    struct webCrawl *webCrawlST // Structure to change url for
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: Sec-1 Sub-1: setWebCrawlURL
   '  - Sets up changes the url for the webCrawl structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output:
|  - Modifes:
|    o buffCStr to hold the final part of the pdf link
|      x Will be "/articles/pmcid/file.pdf"
|      x Will be missing "www.ncbi.nlm.nih.gov/pmc"
|  - Returns:
|    o 1 for success
|    o 2 if unable to find the link
|    o 4 if unable to access the webpage
\---------------------------------------------------------------------*/
unsigned char findPdfLink(
    char *buffCStr,    // buffer to copy the pdf link to
    struct webCrawl *webCrawlST
       // Holds the curl handle has the link to the PMC page
       // Holds the buffer assigned to the curl handle
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-06 TOC: Sec-1 Sub-1: findPdfLink
   '  - Find the pdf link for a pubmed centeral id (PMCID)
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: -1 if failed, number of written characters if succeded
\---------------------------------------------------------------------*/
static size_t curlWriteToFile(
    void *buffer,   // Buffer from curl
    size_t elmSize, // Size of a single element
    size_t numElm,  // Number of elements to print out
    void *fileST    // fileStrut with the file that curl writes to
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-07 TOC: Sec-1 Sub-1: curlWriteToFile
   '  - writes the output from curl to a file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Returns a webGetPdf structer to get a pdf with (0 if failed)
\---------------------------------------------------------------------*/
struct webGetPdf * makeWebGetPdfST(
    char *urlCStr,      // url to browse to (input 0 to ignore)
    char *fileNameCStr  // File name to copy over (input 0 to ignore)
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-08 TOC: Sec-1 Sub-1: makeWebGetPdfST
   '  - makes a webGetPdf structure on the heap
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Frees input webGetPdf structer (does not set the pointer to 0)
\---------------------------------------------------------------------*/
void freeWebGetPdfST(
    char stackBl,                 // 1: on the stack; 0 on the heap
    struct webGetPdf *webGetPdfST // structure to free
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-09 TOC: Sec-1 Sub-1: freeWebGetPdf
   '  - Frees a webGetPdf structer and its variables
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output:
|  - Modifes:
|    o webGetPdf->fileSt->outFILE to hold the found pdf
|  - Returns:
|    o 1 for success
|    o 8 if unable to download the pdf
\---------------------------------------------------------------------*/
unsigned char downloadPdf(
    struct webGetPdf *webGetPdfST
       // Holds the curl handle & the link to the pdf in PMC page
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-10 TOC: Sec-1 Sub-1: downloadPdf
   '  - Downloads a pdf to the file in webGetPdfST
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output: Changes file name in webGetPdfST & closes old file if open
\---------------------------------------------------------------------*/
void webGetPdfChangeFile(
    char *newFileCStr,
    struct webGetPdf *webGetPdfST
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-11 TOC: Sec-1 Sub-1: webGetChangeFile
   '  - Changes the output file for a webGetPdf structuer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*---------------------------------------------------------------------\
| Output:
|  - Modifies:
|    o webGetPdfST->curlHandle to have the new url (sets to "" if no url)
\---------------------------------------------------------------------*/
void setWebGetPdfURL(
    char *urlCStr,               // new url
    struct webGetPdf *webGetPdfST // Structure to change url for
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-12 TOC: Sec-1 Sub-1: setWebGetPdfURL
   '  - Sets up changes the url for the webGetPdf structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif
