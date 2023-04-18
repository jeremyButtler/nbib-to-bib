/*#########################################################
# Name: getPMCPdf.h
# Use:
#  - Holds structures and functions to downloads a pdf from
#    pubmed central (PMC) using curl
# Dependencies:
#  - "getPMCPdf.h"
#  o "cnvtPubmedToBib.h"
#  o "cStrFun.h"
#  o <stdlib.h>
#  o <string.h>
#  o <stdio.h>
#  o <unac.h>
#  o <curl/curl.h>
# Note to self:
#   http://graphics.stanford.edu/~seander/bithacks.html
#   Has log2 by bit shift and other optiontions
#########################################################*/

// Allow user to turn off
#ifdef PDF
    #include "getPMCPdf.h"
#endif

#include "cnvtPubmedToBib.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' SOP: Start Of Program
'  - main:
'    o Driver for converting .pubmed to .bib
'  - fun-01 getUserInput:
'    o Gets the user input from the command line
'  - fun-02 cStrToUS:
'    o converst a c-string into unsigned short
'  - fun-03 tenToX:
'    o Return a power of 10
'  - Fun-04 setBibPVal:
'    o Sets weather to print a bibtext entry or not
'  - Fun-05 pBibEntryHelp:
'    o Prints out the help message for selecting which
'      bibtex entries to print
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o All input,except argsIn &numArgsI to hold user input
|  - Returns:
|    o 0 if succeded
|    o Pointer to the proplematic paramter if failed
\--------------------------------------------------------*/
char * getUserInput(
   char *argsIn[],      // Arguments input by the user
   int numArgsI,        // Number of args input by the user
   char **pubFileCStr,  // Name of the .pubmed file to read
   char **bibFileCStr,  // Name of the .bib file to output
   #ifdef PDF         // NCBI blocks this so disabled
       char *getPdfBl,   // get pdf if PMC is present
   #endif
   struct pubmedStruct *pubS, // Holds line wraping & tags
   struct pubOut *pubOutST 
    // Has bolleanas that tell which bibtex entries to
    // print out
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-01 Sub-01: getUserInput
   '  - Gets the user input from the command line
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Returns: pointer to the last unconverted character
|  - Modifies: retUS to hold an unsigned short
| Note:
|  - This function assumes an unsigned short is 16 bits
\--------------------------------------------------------*/
char * cStrToUS(
    char *inCStr,          //C-string to convert to number
    unsigned short *retUS  /*Holds converted number*/
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: Sec-01 Sub-01: cStrToUS
   '  - converst a c-string into an unsigned short
   '  o fun-02 sec-01: Convert the first digit
   '  o fun-02 sec-02: Convert digits with no overlfow risk
   '  o fun-02 sec-03: Convert last 2 digits (can overflow)
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Returns power of ten
\--------------------------------------------------------*/
unsigned long tenToX(
    char powTenUC // Power to raise 10 to
);/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-03 TOC: Sec-01 Sub-01: tenToX
  '  - Return a power of 10
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifes: boolean value in pubOutST
|  - Returns;
|    o 0 for success
|    o 2 for invalid input
\--------------------------------------------------------*/
unsigned char setBibPVal(
  char *parmCStr,         // Parameter input by user
  struct pubOut *pubOutST // Has print values to set
);/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-04 TOC: Sec-01 Sub-01: setBibPVal
  '  - Sets weather to print a bibtext entry or not
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: prints out bibtex entry help message to stdout
\--------------------------------------------------------*/
void pBibEntyHelp(
);/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-05 TOC: Sub-01 Sec-01: pBibEntryHelp
  '  - Prints out the help message for selecting which
  '    bibtex entries to print
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Prints bibtex (.bib) file to stdout or input file
\--------------------------------------------------------*/
int main(
    int numArgsI,
    char *argsIn[]
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Main TOC:
   '  - Converts pubmed format to bibtex
   '  o main sec-01: Variable declerations
   '  o Main sec-02: Get user input
   '  o main sec-03: Check user file input
   '  o main sec-04: Initialize curl variables (if needed)
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Main Sec-01: Variable declerations
   ^  o main sec-01 sub-01: non-help message variables
   ^  o main sec-01 sub-02: help message
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   /******************************************************\
   * Main Sec-01 Sub-01: non-help message variables
   \******************************************************/

   // usr input
   char *pubFileCStr = 0; // .pubmed file to convert
   char *bibFileCStr = 0; // .bib file to output
   
   char *tmpCStr = 0;
   unsigned long numPdfUL = 0;
   unsigned char pubErrUC = 0;  

   #ifdef PDF
       char getPdfBl = defGetPdf;

       struct webCrawl *webCrawlST = 0; 
       struct webGetPdf *webGetPdfST = 0;
       unsigned char pdfErrUC = 0;
   #endif

   struct pubmedStruct pubST;
   struct pubOut pubOutST;

   FILE *pubFILE = 0; // input .pubmed file
   FILE *bibFILE = 0; // output bibtex (.bib) file

   /******************************************************\
   * Main Sec-01 Sub-02: help message
   \******************************************************/

   char *helpCStr = "\
   \n Use: Converts a .pubmed file to a bibtex (.bib) file\
   \n Run: pubmedToBib -pubmed file.pubmed [options...]\
   \n Input:\
   \n   -pubmed file.pubmed: [Required]\
   \n     o .pubmed file to convert to bibtex (.bib) file.\
   \n     o Make sure each citation (entry) in the file is\
   \n       separated by a blank line.\
   \n   -bib file.bib: [stdout]\
   \n     o Name of the output bibtex (.bib) file.\
   \n   -line-wrap 59: [59]\
   \n     o Number of characters to have before breaking a\
   \n       a line (32 to 60000 characters).\
   \n     o -line-wrap 0: to turn of line wrapping.\
   \n     o This only applies to the title, abstract,\
   \n       authors, keywords, mesh terms, and tags.\
   \n   -tag \"something\":  [No tags]\
   \n     o Tags to add to the output .bib file for each\
   \n       .pubmed entry.\
   \n     o You can specify this option multiple times.\
   \n     o Make sure to put call -line-wrap before -tag.\
   \n       Otherwise default line wrapping will be used.\
   \n   -break-unix:[Default]\
   \n     o Uses a \'\\n\' mark new lines.\
   \n   -break-win:[No]\
   \n     o Uses just a \"\\r\\n\" mark new lines.\
   \n     o Used to be how windows treated line breaks,\
   \n       but may not apply any more.\
   \n   -bib-help:\
   \n     o Prints out all options for controlling which\
   \n       bibtex entries are print out.\
   \n Output:\
   \n  o Prints a bibtex entry to stdout for each entry in\
   \n    the .pubmed file.\
   \n Requires:\
   \n  o Unac (https://github.com/QuickDict/unac)\
   \n    - unac is used to remove accents from names. This\
   \n      prevents errors that might occur when programs\
   \n      the bibtex file.\
   \n    - This Can be disabled at compile time with\
   \n      \"make noaccent\" (CFLAGS=\"-DNORMACCENT\"),\
   \n      but will disable using the authors name in the\
   \n      citation key or file name.\
   ";

    /* This was here until I found out that NCBI blocks
       curl. The code should work in theory, but may not
       work in real life
    \n   -get-pmc-pdf: [No]\
    \n     o Download the pdf for any file that has a\
    \n       pubmed centeral (PMC) id.\
    \n     o This can be enabled by \"make pdf"\
    \n       (CFLAGS="-DPDF") \
    \n  o Curl (https://github.com/curl/curl)\
    */

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Main Sec-02: Get user input
   ^  o main sec-02 sub-01: Get user input
   ^  o main sec-02 sub-02: Check if printing help message
   ^  o main sec-02 sub-03: printing version number?
   ^  o main sec-02 sub-04: Error from -line-wrap input?
   ^  o main sec-02 sub-05: Error is from unknown parameter
   ^  o main sec-02 sub-06: Is line wrap is large enough?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   /*****************************************************\
   * Main Sec-02 Sub-01: Get user input
   \*****************************************************/
   
   // Set up the default values (1,1 for line wrap & tags)
   blankPubmedST(1, 1, 1, 1, &pubST);
   blankPubOutST(&pubOutST);

   tmpCStr = 
     getUserInput(
       argsIn,
       numArgsI,
       &pubFileCStr,
       &bibFileCStr,
       #ifdef PDF
           &getPdfBl, // Only here if grabbing the pdf
       #endif
       &pubST,
       &pubOutST
   );

   /******************************************************\
   * Main Sec-02 Sub-02: Check if printing help message
   \******************************************************/

   if(tmpCStr != 0)
   { // If their was invalid input
       if(strcmp(tmpCStr, "-h") == 0 ||
          strcmp(tmpCStr, "--h") == 0 ||
          strcmp(tmpCStr, "-help") == 0 ||
          strcmp(tmpCStr, "--help") == 0
       ) { // If the user wanted the help message
           printf("%s\n", helpCStr);
           exit(0);
       } // If the user wanted the help message

       if(strcmp(tmpCStr, "-bib-help") == 0)
       { // If user wanted the bibtext help message
         pBibEntyHelp();
         exit(0);
       } // If user wanted the bibtext help message

       /**************************************************\
       * Main Sec-02 Sub-03: Printing version number?
       \**************************************************/

       if(strcmp(tmpCStr, "-v") == 0 ||
          strcmp(tmpCStr, "-V") == 0 ||
          strcmp(tmpCStr, "--v") == 0 ||
          strcmp(tmpCStr, "--V") == 0 ||
          strcmp(tmpCStr, "-version") == 0 ||
          strcmp(tmpCStr, "--version") == 0
       ) { // If the user wanted the version number (date)
           printf("pubmedToBib version: %d\n", defVersion);
           exit(0);
       } // If the user wanted the version number (date)

       /**************************************************\
       * Main Sec-02 Sub-04: Error from -line-wrap input?
       \**************************************************/

       if(strcmp(tmpCStr, "-line-wrap") == 0)
       { // If user input an invalid value for line wraping 
           for(int iArg = 1; iArg < numArgsI; ++iArg)
           { // Loop till on the invalid input
               tmpCStr = *(argsIn + iArg);

               if(strcmp(tmpCStr,"-line-wrap") == 0)
               { // If found the line-wrap input
                   tmpCStr = *(argsIn + iArg + 1);
                   break;
               } // If found the line-wrap input
           } // Loop till on the invalid input

           pubFileCStr = cStrToUS(tmpCStr, &pubST.wrapUS);

           if(*pubFileCStr > 47 && *pubFileCStr < 58)
           { // If the number was to large
               fprintf(
                   stderr,
                   "-line-wrap %s is > 65536\n",
                   tmpCStr
               );

               exit(-1);
           } // If the number was to large

           fprintf(
               stderr,
               "-line-wrap (%s) has Non-numeric value\n",
               tmpCStr
           );

           exit(-1);
       } // If user input an invalid value for line wraping 

       /**************************************************\
       * Main Sec-02 Sub-05: Error is unknown parameter
       \**************************************************/

       fprintf(
           stderr,
           "%s\nInvalid paramter (%s) input\n",
           helpCStr,
           tmpCStr
       ); // Let user know about the error

       exit(-1);
   } // If their was invalid input

   /******************************************************\
   * Main Sec-02 Sub-06: Is line wrap is large enough?
   \******************************************************/

   if(pubST.wrapUS > 0 && pubST.wrapUS < 32)
   { // If line wrap setting is to low
       fprintf(
           stderr,
           "-line-wrap %u, is smaller than 32\n",
           pubST.wrapUS
       ); // Let user know the error

       exit(-1);
   } // If line wrap setting is to low

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Main Sec-03: Check user file input
   ^  o main sec-03 sub-01: Can I open input .pubmed file?
   ^  o main sec-03 sub-02: Can I open the output file?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   /******************************************************\
   * Main Sec-03 Sub-01: Can I open input .pubmed file?
   \******************************************************/

   if(pubFileCStr == 0)
   { // If the user id not input a fil to process
       fprintf(stderr, "No file input with -pubmed\n");
       exit(-1);
   } // If the user id not input a fil to process

   pubFILE = fopen(pubFileCStr, "r");

   if(pubFILE == 0)
   { // If I could not open the input file
       fprintf(
           stderr,
           "Could not open -pubmed file (%s)\n",
           pubFileCStr
       ); // Let user know about the failure

       exit(-1);
   } // If I could not open the input file

   /******************************************************\
   * Main Sec-03 Sub-02: Can I open the output file?
   \******************************************************/
    
   if(bibFileCStr == 0) bibFILE = stdout;

   else bibFILE = fopen(bibFileCStr, "a");

   if(bibFILE == 0)
   { // If I could not open the output file
       fclose(pubFILE);
       fprintf(
           stderr,
           "Could not open -bib file (%s)\n",
           bibFileCStr
       ); // Let the user know about the failure

       exit(-1);
   } // If I could not open the output file

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Main Sec-04: Initialize curl variables (if needed)
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   #ifdef PDF
     if(getPdfBl != 0)
     { // If using curl
       // intailize curl
       curl_global_init(CURL_GLOBAL_DEFAULT);
       webCrawlST = makeWebCrawlST(""); 
       webGetPdfST = makeWebGetPdfST(0, 0);

       if(webCrawlST == 0)
       { // If i could not build the webCrawl structure
         fprintf(
           stderr,
           "Error in get pdf setup (main: struct-1)\n"
         ); // Let user know why stopped

         fclose(pubFILE);
         fclose(bibFILE);
         exit(-1);
       } // If i could not build the webCrawl structure

       if(webGetPdfST == 0)
       { // If I could not build the webGetPdf structure
         fprintf(
             stderr,
             "Error in get pdf setup (main: struct-2)\n"
         ); // Let user know why stoped

         fclose(pubFILE);
         fclose(bibFILE);
         freeWebCrawlST(0, webCrawlST);
         exit(-1);
       } // If I could not build the webGetPdf structure
     } // If using curl
   #endif

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Main Sec-04: Process pubmed entries and download pdfs
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   finshPubmedSTTag(&pubST); // Clean up the tag entry

   pubErrUC = 1;  // So the loop fires

   // FILE CHECKS AND USER INPUT HERE
   while(pubErrUC & 1)
   { // While I have .pubmed entries to extract
       // Extract data from the bpumed file
       pubErrUC = processPubMed(pubFILE, &pubST);

       if(pubErrUC & 2)
       { // If was a blank line, see if next is entry
         pubErrUC = 1;
         continue;
       } // If was a blank line, see if next is entry

       if(pubST.titleCStr[0] == '\0')
           continue; // No title, likely blank

       if(pubErrUC > 1) break; // error

       ++numPdfUL;
       pubmedSTToBib(&pubST, &pubOutST, bibFILE);

       // Add a blank space if there will be another
       // entry
       if(pubErrUC & 1)
         fprintf(bibFILE, "%s", pubST.lineBreakCStr);

       #ifdef PDF  // If using curl support at compile
         if(getPdfBl == 0) continue; // Not extracting pdf
         if(pubST.pmcidCStr[0] == '\0') continue;
           // no pmc to extract pdf with

         // Set up for grabbing the pdf
         webGetPdfChangeFile(pubST.fileCStr, webGetPdfST);
              
         pdfErrUC =
           getPMCPdf(
             pubST.pmcidCStr,
             webCrawlST,
             webGetPdfST
         ); // Get the pdf

         if(!(pdfErrUC & 1)) break; // Error
       #endif
   } // While I have .pubmed entries to extract

   // Only use if user wanted to complie with curl support
   #ifdef PDF
       if(getPdfBl == 0)
       { // If I used curl, then I need to do clean up
           freeWebCrawlST(0, webCrawlST);
           freeWebGetPdfST(0, webGetPdfST);
           webCrawlST = 0;
           curl_global_cleanup();
       } // If I used curl, then I need to do clean up
   #endif

   if(pubFILE != 0) fclose(pubFILE);
   if(bibFILE != 0) fclose(bibFILE);

   if(pubErrUC & 4)
   { // If had an invalid pubmed file
       fprintf(
           stderr,
           "-pubmed %s is an invalid file\n",
           pubFileCStr
       );

       exit(-1);
   } // If had an invalid pubmed file

   if(pubErrUC & 8)
   { // If failed to make the citation key
       fprintf(
           stderr,
           "Failed to make the citation key for\n"
       );

       fprintf(
           stderr,
           " entry number: %lu (PMID %s / PMC %s)\n",
           numPdfUL,
           pubST.pmidCStr,
           pubST.pmcidCStr
       );

       exit(-1);
   } // If failed to make the citation key

   
   #ifdef PDF
       if(getPdfBl != 0 && !(pdfErrUC & 1))
       { // If had a problem downloading the pdf file
           fprintf(stderr, "Failed to get pdf\n");
           fprintf(stderr,"Problem was with entry number");
           fprintf(
               stderr,
               " %lu (PMID %s / PMC %s)\n",
               numPdfUL,
               pubST.pmidCStr,
               pubST.pmcidCStr
           );

           exit(-1);
       } // If had a problem downloading the pdf file
   #endif

   exit(0);
} // main

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o All input,except argsIn &numArgsI to hold user input
|  - Returns:
|    o 0 if succeded
|    o Pointer to the proplematic paramter if failed
\--------------------------------------------------------*/
char * getUserInput(
   char *argsIn[],      // Arguments input by the user
   int numArgsI,        // Number of args input by the user
   char **pubFileCStr,  // Name of the .pubmed file to read
   char **bibFileCStr,  // Name of the .bib file to output
   #ifdef PDF         // NCBI blocks this so disabled
       char *getPdfBl,  // get pdf if PMC is present
   #endif
   struct pubmedStruct *pubST,// Holds line wraping & tags
   struct pubOut *pubOutST 
    // Has bolleanas that tell which bibtex entries to
    // print out
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-01 Sub-01: getUserInput
   '  - Gets the user input from the command line
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   char *inputCStr = 0;
   char *parameterCStr = 0;

   for(int iArg = 1; iArg < numArgsI; ++iArg)
   { // For all user input arguments 
       parameterCStr = *(argsIn + iArg);
       inputCStr = *(argsIn + iArg + 1);

       if(strcmp(parameterCStr, "-pubmed") == 0)
       { // If the user input a pubmed file
            *pubFileCStr = inputCStr;
            ++iArg;
            continue;   
       } // If the user input a pubmed file

       if(strcmp(parameterCStr, "-bib") == 0)
       { // If user provided a name for output bibtex file
           *bibFileCStr = inputCStr;
           ++iArg;
           continue;
       } // If user provided a name for output bibtex file

       if(strcmp(parameterCStr, "-line-wrap") == 0)
       { // If the user wanted to set the line wrapping
           inputCStr = cStrToUS(inputCStr, &pubST->wrapUS);

           if(*inputCStr > 32) return parameterCStr;

           ++iArg;
           continue;
       } // If the user wanted to set the line wrapping

       #ifdef PDF
           if(strcmp(parameterCStr, "-get-pmc-pdf") == 0)
           { // If user wanted to download pmc pdfs
               *getPdfBl = !defGetPdf;
               continue;
           } // If user wanted to download pmc pdfs
       #endif

       if(strcmp(parameterCStr, "-tag") == 0)
       { // If the user wanted to supply a tag
           pubmedSTAddTag(
               inputCStr,
               strlen(inputCStr),
               pubST
           ); // Add tags for all pubmed entries

           ++iArg;
           continue;
       } // If the user wanted to supply a tag

       if(strcmp(parameterCStr, "-break-unix") == 0)
       { // If the user wanted the linux line break
           pubmedSTSetBreak("\n", pubST);
           continue;
       } // If the user wanted the linux line break

       if(strcmp(parameterCStr, "-break-win") == 0)
       { // If user wanted a windows line break
           pubmedSTSetBreak("\r\n", pubST);
           continue;
       } // If user wanted a windows line break

       if(setBibPVal(parameterCStr, pubOutST) == 0)
         continue; // Already set the correct value

       return parameterCStr; // Invalid input
   } // For all user input arguments 

   return 0;
} // getUserInput

/*--------------------------------------------------------\
| Output:
|  - Returns: pointer to the last unconverted character
|  - Modifies: retUS to hold an unsigned short
| Note:
|  - This function assumes an unsigned short is 16 bits
\--------------------------------------------------------*/
char * cStrToUS(
    char *inCStr,          // C-string to convert to number
    unsigned short *retUS  // Holds converted number
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-02 TOC: Sec-01 Sub-01: cStrToUS
  '  - converst a c-string into an unsigned short
  '  o fun-02 sec-01: Convert the first digit
  '  o fun-02 sec-02: Convert digits with no overlfow risk
  '  o fun-02 sec-03: Convert last 2 digits (can overflow)
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  unsigned short firstDigUS = *inCStr - 48;
  unsigned short highestPowTenUS = 
    tenToX((sizeof(unsigned short) << 3) / 3.333333333);
  /* Logic
     - sizeof(unsigned short) << 3 gives me the number of
       bits in an unsigned short
     - (bits in unsinged short) / 3.333333333 gives me the
       closest power of 10 to the maximum short value.
     - tenToX does a while loop to find the power of 10
   - This step should be complied out under O2 or O3 and
     makes the code handle any size of unsigned short.
  */

  unsigned short maxUSDig =
      ((1 << (sizeof(unsigned short) << 3)) - 1)
    / highestPowTenUS;
  /*Logic:
     - sizeof(unsigned short) << 3 gives me the number of
       bits in an unsigned short
     - (1 << bits in unsigned short) gives me a mutiple of
       two higher that is maximum short valute + 1.
     - (highest multiple) - 1 gives me the maximum value
       for a short.
     - (maximum unsinged short / closest power 10) gives
        me the digit in the highest postion.
   - This step should be complied out under O2 or O3 and
     makes the code handle any size of unsigned short.
  */

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-02 Sec-01 Sub-01: Convert the first digit
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  /*Convert first number*/
  if(firstDigUS >= 0 && firstDigUS < 9)   /*1st digit*/
    *retUS = firstDigUS;
  else
  { /*Else non-numeric*/
    *retUS = 0;
    return inCStr;
  } /*Else non-numeric*/

  ++inCStr;

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-02 Sec-02 Sub-01: Digits with no overlfow risk
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  //  o ((sizeof(unsignedshort) << 3) / 3.333333) - 3 is
  //     here to give the number of safe bits to convert.
  //  o sizeof(unsigned short) << 3 gives me the number 
  //     of bits in an unsigned short
  //  o /3.3333 converts bits to number of base 10 digits
  //  o - 2 accounts for the first bit converted and the
  //    last bit at end (has a risk of overflow)
  for(
    int iDig = 0;
    iDig < ((sizeof(unsigned short) << 3) / 3.333333) - 2;
    ++iDig
  ) { // Loop through all safe digits to convert
    if(*inCStr < 58 && *inCStr > 47)
      *retUS = *retUS * 10 + *inCStr - 48;

    else return inCStr;

    ++inCStr;
  } // Loop through all safe digits to convert

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-02 Sec-03 Sub-01: Convert last digits (overflow)
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  if(*inCStr > 47)
  { /*If have one more number*/
    // Check if there is a 0 there will be no change
    if(*inCStr == '0') return inCStr + 1;

    // Check if number will overflow by a large amount
    if(firstDigUS > maxUSDig) return inCStr;

    if(*inCStr < 58)
    { // If this is a number greater than 0
      firstDigUS = 10 * *retUS + *inCStr - 48;

      /* Check if had an overflow
          - The idea is that an overflow will result
            in the number being shorter than the nearest
            highest power of 10, while any non overflow
            will be at least as high as the nearest power
            of 10. This is ensured by discarding any values
            that have the highest power digit higher than
            the highest power unsigned short digit.
      */
      if(firstDigUS < highestPowTenUS)
          return inCStr;

      *retUS = firstDigUS;
      ++inCStr;
    } // If this is a number greater than 0
  } /*If have one more number*/

  return inCStr;
} /*cStrToUS*/

/*--------------------------------------------------------\
| Output: Returns power of ten
\--------------------------------------------------------*/
unsigned long tenToX(
    char powTenUC // Power to raise 10 to
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-03 TOC: Sec-01 Sub-01: tenToX
  '  - Return a power of 10
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  unsigned long tenUL = 1;

  while(powTenUC > 0)
  { // While have a power of 10 to find
      tenUL *= 10;
      --powTenUC;
  } // While have a power of 10 to find

  return tenUL;
} // tenToX

/*--------------------------------------------------------\
| Output:
|  - Modifes: boolean value in pubOutST
|  - Returns;
|    o 0 for success
|    o 2 for invalid input
\--------------------------------------------------------*/
unsigned char setBibPVal(
  char *parmCStr,         // Parameter input by user
  struct pubOut *pubOutST // Has print values to set
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-04 TOC: Sec-01 Sub-01: setBibPVal
  '  - Sets weather to print a bibtext entry or not
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if(*(parmCStr + 1) == 'p')
  { // If setting a entry to be printed out
    if(strcmp(parmCStr, "-p-citekey") == 0)
      pubOutST->citeKeyBl = 1;

    else if(strcmp(parmCStr, "-p-journal-id") == 0)
      pubOutST->journalIDBl = 1;

    else if(strcmp(parmCStr, "-p-month") == 0)
      pubOutST->monthBl = 1;

    else if(strcmp(parmCStr, "-p-day") == 0)
      pubOutST->dayBl = 1;

    else if(strcmp(parmCStr, "-p-volume") == 0)
      pubOutST->volBl = 1;

    else if(strcmp(parmCStr, "-p-issue") == 0)
      pubOutST->issueBl = 1;

    else if(strcmp(parmCStr, "-p-doi") == 0)
      pubOutST->doiBl = 1;

    else if(strcmp(parmCStr, "-p-page-number") == 0)
      pubOutST->pgBl = 1;

    else if(strcmp(parmCStr, "-p-edition") == 0)
      pubOutST->editionBl = 1;

    else if(strcmp(parmCStr, "-p-pmid") == 0)
      pubOutST->pmidBl = 1;

    else if(strcmp(parmCStr, "-p-pmc") == 0)
      pubOutST->pmcBl = 1;

    else if(strcmp(parmCStr, "-p-isbn") == 0)
      pubOutST->isbnBl = 1;

    else if(strcmp(parmCStr, "-p-issn") == 0)
      pubOutST->issnBl = 1;

    else if(strcmp(parmCStr, "-p-url") == 0)
      pubOutST->urlBl = 1;

    else if(strcmp(parmCStr, "-p-abstract") == 0)
      pubOutST->abstractBl = 1;

    else if(strcmp(parmCStr, "-p-journal-short") == 0)
      pubOutST->abvJournalBl = 1;

    else if(strcmp(parmCStr, "-p-article-type") == 0)
      pubOutST->articleTypeBl = 1;

    else if(strcmp(parmCStr, "-p-language") == 0)
      pubOutST->langBl = 1;

    else if(strcmp(parmCStr, "-p-mesh-terms") == 0)
      pubOutST->meshBl = 1;

    else if(strcmp(parmCStr, "-p-keywords") == 0)
      pubOutST->keyWordsBl = 1;

    else if(strcmp(parmCStr, "-p-file-tag") == 0)
      pubOutST->fileNameBl = 1;

    else if(strcmp(parmCStr, "-p-tags-tag") == 0)
      pubOutST->tagsBl = 1;

    else if(strcmp(parmCStr, "-p-supplemental-tag") == 0)
      pubOutST->supBl = 1;

    else if(strcmp(parmCStr, "-p-notesp-tag") == 0)
      pubOutST->notesBl = 1;

    else return 2; // invalid entry
  } // If setting a entry to be printed out

  else if(*(parmCStr + 1) =='n' && *(parmCStr + 2) == 'o')
  { // Else if disabling printing out of an entry
    if(strcmp(parmCStr, "-no-citekey") == 0)
      pubOutST->citeKeyBl = 0;

    else if(strcmp(parmCStr, "-no-journal-id") == 0)
      pubOutST->journalIDBl = 0;

    else if(strcmp(parmCStr, "-no-month") == 0)
      pubOutST->monthBl = 0;

    else if(strcmp(parmCStr, "-no-day") == 0)
      pubOutST->dayBl = 0;

    else if(strcmp(parmCStr, "-no-volume") == 0)
      pubOutST->volBl = 0;

    else if(strcmp(parmCStr, "-no-issue") == 0)
      pubOutST->issueBl = 0;

    else if(strcmp(parmCStr, "-no-doi") == 0)
      pubOutST->doiBl = 0;

    else if(strcmp(parmCStr, "-no-page-number") == 0)
      pubOutST->pgBl = 0;

    else if(strcmp(parmCStr, "-no-edition") == 0)
      pubOutST->editionBl = 0;

    else if(strcmp(parmCStr, "-no-pmid") == 0)
      pubOutST->pmidBl = 0;

    else if(strcmp(parmCStr, "-no-pmc") == 0)
      pubOutST->pmcBl = 0;

    else if(strcmp(parmCStr, "-no-isbn") == 0)
      pubOutST->isbnBl = 0;

    else if(strcmp(parmCStr, "-no-issn") == 0)
      pubOutST->issnBl = 0;

    else if(strcmp(parmCStr, "-no-url") == 0)
      pubOutST->urlBl = 0;

    else if(strcmp(parmCStr, "-no-abstract") == 0)
      pubOutST->abstractBl = 0;

    else if(strcmp(parmCStr, "-no-journal-short") == 0)
      pubOutST->abvJournalBl = 0;

    else if(strcmp(parmCStr, "-no-article-type") == 0)
      pubOutST->articleTypeBl = 0;

    else if(strcmp(parmCStr, "-no-language") == 0)
      pubOutST->langBl = 0;

    else if(strcmp(parmCStr, "-no-mesh-terms") == 0)
      pubOutST->meshBl = 0;

    else if(strcmp(parmCStr, "-no-keywords") == 0)
      pubOutST->keyWordsBl = 0;

    else if(strcmp(parmCStr, "-no-file-tag") == 0)
      pubOutST->fileNameBl = 0;

    else if(strcmp(parmCStr, "-no-tags-tag") == 0)
      pubOutST->tagsBl = 0;

    else if(strcmp(parmCStr, "-no-supplemental-tag") == 0)
      pubOutST->supBl = 0;

    else if(strcmp(parmCStr, "-no-notesp-tag") == 0)
      pubOutST->notesBl = 0;

    else return 2; // invalid entry
  } // Else if disabling printing out of an entry

  else return 2; // invalid entry

  return 0; // Success
} // setBibPVal

/*--------------------------------------------------------\
| Output: prints out bibtex entry help message to stdout
\--------------------------------------------------------*/
void pBibEntyHelp(
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-05 TOC: Sub-01 Sec-01: pBibEntryHelp
  '  - Prints out the help message for selecting which
  '    bibtex entries to print
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  char *helpCStr="\
  \n This help message shows the options that can be\
  \n   enabled or disabled for printing in a bibtex entry.\
  \n Most of these are not a standard bibtex format.\
  \n\
  \n Options:\
  \n  -p-citekey: [Yes]\
  \n    o Print out the generated citation key for a\
  \n      bibtex file.\
  \n    o Disabled by: -no-citekey\
  \n    o Format:\
  \n      - With unac: author1-year-pmid-pmid#\
  \n      - No unac: year-pmid-pmid#\
  \n  -p-journal-id:[Yes]\
  \n    o Prints out the journal id (not standard bibtex).\
  \n    o Disabled by: -no-journal-id\
  \n  -p-month:[Yes]\
  \n    o Prints out the month of publication if found.\
  \n    o Disabled by: -no-month\
  \n    o pubmedToBib only used the DP entry, so this can\
  \n      be missed when it is only in the DEP entry.\
  \n  -p-day:[Yes]\
  \n    o Prints out the day of publication if found.\
  \n    o Disabled by: -no-day\
  \n    o pubmedToBib only used the DP entry, so this can\
  \n      be missed when it is only in the DEP entry.\
  \n  -p-volume:[Yes]\
  \n    o Prints out the volume number entry\
  \n    o Disabled by: -no-volume\
  \n  -p-issue:[Yes]\
  \n    o Prints out the issue number entry\
  \n    o Disabled by: -no-issue\
  \n  -p-doi:[Yes]\
  \n    o Prints out the doi\
  \n    o Disabled by: -no-doi\
  \n  -p-page-number:[Yes]\
  \n    o Prints out the page number\
  \n    o Disabled by: -no-page-number\
  \n  -p-edition:[Yes]\
  \n    o Prints out the edition\
  \n    o Disabled by: -no-edition\
  \n  -p-pmid:[Yes]\
  \n    o Prints out the pmid number\
  \n    o Disabled by: -no-pmid\
  \n  -p-pmc:[Yes]\
  \n    o Prints out the pubmed central id (PMC id)\
  \n    o Disabled by: no-pmc\
  \n  -p-isbn:[No]\
  \n    o Prints out the ISBN number\
  \n    o Disabled by: no-isbn\
  \n  -p-issn:[Yes]\
  \n    o Prints out the ISSN number\
  \n    o Disabled by: no-issn\
  \n  -p-url:[Yes]\
  \n    o Prints out the full doi address\
  \n    o Disabled by: no-url\
  \n  -p-abstract:[Yes]\
  \n    o Prints out the articles abstract\
  \n    o Disabled by: no-abstract\
  \n  -p-journal-short:[No]\
  \n    o Prints out the abbreviated journal name\
  \n    o Disabled by: no-journal-short\
  \n  -p-article-type:[No]\
  \n    o Prints out the list of article types pubmed\
  \n      classified this article as\
  \n    o Disabled by: no-article-type\
  \n  -p-language:[No]\
  \n    o Print out the language the article was published\
  \n      in\
  \n    o Disabled by: no-language\
  \n  -p-mesh-terms:[No]\
  \n    o Print out the mesh terms pubmed added to the\
  \n      article\
  \n    o Disabled by: -no-mesh-terms\
  \n  -p-keywords:[Yes]\
  \n    o Print out the keywords the authors used\
  \n    o Disabled by: -no-keywords\
  \n  -p-file-tag:[Yes]\
  \n    o Print out a File={citekey.pdf}, tag.\
  \n    o This tag is useful to me, but maybe not to\
  \n      others\
  \n    o Disable by: -no-file-tag\
  \n  -p-tags-tag:[Yes]\
  \n    o Print out Tags={tag1, tag2, tag3, ..., tabn},\
  \n    o This tag is useful to me, but may not be to\
  \n      others. It is my way of adding in my own\
  \n      keywords\
  \n    o Disable by: -no-tags-tag\
  \n  -p-supplemental-tag:[Yes]\
  \n    o Prints out an Supplement={}, tag for\
  \n      supplemental files. This tag is something I may\
  \n      use, but may not be useful to others\
  \n    o Disable by: -no-supplemental-tag\
  \n  -p-notesp-tag:[Yes]\
  \n    o Prints out a NotesP{}, tag. I use this tag to\
  \n      my notes about the article. This is useful to\
  \n      me, but may not be useful to others\
  \n    o Disable by: -no-notesp-tag\
  \n";

  printf("%s", helpCStr);
} // pBibEntryHelp
