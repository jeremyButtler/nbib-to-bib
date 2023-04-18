/*#########################################################
# Name: cStrFun
# Use:
#  o Holds functions for copying or manipualting c-strings
# Dependencies:
#  - <string.h> (memcpy for line wrap functions)
########################################################*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' cStrFun SOF: Start Of Functions
'  - fun-1 cStrCpInvsDelm:
'     o Copy c-string till tap, newline, space, or null
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
'     o Trims a new line off the end of a buffer
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef CSTRFUN_H
#define CSTRFUN_H

#include <string.h>

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
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-1 Sub-1: cStrCpInvsDelim
   '  - Copy c-string till tab, newline, or null
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: Sec-1 Sub-1: cpSpaceCStr
   '  - Copies two strings, adding a space between each
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: cStrWrapCp
   '  - Copys a c-string and if needed breaks the c-string
   '    into separate lines
   '  o fun-03 sec-1: Variable declerations
   '  o fun-03 sec-2: Check if need to add a new line
   '  o fun-03 sec-3: Copy the c-string & add needed breaks
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-04 TOC: cStrCpNoWrapSentence
   '  o Copies a c-string and if needed will break at the
   '    start of the copied c-string.
   '  o fun-04 sec-1: Need to add new entry on a new line?
   '  o fun-04 sec-2: Copy the new entry to the buffer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o buffCStr to replace '\n', '\r', or "\r\n" with '\0'
|    o lenBuffUL to account for removal of new line
\--------------------------------------------------------*/
void trimNewLineAtEnd(
    char *buffCStr,  // Buffer to check for new line at end
    unsigned long *lenBuffUL
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: Sec-1 Sub-1: trimNewLineAtEnd
   '  - Trims a new line off the end of a buffer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif
