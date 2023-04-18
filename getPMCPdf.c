/*######################################################################
# Name: getPMCPdf.h
# Use:
#  - Holds structures and functions to downloads a pdf from pubmed
#    central (PMC) using curl
# Dependencies:
#  - <string.h>
#  - <stdio.h>
#  - <curl/curl.h>
#  - "cStrFun.h"
######################################################################*/

#include "getPMCPdf.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' SOF: Start Of Functions
'  - fun-01 getPMCPdf:
'    o Driver function to download a pdf from pubmed centeral
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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-1 Sub-1: getPMCPdf 
   '  - Driver function to download a pdf from pubmed centeral
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   char *ncbiAddressCStr = "https://www.ncbi.nlm.nih.gov/pmc";
   char pdfAddressCStr[64];
   char *endOfAddressCStr = 0;
   unsigned char errUC = 0;

   // Blank the buffer
   webCrawlST->buffST->numElmInBuff = 0;

   // Set up the address to search for the pdf link at
   endOfAddressCStr = cStrCpInvsDelm(pdfAddressCStr, ncbiAddressCStr);
   endOfAddressCStr = cStrCpInvsDelm(endOfAddressCStr, "/articles/");
   endOfAddressCStr = cStrCpInvsDelm(endOfAddressCStr, PMCIdCStr);
   endOfAddressCStr = cStrCpInvsDelm(endOfAddressCStr, "/pdf");

   // Find the pdf link (set up url for curl and run curl)
   setWebCrawlURL(pdfAddressCStr, webCrawlST);
   errUC = findPdfLink(endOfAddressCStr, webCrawlST);

   // Set up for adding the full pdf adress
   endOfAddressCStr = cStrCpInvsDelm(pdfAddressCStr, ncbiAddressCStr);
   if(!(errUC & 1)) return errUC; // could not get the pdf link (2 or 4)

   // Download the pdf (set web address and do download)
   setWebGetPdfURL(pdfAddressCStr, webGetPdfST);
   errUC = downloadPdf(webGetPdfST);

   return errUC; // finished (is a 1 or an 8)
} // getPMCPdf

/*---------------------------------------------------------------------\
| Output: Allows curl to write data in buffer to buffInST->buffCStr
\---------------------------------------------------------------------*/
size_t writeDataToBuff(
    void *buffer,        // buffer with data to copy from curl
    size_t sizeOfElm,    // Size of a single read in element
    size_t numElmInBuff, // Number of elements in the buffer
    void *buffInST       // Buffer in a structer (so I have the size)
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: Sec-1 Sub-1: writeDataToBuff
   '  - Function to write data output from curl
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   struct buffStruct *buffST = (struct buffStruct *) buffInST;
   size_t numBytesToCopy = sizeOfElm * numElmInBuff;
   size_t totalBytes = numBytesToCopy + buffST->numElmInBuff;

   if(buffST->lenBuff < totalBytes)
   { // If i need to copy some more bytes over
       buffST->buffCStr = realloc(buffST->buffCStr, totalBytes + 1);

       if(buffST->buffCStr == 0)
       { // If ran out of memory
           fprintf(stderr, "Ran out of memory for curl\n");
           return 0;
       } // If ran out of memory

       buffST->lenBuff = totalBytes;
   } // If i need to copy some more bytes over

   memcpy(
       buffST->buffCStr+buffST->numElmInBuff,
       buffer,
       numBytesToCopy
   ); // Copy input buffer and update the number of bytes in the buffer

   buffST->numElmInBuff = totalBytes;

   // Make sure is a c-string
   *(buffST->buffCStr + buffST->numElmInBuff) = '\0';
   return totalBytes;
} // writeDataToBuff

/*---------------------------------------------------------------------\
| Output: Returns a webCrawl structure
\---------------------------------------------------------------------*/
struct webCrawl * makeWebCrawlST(
    char *urlCStr // url to copy over for webCrawl to use
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: Sec-1 Sub-1: makeWebCrawlST
   '  - makes a webCrawl structer with a curl handle and a buffer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   struct webCrawl *webCrawlST = malloc(sizeof(webCrawl));

   if(webCrawlST == 0)
       return 0;

   webCrawlST->buffST = malloc(sizeof(struct buffStruct));

   if(webCrawlST->buffST == 0)
   { // If I ran out of memory for making the buffer
       free(webCrawlST);
       return 0;
   } // If I ran out of memory for making the buffer

   webCrawlST->buffST->buffCStr = malloc(sizeof(char));

   if(webCrawlST->buffST->buffCStr == 0)
   { // If I could not even make a buffer
       free(webCrawlST->buffST);
       free(webCrawlST);
   } // If I could not even make a buffer

   webCrawlST->curlHandle = curl_easy_init();

   if(webCrawlST->curlHandle == 0)
   { // If no curl handle was made (memory)
       free(webCrawlST->buffST->buffCStr);
       free(webCrawlST->buffST);
       free(webCrawlST);
       return 0;
   } // If no curl handle was made (memory)

   // Set up the other paramaters in the web crawl structure
   webCrawlST->buffST->lenBuff = 0;
   webCrawlST->buffST->numElmInBuff = 0;
   setWebCrawlURL(urlCStr, webCrawlST);

   // Set the curl paramaters
   curl_easy_setopt(
       webCrawlST->curlHandle,
       CURLOPT_WRITEFUNCTION,
       writeDataToBuff
   ); // Set up the function to add data to the buffer

   curl_easy_setopt(
       webCrawlST->curlHandle,
       CURLOPT_WRITEDATA,
       webCrawlST->buffST
   ); // Set up the buffer for curl to read to

   // Make data unique to this handle
   curl_easy_setopt(webCrawlST->curlHandle, CURLOPT_ACCEPT_ENCODING, "");

   curl_easy_setopt(
       webCrawlST->curlHandle,
       CURLOPT_USERAGENT,
       "PMCDownload"
   ); // Let the website know what is accessing it

   curl_easy_setopt(
       webCrawlST->curlHandle,
       CURLOPT_HTTPAUTH,
       CURLAUTH_ANY
   ); // Use an authentication method

   // Allow redirections
   curl_easy_setopt(webCrawlST->curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
   curl_easy_setopt(webCrawlST->curlHandle, CURLOPT_MAXREDIRS, 10L);
      // Allow a maximum of 10 redirects

   return webCrawlST;
} // makeWebCrawlST

/*---------------------------------------------------------------------\
| Output: Frees a webCrawlST structer (you need to set it to 0)
\---------------------------------------------------------------------*/
void freeWebCrawlST(
    char stackBl,               // 1: strucuter on stack; 0 on heap
    struct webCrawl *webCrawlST // Structure to free
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-04 TOC: Sec-1 Sub-1: freeWebCrawlST
   '  - Frees a webCrawlST delared
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(webCrawlST == 0) return;

   if(webCrawlST->buffST != 0)
   { // If have a buffer to free
       if(webCrawlST->buffST->buffCStr != 0)
           free(webCrawlST->buffST->buffCStr);

       free(webCrawlST->buffST);
       webCrawlST->buffST = 0;
   } // If have a buffer to free

   if(webCrawlST->curlHandle != 0)
   { // If I need to free the culr handle
       curl_easy_cleanup(webCrawlST->curlHandle);
       webCrawlST->curlHandle = 0;
   } // If I need to free the culr handle

   if(!(stackBl & 1))
       free(webCrawlST);  // Need to free the input structure

   return;
} // freeWebCrawlST

/*---------------------------------------------------------------------\
| Output:
|  - Modifies:
|    o webCrawlST->curlHandle to have the new url (sets to "" if no url)
\---------------------------------------------------------------------*/
void setWebCrawlURL(
    char *urlCStr,             // new url
    struct webCrawl *webCrawlST // Structure to change url for
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: Sec-1 Sub-1: setWebCrawlURL
   '  - Sets up changes the url for the webCrawl structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(urlCStr != 0)
       curl_easy_setopt(
           webCrawlST->curlHandle,
           CURLOPT_URL,
           urlCStr
       );

   else
       curl_easy_setopt(webCrawlST->curlHandle, CURLOPT_URL, "");

   return;
} // setWebCrawlURL

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-06 TOC: Sec-1 Sub-1: findPdfLink
   '  - Find the pdf link for a pubmed centeral id (PMCID)
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   char *cpCStr;
   CURLcode curlResult = curl_easy_perform(webCrawlST->curlHandle);

   *buffCStr = '\0'; // Make sure null terminated

   if(curlResult != CURLE_OK) return 4;
       // NCBI blocks me due to detecting curl and blocking it

   // Hunt for start of link
   cpCStr =
       strstr(webCrawlST->buffST->buffCStr, "<meta name=\"ncbi_pcid\"");

   if(cpCStr == 0) return 2;    // failed to find a pdf link tag
   
   cpCStr = strstr(cpCStr, "/articles");

   if(cpCStr == 0) return 2;    // No link

   while(*cpCStr != '"')
   { // While I have a link to copy over
       *buffCStr = *cpCStr;
       ++buffCStr;
       ++cpCStr;
   } // While I have a link to copy over

   *buffCStr = '\0'; // Make a c-string
   return 1; // Success
} // findPdfLink

/*---------------------------------------------------------------------\
| Output: -1 if failed, number of written characters if succeded
\---------------------------------------------------------------------*/
static size_t curlWriteToFile(
    void *buffer,   // Buffer from curl
    size_t elmSize, // Size of a single element
    size_t numElm,  // Number of elements to print out
    void *fileST    // fileStrut with the file that curl writes to
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-07 TOC: Sec-1 Sub-1: curlWriteToFile
   '  - writes the output from curl to a file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   struct fileStruct *tmpFileST = (struct fileStruct *) fileST;

   // Check if I need ot make a new file
   if(tmpFileST->outFILE == 0)
   { // If I need to make a new file
       tmpFileST->outFILE = fopen(tmpFileST->fileNameCStr, "w");
       if(tmpFileST->outFILE == 0) return -1;  // unable to open file
   } // If I need to make a new file

   // Write the buffer to the file
   return fwrite(buffer, elmSize, numElm, tmpFileST->outFILE);
} // curlWriteToFile

/*---------------------------------------------------------------------\
| Output: Returns a webGetPdf structer to get a pdf with (0 if failed)
\---------------------------------------------------------------------*/
struct webGetPdf * makeWebGetPdfST(
    char *urlCStr,      // url to browse to (input 0 to ignore)
    char *fileNameCStr  // File name to copy over (input 0 to ignore)
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-08 TOC: Sec-1 Sub-1: makeWebGetPdfST
   '  - makes a webGetPdf structure on the heap
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   struct webGetPdf *webGetPdfST = malloc(sizeof(struct webGetPdf));

   if(webGetPdfST == 0) return 0; // Failed to make the structer

   webGetPdfST->fileST = malloc(sizeof(fileStruct));

   if(webGetPdfST->fileST == 0)
   { // If I failed to make a file structuer
       free(webGetPdfST);
       return 0; // Ran out of memory
   } // If I failed to make a file structuer

   webGetPdfST->curlHandle = curl_easy_init();

   if(webGetPdfST->curlHandle == 0)
   { // If I failed to make a handle for curl
       free(webGetPdfST->fileST);
       free(webGetPdfST);
       return 0;
   } // If I failed to make a handle for curl

   // Copy over the file name and initialize the file
   setWebGetPdfURL(urlCStr, webGetPdfST);
   webGetPdfChangeFile(fileNameCStr, webGetPdfST);
   webGetPdfST->fileST->outFILE = 0; // So curl opens the file up

    // Set up curl commands
    curl_easy_setopt(
        webGetPdfST->curlHandle,
        CURLOPT_WRITEFUNCTION,
        curlWriteToFile
    ); // Set up the function to add data to a file

    curl_easy_setopt(
        webGetPdfST->curlHandle,
        CURLOPT_WRITEDATA,
        webGetPdfST->fileST
    ); // Set up the buffer for curl to read to

    curl_easy_setopt(
        webGetPdfST->curlHandle,
        CURLOPT_ACCEPT_ENCODING,
        ""
    ); // Not really sure what does, but was in the example

    curl_easy_setopt(
        webGetPdfST->curlHandle,
        CURLOPT_USERAGENT,
        "PMCDownload"
    ); // Let the website know what is accessing it

    curl_easy_setopt(
        webGetPdfST->curlHandle,
        CURLOPT_HTTPAUTH,
        CURLAUTH_ANY
    ); // Use an authentication method

    // Allow redirections
    curl_easy_setopt(webGetPdfST->curlHandle, CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(webGetPdfST->curlHandle, CURLOPT_MAXREDIRS, 10L);
       // Allow a maximum of 10 redirects

    return webGetPdfST;
} // makeWebGetPdfST

/*---------------------------------------------------------------------\
| Output: Frees input webGetPdf structer (does not set the pointer to 0)
\---------------------------------------------------------------------*/
void freeWebGetPdfST(
    char stackBl,                 // 1: on the stack; 0 on the heap
    struct webGetPdf *webGetPdfST // structure to free
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-09 TOC: Sec-1 Sub-1: freeWebGetPdf
   '  - Frees a webGetPdf structer and its variables
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(webGetPdfST == 0) return;

   if(webGetPdfST->fileST != 0)
   { // If have a file structure to free
       if(webGetPdfST->fileST->outFILE != 0)
       { // If I need to close the output file
           fclose(webGetPdfST->fileST->outFILE);
           webGetPdfST->fileST->outFILE = 0;
       } // If I need to close the output file

       free(webGetPdfST->fileST);
       webGetPdfST->fileST = 0;
   } // If have a file structure to free

   if(webGetPdfST->curlHandle != 0)
   { // If I need to free the handle give to curl
       curl_easy_cleanup(webGetPdfST->curlHandle);
       webGetPdfST->curlHandle = 0;
   } // If I need to free the handle give to curl

   // Check if I need to free the webBetPdf structer
   if(!(stackBl & 1)) free(webGetPdfST);

   return; // done
} // freeWebGetPdf

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-10 TOC: Sec-1 Sub-1: downloadPdf
   '  - Downloads a pdf to the file in webGetPdfST
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   CURLcode curlResult = curl_easy_perform(webGetPdfST->curlHandle);
   if(curlResult != CURLE_OK) return 8; // failed to acces web page
   return 1; // Success
} // findPdfLink

/*---------------------------------------------------------------------\
| Output: Changes file name in webGetPdfST & closes old file if open
\---------------------------------------------------------------------*/
void webGetPdfChangeFile(
    char *newFileCStr,
    struct webGetPdf *webGetPdfST
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-11 TOC: Sec-1 Sub-1: webGetChangeFile
   '  - Changes the output file for a webGetPdf structuer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   char *dupCStr = webGetPdfST->fileST->fileNameCStr;

   if(webGetPdfST->fileST->outFILE != 0)
   { // If I need to close the old file
       fclose(webGetPdfST->fileST->outFILE);
       webGetPdfST->fileST->outFILE = 0;
   } // If I need to close the old file

   if(newFileCStr == 0)
   { // If is just a blank entry
       *dupCStr = '\0';
       return;
   } // If is just a blank entry
   
   while(*newFileCStr != '\0')
   { // While have a file name to copy over
       *dupCStr = *newFileCStr;
       ++dupCStr;
       ++newFileCStr;
   } // While have a file name to copy over

   *dupCStr = '\0';
   return;
} // webGetPdfChangeFile

/*---------------------------------------------------------------------\
| Output:
|  - Modifies:
|    o webGetPdfST->curlHandle to have the new url (sets to "" if no url)
\---------------------------------------------------------------------*/
void setWebGetPdfURL(
    char *urlCStr,               // new url
    struct webGetPdf *webGetPdfST // Structure to change url for
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-12 TOC: Sec-1 Sub-1: setWebGetPdfURL
   '  - Sets up changes the url for the webGetPdf structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(urlCStr != 0)
       curl_easy_setopt(
           webGetPdfST->curlHandle,
           CURLOPT_URL,
           urlCStr
       );

   else
       curl_easy_setopt(webGetPdfST->curlHandle, CURLOPT_URL, "");

   return;
} // setWebGetPdfURL
