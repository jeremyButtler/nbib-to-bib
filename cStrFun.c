/*#########################################################
# Name: cStrFun
# Use:
#  o Holds functions for copying or manipualting c-strings
# Dependencies:
#  - <string.h> (memcpy for line wrap functions)
#########################################################*/

#include "cStrFun.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' cStrFun SOF: Start Of Functions
'  - fun-1 cStrCpInvsDelm:
'     o Copy c-string till tab, newline, or null
'  - fun-2 cpParmAndArg:
'     o Copies two strings, adding a space between each
'  - fun-03 cStrWrapCp:
'     o Copys a c-string and if needed breaks the c-string
'       into separate lines
'  - fun-04 cStrCpNoWrapSentence:
'     o Copies a c-string and if needed will break at the
'       start of the copied c-string. This is for repeated
'       copying.
'  - fun-05 trimNewLineAtEnd:
'     o Trims a new line or "\r\n" of the end of a buffer
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o cpToCStr to hold the copied C-string
|  - Returns:
|    o pointer to null at end of cpToCStr
\--------------------------------------------------------*/
char * cStrCpInvsDelm(
    char *cpToCStr,  /*C-string to copy values to*/
    char *cpFromCStr /*C-string to copy*/
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-1 Sub-1: cStrCpInvsDelim
   '  - Copy c-string till tab, newline, or null
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    while(*cpFromCStr > 31)
    { /*While have a  c-string to copy*/
        *cpToCStr = *cpFromCStr;
        ++cpToCStr;
        ++cpFromCStr;
    } /*While have a  c-string to copy*/

    *cpToCStr = '\0';
    return cpToCStr;
} /*cStrCpInvsDelim*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o cpToCStr to hold space, parameter, space, & argument
|  - Returns:
|    o pointer to null at end of cpToCStr
\--------------------------------------------------------*/
char * cpParmAndArg(
    char *cpToCStr,  /*Holds copied parameter & argement*/
    char *cpParmCStr,/*Paramater to copy*/
    char *cpArgCStr  /*Argument to copy*/
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: Sec-1 Sub-1: cpSpaceCStr
   '  - Copies two strings, adding a space between each
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    *cpToCStr = ' ';
    ++cpToCStr;
    
    while(*cpParmCStr > 31)
    { /*While have a  c-string to copy*/
        *cpToCStr = *cpParmCStr;
        ++cpToCStr;
        ++cpParmCStr;
    } /*While have a  c-string to copy*/

    *cpToCStr = ' ';
    ++cpToCStr;

    while(*cpArgCStr > 31)
    { /*While have a  c-string to copy*/
        *cpToCStr = *cpArgCStr;
        ++cpToCStr;
        ++cpArgCStr;
    } /*While have a  c-string to copy*/

    *cpToCStr = '\0';
    return cpToCStr;
} /*cpParmAndArg*/

/*--------------------------------------------------------\
| output:
|  - modifies:
|    o dupcstr to hold the contents of cpcstr
|    o cntus to have number of chars in current line. this
|      allows copying of multiple strings to dupcstr.
|    o numcharindupul to have the number chars in dupcstr
|  - returns:
|    o char pointer to '\0' at end of dupcstr
| - note:
|   o copies cpcstr at dupcstr + numcharindupul
|     - When numCharInDupUL = 0, it will overwrite dupCStr
|     - numCharInDupUL > dupCStr whill result in overflow
\--------------------------------------------------------*/
char * cStrWrapCp(
    char *dupCStr,     // Buffer to copy c-string to
    char *cpCStr,      // C-string to copy
    char *padCStr,     // Padding to add to start of breaks
    unsigned char lenPadUC,// Length of padding (index 1)
    unsigned short *cntUS,
      // Number characters in current line of dpuCStr
    unsigned short wrapUS, // Max characters per line
    char *lineBreakCStr,   // What to break a line with
    unsigned char lenBreakUC, // length of lineBreakCStr
    unsigned long *numCharInDupUL
     // Number of characters in dupCStr
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: cStrWrapCp
   '  - Copys a c-string and if needed breaks the c-string
   '    into separate lines
   '  o fun-03 sec-1: Variable declerations
   '  o fun-03 sec-2: Check if need to add a new line
   '  o fun-03 sec-3: Copy the c-string & add needed breaks
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
    ^ Fun-03 Sec-1: Sub-1: Variable declerations
    \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

    char *tmpCStr = padCStr;

    // These variables are for adding in padding. In these
    // cases I may need to maintain a temporary buffer.
    char swapCAry[lenPadUC + 2];      // swap buffer
    unsigned short numSwapCharUS = 0; // characters to swap
    unsigned short swapElmOnUS = 0;   // were at in swap

    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
    ^ Fun-03 Sec-2: Sub-1: Check if need to add a new line
    \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

    dupCStr += *numCharInDupUL; // Move to last character

    if(*cntUS + 1 >= wrapUS)
    { // If I need to start a new line

      while(*dupCStr < 33)
      { // While I am on white space
        --dupCStr;
        --(*numCharInDupUL);
      } // While I am on white space

      ++dupCStr;
      ++(*numCharInDupUL);

      // Add in the line break
      memcpy(dupCStr, lineBreakCStr, lenBreakUC);
      dupCStr += lenBreakUC;

      // Add in the padding
      memcpy(dupCStr, padCStr, lenPadUC);
      dupCStr += lenPadUC;           // Move past padding

      // Adjust the counters
      (*numCharInDupUL) += lenPadUC + lenBreakUC;
      *cntUS = lenPadUC;
    } // If I need to start a new line

    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
    ^ Fun-03 Sec-3: Copy the c-string
    ^  o fun-03 sec-3 sub-1: Need to break a line?
    ^  o fun-03 sec-3 sub-2: Add wrap to end of line
    ^  o fun-03 sec-3 sub-3: 1 character after break point
    ^  o fun-03 sec-3 sub-4: > 1 character after break
    ^  o fun-03 sec-3 sub-5: Add new character to buffer
    \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

    while(*cpCStr != '\0')
    { // While not at the end of the string

        /*************************************************\
        * Fun-03 Sec-3 Sub-1: Need to break a line?
        \*************************************************/

        if(*cntUS >= wrapUS - 1)
        { // If I need to wrap the string
            *cntUS = lenPadUC;
            *dupCStr = '\0';
            tmpCStr = dupCStr - 1;

            while(*tmpCStr > 32)
            { // While not on white space
                --tmpCStr;
                ++(*cntUS); // number of char after break
            } // While not on white space

            /*********************************************\
            * Fun-03 Sec-3 Sub-2: Add wrap to end of line
            \*********************************************/

            // Check if line is 1 word (can not break up)
            // Or if I can just add a break at the end
            if(*cntUS + lenPadUC >= wrapUS   ||
                 *cntUS <= lenPadUC
            ) { // If I am wraping at the end of the line

               while(*dupCStr < 33)
               { // While on white space
                 --dupCStr;
                 --(*numCharInDupUL);
               } // While on white space

               ++dupCStr;
               ++(*numCharInDupUL);

               // Add in the line break
               memcpy(dupCStr, lineBreakCStr, lenBreakUC);
               dupCStr += lenBreakUC; // Move past break

               // Add in the padding
               memcpy(dupCStr, padCStr, lenPadUC);
               dupCStr += lenPadUC; // Move past padding

               // Adjust the counters
               (*numCharInDupUL) += lenPadUC + lenBreakUC;
               *cntUS = lenPadUC;
            } // If I am wraping at the end of the line

            /*********************************************\
            * Fun-03 Sec-3 Sub-4: > 1 character after break
            \*********************************************/

            else
            { // Else I can break up the line
                // Add in the line break
                dupCStr = tmpCStr;
                tmpCStr = lineBreakCStr;
                numSwapCharUS = 0;

                while(numSwapCharUS < lenBreakUC)
                { // Loop till I have copied the break
                    *dupCStr = *tmpCStr;
                    ++tmpCStr;
                    ++dupCStr;

                    swapCAry[numSwapCharUS] = *dupCStr;
                    ++numSwapCharUS;
                } // Loop till I have copied the break

                // Add the padding after the break
                tmpCStr = padCStr;

                while(numSwapCharUS < lenBreakUC +lenPadUC)
                { // Loop till I have copied the break
                    *dupCStr = *tmpCStr;
                    ++tmpCStr;
                    ++dupCStr;

                    swapCAry[numSwapCharUS] = *dupCStr;

                    if(*dupCStr == '\0')
                      *(dupCStr + 1) = '\0';

                    ++numSwapCharUS;
                } // Loop till I have copied the break

                // Remake the line
                //--numSwapCharUS;  // convert to 0 index
                swapCAry[numSwapCharUS] = *dupCStr;
                tmpCStr = dupCStr;
                ++dupCStr;
                swapElmOnUS = 0;

                while(swapCAry[swapElmOnUS] != '\0')
                { // While I have chars to add back in
                    *tmpCStr = swapCAry[swapElmOnUS];
                    swapCAry[swapElmOnUS] = *dupCStr;

                    ++swapElmOnUS; // Move to the next swap
                    ++tmpCStr;
                    ++dupCStr;

                    // Do I need to go back to index 0?
                    if(swapElmOnUS >= numSwapCharUS)
                      swapElmOnUS = 0;  
                } // While I have chars to add back in

                 // Currently is one element to far foward
                 --dupCStr;
                (*numCharInDupUL) +=
                    lenPadUC
                  + lenBreakUC
                  - 1; // -1 for the space I used
            } // Else I can break up the line

            if(*cpCStr == '\0') break; // done
        } // If I need to wrap the string

        /*************************************************\
        * Fun-03 Sec-3 Sub-6: Add new character to buffer
        \*************************************************/

        *dupCStr = *cpCStr;
        ++dupCStr;
        ++cpCStr;
        ++(*numCharInDupUL);
        ++(*cntUS);
    } // While not at the end of the string

    *dupCStr = '\0'; // Mark end of buffer
    return dupCStr;
} // cStrWrapCp

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o dupcstr to hold the contents of cpcstr
|    o cntus to have number of chars in current line. this
|      allows copying of multiple strings to dupcstr.
|  - Returns:
|    o char pointer to '\0' at end of dupcstr
| - Note:
|   o copies cpcstr at dupcstr + numcharindupul
|     - When numCharInDupUL = 0, it will overwrite dupCStr
|     - When numCharInDupUL > dupCStr will overflow dupCStr
\--------------------------------------------------------*/
char * cStrCpNoWrapBuff(
    char *dupCStr,     // Buffer to copy c-string to
    char *cpCStr,      // C-string to copy
    unsigned short lenCpUS,
    char *padCStr,     // Padding to add to start of breaks
    unsigned char lenPadUC,// Length of padding (index 1)
    char *endStrCStr,      // Ending c-string to add in
    unsigned char lenEndUC,// Length of the ending c-string
    unsigned short *cntUS,
      // Number characters in current line of dpuCStr
    unsigned short wrapUS, // Max characters per line
    char *lineBreakCStr,   // What to break a line with
    unsigned char lenBreakUC, // length of lineBreakCStr
    unsigned long *numCharInDupUL
     // Number of characters in dupCStr
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-04 TOC: cStrCpNoWrapSentence
  '  o Copies a c-string and if needed will break at the
  '    start of the copied c-string.
  '  o fun-04 sec-1: Need to add new entry on a new line?
  '  o fun-04 sec-2: Copy the new entry to the buffer
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-04 Sec-1: Sub-1: Should I add entry on new line?
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  dupCStr += *numCharInDupUL;

  if(*cntUS + lenCpUS + lenEndUC >= wrapUS)
  { // If I need to wrap a line

    while(*dupCStr < 33)
    { // While have trailing white space
      --dupCStr;
      --(*numCharInDupUL);
    } // While have trailing white space

    // Acount for one shift off
    ++dupCStr;
    ++(*numCharInDupUL);

    // Add in the line break
    memcpy(dupCStr, lineBreakCStr, lenBreakUC);
    dupCStr += lenBreakUC;

    // Add in the padding
    memcpy(dupCStr, padCStr, lenPadUC);
    dupCStr += lenPadUC;           // Move past padding

    // Adjust the counters
    (*numCharInDupUL) += lenPadUC + lenBreakUC;
    *cntUS = lenPadUC;
  } // If I need to wrap a line

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-04 Sec-2: Sub-1: Copy the new entry to the buffer
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  // Copy in the new string and the ending
  memcpy(dupCStr, cpCStr, lenCpUS);
  dupCStr += lenCpUS;
  (*numCharInDupUL) += lenCpUS;
  *cntUS += lenCpUS;

  if(lenEndUC > 0)
  { // If have an ending to add in
    memcpy(dupCStr, endStrCStr, lenEndUC);
    dupCStr += lenEndUC;
    (*numCharInDupUL) += lenEndUC;
    *cntUS += lenEndUC;
  } // If have an ending to add in

  *dupCStr = '\0';
  return dupCStr;
} // cStrCpNoWrapBuff

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o buffCStr to replace '\n', '\r', or "\r\n" with '\0'
|    o lenBuffUL to account for removal of new line
\--------------------------------------------------------*/
void trimNewLineAtEnd(
    char *buffCStr,  // Buffer to check for new line at end
    unsigned long *lenBuffUL
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: Sec-1 Sub-1: trimNewLineAtEnd
   '  - Trims a new line off the end of a buffer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(*(buffCStr + *lenBuffUL - 1) == '\n')
    { // If I have a new line ending I need to remove
        --(*lenBuffUL); // Buffer will be one char shorter

        if(*(buffCStr + *lenBuffUL - 1) == '\r')
        { // If I need to remove the new line
            *(buffCStr + *lenBuffUL - 1) = '\0';
            --(*lenBuffUL);
        } // If I need to remove the new line

        else *(buffCStr + *lenBuffUL) = '\0';
 
        return;
    } // If I have a new line ending I need to remove

    if(*(buffCStr + *lenBuffUL - 1) == '\r')
    { // If I need to remove the carriage return
        *(buffCStr + *lenBuffUL - 1) = '\0';
        --(*lenBuffUL);
        return;
    } // If I need to remove the carriage return

    return;
} // trimNewLineAtEnd
