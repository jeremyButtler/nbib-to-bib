/*#########################################################
# Name: cnvtPubmedToBib
# Use:
#  - Holds functions to convert .pubmed to .bib
# Dependencies:
#  - "cStrFun.h"
#  - "nbibToBibSettings.h"
#  - "unac.h" or <unac.h> (-DLINKUNAC)
#    o libiconv-1.9.1
#  - <stdlib.h>
#  - <stdio.h>
#  o <string.h>
#########################################################*/

#ifndef CNVTNBIBTOBIB_H
#define CNVTNBIBTOBIB_H

#include <stdlib.h>
#include <stdio.h>
#include "cStrFun.h"
#include "nbibToBibSettings.h"

// Check if user wanted to complie with no accent removal
#ifndef NORMACCENT
    #ifdef LINKUNAC
      #include <unac.h> // Link with -luac
    #else
      #include "unac.h" // unac files with ode
    #endif
   // Debain install: apt-get install libunac1-dev
   // Source: https://github.com/QuickDict/unac
   // Doc: https://www.nongnu.org/unac/unac-man3.en.html
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' SOH: Start Of Header
'  - struct-01 pubmedStruct:
'    o Holds data from a .pubmed file entry
'  - struct-02 pubOut:
'    o Holds bolleans for bibtex printing
'  - fun-01 processPubMed:
'    o Processes one section (one paper) of a .pubmed file
'  - fun-02 pubmedSTToBib:
'    o Outputs data in a pubmedStruct to a .bib file
'  - fun-03 blankPubmedST:
'    o Blanks a pubmedStruct structer to 0's and '\0'
'  - fun-04 makeBibEntryName:
'    o Build the citekey and file name.
'    o Format: author1-year-pmid-pmidNumber.
'  - fun-05 findPublicatonType:
'    o Finds the bibtex publication type of a pubmed file.
'    o This has only a limited number of entries
'  - fun-06 getPubmedEntry:
'    o Adds pubmed entry to a pubmedStruct structure
'  - fun-07 getDPPubDate:
'    o Extract year, month, and day from a pubmed DP entry
'  - fun-08 moveToStartOfPubEntry:
'    o Move to the start of pubmed entry & adjust length
'  - fun-09 moveToStartOfPubEntryIgnoreLen:
'    o Move to the start of the pubmed entry only
'  - fun-10 pubmedSTAddTag:
'    o Add a tag to the tag list in a pubmedStruct tag list
'  - fun-11 pubmedSTAddAbstract:
'    o Add an abstract entry to pubmedStruct structure
'  - fun-12 pubmedSTAddFullAuthor:
'    o Add an full author name to the list of authors
'  - fun-13 pubmedSTAddMesh:
'    o Add an mesh term to a list of mesh terms
'  - fun-14 pubmedSTAddKeyWord:
'    o Add an key word to a list of key words
'  - fun-15 pubmedSTAddArticleType:
'    o Add an article type to a list of article types
'  - fun-16 pubmedSTAddTitle:
'    o Add an title to a pubmedStruct structure
'  - fun-17 pubSTAddJounral:
'    o Add an journal name to a pubmedStruct structure
'  - fun-18 finishPubmedSTTag:
'    o Removes the ", " off the end of a tags list
'  - fun-19 finishPubmedSTKeyWords:
'    o Removes the ", " off the end of a key words list
'  - fun-20 finishPubmedSTMesh:
'    o Removes the ", " off the end of a mesh list
'  - fun-21 finishPubmedSTArticleType:
'    o Removes the ", " off the end of an article type list
'  - fun-22 finishPubmedSTAuthors:
'    o Removes the " and " off the end of the author list
'  - fun-23 pubmedSTSetPad:
'    o Set the padding on a pubmedStruct variable
'  - fun-24 pubmedSTSetBreak:
'    o Set the line break values in a pubmedStruct
'  - fun-25 blankPubOutST:
'    o Sets a pubOut structure to default values
'  - fun-26 pubmedLang:
'    o Convert a pubmed language code to full language name
\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Struct-01: pubmedStruct: Holds data from a .pubmed file
\--------------------------------------------------------*/
typedef struct pubmedStruct
{ // pubmedStruct
    // Line wrapping/breaking variables
    unsigned short wrapUS;   // When to wrap lines
    char lineBreakCStr[16];  // What to break lines with
    unsigned char lenBreakUC; // Length of line break
    char padCStr[16];         // Padding to add to lines
    unsigned char lenPadUC;

    // Holds the file name of the pdf
    char fileCStr[255];
    unsigned char lenFileStrUC;

    // Holds my entry name for this entry
    char citeKeyCStr[64]; // Name for this entry
    unsigned char lenCiteKeyStrUC; // # of chars in key

    // Holds bibtex entry type
    char bibTypeCStr[16];
    unsigned char lenBibTypeStrUC;

    // Tags provided by user
    char tagsCStr[1024];         // Tags the user input
    unsigned long lenTagsStrUL; // number of chars in tag
    unsigned short lenTagLineOnUS; // How many chars line

    // pubmed and pubmed centeral ids (PMID or PMC)
    char pmidCStr[16];           // Holds the pubmed id
    unsigned char lenPMIDStrUC;  // lengh of pmidCStr
    char pmcidCStr[16];      // Holds pubmed central id
    unsigned char lenPMCIDStrUC; // lengh of pmcidCStr

    // Abstract variables (AB)
    // 8192 characters should be enough for an abstract
    char abstractCStr[1 << 13];
    unsigned long lenAbstractStrUL;
    unsigned short lenAbstractLineOnUS;
      // How many characters in current line

    // Holds the publication date (DP)
    char yearCStr[8];
    char monthCStr[16];
    char dayCStr[4];
    unsigned char lenYearStrUC;
    unsigned char lenMonthStrUC;
    unsigned char lenDayStrUC;

    // journal id (JID)
    char journalIdCStr[16];
    unsigned char lenJournalIdStrUC;

    // Holds the journal (JT)
    char journalCStr[512];
    unsigned long lenJournalStrUL;
    unsigned short lenJournalLineOnUS;

    // Holds the edition data (EN)
    char editionCStr[64];
    unsigned char lenEditionStrUC;

    // Hold the list of authors names (full names) (FAU)
    char authorsCStr[2048];
    unsigned long lenAuthorsStrUL;
    unsigned short lenAuthorsLineOnUS;

    // Holds the issue number (IP)
    char issueCStr[32];
    unsigned char lenIssueStrUC;

    // Holds the ISBN number
    char isbnCStr[32];
    unsigned char lenIsbnStrUC;
 
    // Holds the ISSN number
       // Linking has priority,
       /// followed by paper, and then eletronic
    char issnCStr[32];
    unsigned char lenIssnStrUC;
    // Booleans to tell which Issn was extracted
    char printISSNBl;
    char electronicISSNBl;
    char linkISSNBl;

    // Hold the list of article types (PT)
    char articleTypeCStr[1024];
    unsigned long lenArticleTypeStrUL;
    unsigned short lenArticleTypeLineOnUS;

    // Holds the journal abrevation (TA)
    char abrevJournalCStr[128];
    unsigned char lenAbrevJournalStrUC;

    // Holds the title of the paper (TI)
    char titleCStr[1024];
    unsigned long lenTitleStrUL;
    unsigned short lenTitleLineOnUS;

    // holds the langauge of the printed article (LA)
    char langCStr[32];
    unsigned char lenLangStrUC;

    // Holds the doi (LID)
    char doiCStr[64];
    unsigned char lenDoiStrUC;

    // Holds the electronic page number (LID)
    char electronicPageCStr[32];
    unsigned char lenEPgStrUC;

    // Hold the list of mesh terms (MH)
    char meshCStr[2048];
    unsigned long lenMeshStrUL;
    unsigned short lenMeshLineOnUS;

    // Hold the list of key words (OT)
    char keyWordsCStr[2048];
    unsigned long lenKeyWordsStrUL;
    unsigned short lenKeyWordsLineOnUS;

    // Holds the non-electronic page number (PG)
    char pageNumCStr[32];
    unsigned char lenPgStrUC;

    // Holds the volume number (VI)
    char volumeNumCStr[64];
    unsigned char lenVolStrUC;
}pubmedStruct;

/*--------------------------------------------------------\
| Struct-02: pubOut: Holds bolleans for bibtex printing
\--------------------------------------------------------*/
typedef struct pubOut
{ // pubOut

  char citeKeyBl;  // Print out the citation key
  char journalIDBl; // Print out the journal id
  char monthBl;    // Print out the publication month
  char dayBl;      // Print out the publicatoin day
  char volBl;      // Print out the volume number
  char issueBl;    // Print out the issue number
  char doiBl;      // Print out the doi number
  char pgBl;       // Print out the page number

  char editionBl;  // Print out the edition
  char pmidBl;     // Print out the pubmed id
  char pmcBl;      //. Print out the pubmed centeral id
  char isbnBl;     // Print out the isbn number
  char issnBl;     // Print out the issn number
  char urlBl;      // Print out url (full doi address)
  char abstractBl; // Print out abstract
  char abvJournalBl; // Print out abbrivated journal
  char articleTypeBl; // Print out the article types
  char langBl;     // Print out the language
  char meshBl;     // Print out the mesh terms
  char keyWordsBl; // Print out the authors key words
  
  char fileNameBl;  // Print out the generated file name
  char tagsBl;      // Print out tags
  char supBl;       // Print out the supplement tag
  char notesBl;     // Print out the notes tag

}pubOut;


/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubmedST to hold extracted information
|    o pubmedFILE to point next PubMed entry to convert
|  - Returns:
|    o 0 for EOF
|    o 1 if succeded
|    o 2 for an invalid file
\--------------------------------------------------------*/
unsigned char processPubMed(
    FILE *pubmedFILE, // pointer to pubmed file to process
    struct pubmedStruct *pubmedST // holds data from file
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-01 Sub-01: processPubMed
   '  - Processes one section (one paper) of a .pubmed file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: prints pubmedStruct to a bibtext file
\--------------------------------------------------------*/
void pubmedSTToBib(
    struct pubmedStruct *pubST, // has .pubmed entries
    struct pubOut *pubOutST,    // tells entries to print
    FILE *bibFILE               // File to output data to
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: pubmedSTToBib
   '  - Outputs data in pubST to a bibtex file
   '  o fun-05 sec-01: Output format
   '  o fun-05 sec-02: Print out bibtex entry type and key
   '  o fun-05 sec-03: Print out the title
   '  o fun-05 sec-04: Print out the author list
   '  o fun-05 sec-05:jounral,year,month,volume,& issue
   '  o fun-05 sec-06: Check if printing the edition
   '  o fun-05 sec-07: Check which page count to print out
   '  o fun-05 sec-08: Print doi, url, ISSN, PMID, & PMCID
   '  o fun-05 sec-09: Check if have a PMC id
   '  o fun-05 sec-10: Check if have a isbn number
   '  o fun-05 sec-11: Check if printing out the file name
   '  o fun-05 sec-12: Print out the tags
   '  o fun-05 sec-13: Check if printing wrapped key words
   '  o Fun-05 sec-14: Check if printing wrapped mesh terms
   '  o fun-05 sec-15: out, notes, abstract, and ending }
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Sets all variables in pubmedST to defaults.
\--------------------------------------------------------*/
void blankPubmedST(
    char blankTagsBl,  // 1: wipe tags; 0: do not
    char blankWrapBl,  // 1: set line wrap to defaults
    char blankBreakBl, //1:Set line break string to default
    char blankPadBl,   //1:Set padding to defaults ("    ")
    struct pubmedStruct *pubmedST // Structure to blank
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: Sec-1 Sub-1: blankPubmedST
   '  - Blanks a pubmedStruct structer to 0's and '\0'
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubmedST->citeKeyCStr to hold the citation key
|    o pubmedST->fileCStr citation key with .pdf ending
\--------------------------------------------------------*/
unsigned char makeBibEntryName(
    struct pubmedStruct *pubmedST
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-04 TOC: makeBibEntryName
   '  - Make name for the cite key/file name.
   '    Format is: author1-year-pmid-pmidNumber.
   '  o fun-04 sec-01: Remove accents from 1st authors name
   '  o fun-04 sec-02: Build the cite key
   '  o fun-04 sec-03: Get length of cite key & add to file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Puts puclication type in pubmedST->bibTypeCStr.
| Note:
|  - This is not comprehensive, but will work for now.
|    o It would be more time efficent to hardcode in an if
|      tree, but I do not want to commit the time.
|  - Preprints are treated like an article
|  - Their can be mutliple article types in a .pubmed file
\--------------------------------------------------------*/
void findPublicationType(
    struct pubmedStruct *pubmedST
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: findPublicatonType
   '  - Finds the bibtex publication type of a pubmed file.
   '  - This has only a limited number of entries
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Modifies pubmedST to hold input line (if valid)
\--------------------------------------------------------*/
void getPubmedEntry(
    char *pubLineCStr,         // pubmed entry to extract
    struct pubmedStruct *pubmedST, // Holds extracted entry
    unsigned long lenPubLineStrUL, // Length of pubLineCStr
    char *onAbstractBl,
      // 1: on the abstract, which stops when their is no
      //    white space
      // 0: Not on the abstact, white space means new entry
      // Is set to 1 if AB entry is found
    char *onJournalBl, // Like onAbstract, but for journal
    char *onTitleBl // like onAbstract for title entry
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-06 TOC: getPubmedEntry
   '  - Finds out wich entry was read in and updates
   '    pubmedST if the information was useful
   '  o fun-06 sec-01: Check if working on the abstract
   '  o fun-06 sec-02: Is this an abstract entry?
   '  o fun-06 sec-03: Is this the publishing date?
   '  o fun-06 sec-04: Check if line on is the edition
   '  o fun-06 sec-05: Is this an full author name?
   '  o fun-06 sec-06: Is this an ISSN or journal issue?
   '  o fun-06 sec-07: Check if line is the journal name
   '  o fun-06 sec-08: Is this language or doi/epage number
   '  o fun-06 sec-09: Check if line is a mesh term
   '  o fun-06 sec-10: Check if line is a keyword
   '  o fun-06 sec-11: Is PMID,PMCID,page #,or article type
   '  o fun-06 sec-12: paper title or journal abbreviation
   '  o fun-06 sec-13: Check if line is a volume number
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Stores year, month, and day in pubmedST
| Note:
|  - This is only for the DP entry. The electronic date
|    and dates that deal with putting on pubmed will not
|    work with this function.
\--------------------------------------------------------*/
void getDPPubDate(
    char *buffCStr,
      // points to start of the date
    struct pubmedStruct *pubmedST  // holds extracted date
);/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-07 TOC: Sec-01 Sub-01: getDPPubDate
  '  - Extract year, month, and day of publication from a
  '    pubmed DP entry
  '  o fun-07 sec-01: Extract the year from the buffer
  '  o fun-07 sec-02: Extract the month from the buffer
  '  o fun-07 sec-03: Extract the day from the buffer
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o entryCStr to point to pubmed entry in line
|    o lenEntryUL to hold length of modified entryCStr
|      the actual value.
|  - Returns:
|    o pointer to the start of the pubmed entry
|  - Note:
|    o This does not hold for the multi-line abstract
|      entries, which have only space padding after the
|      first abstract line.
\--------------------------------------------------------*/
char * moveToStartOfPubEntry(
    char *entryCStr,  // pubmed entry to move to start of
    unsigned long *lenEntryUL
     // Length of entryCStr, will be changed
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ' Fun-08 TOC: Sec-01 Sub-01: moveToStartOfPubEntry
   '  - Move to the start of the pubmed entry & adjust the
   '    the entry length
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o entryCStr to point to start of data (past " - ")
|  - Returns:
|    o pointer to the start of the pubmed entry
|  - Note:
|    o This does not hold for multi-line abstract entries,
|      which have only space padding.
\--------------------------------------------------------*/
char * moveToStartPubEntryNoLen(
    char *entryCStr // pubmed entry to move to start of
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ' Fun-09 TOC: Sec-01 Sub-01: moveToStartPubEntryNoLen
   '  - Move to the start of the pubmed entry
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Add a tag to a list of tags in pubST
| Note:
|  - This adds ", " to the end.
|  - Call finishPubmedSTTag() to remove trailing ", "
\--------------------------------------------------------*/
void pubmedSTAddTag(
    char *tagCStr,          // Tag to add to the reference
    unsigned long lenTagUL,    // Length of the input tag
    struct pubmedStruct *pubST // Structure to add tag to
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-10 TOC: Sec-01 Sub-01: pubmedSTAddTag
   '  - Add a tag to tag list in a pubmedStruct tag list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds abstract entry to pubST (AB tag)
\--------------------------------------------------------*/
void pubmedSTAddAbstract(
    char *abstractCStr,          // Abstract pubST
    unsigned long lenAbstractUL, // lenth of abstract
    struct pubmedStruct *pubST   // Holds abstract
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-11 TOC: Sec-01 Sub-01: pubmedSTAddAbstract
   '  - Add an abstract entry to pubmedStruct structuer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds full author name (tag FAU) to authors list
| Note:
|  - This adds " and " to the end; call
|    - finishPubmedSTAuthors() to remove this
\--------------------------------------------------------*/
void pubmedSTAddFullAuthor(
    char *authorCStr,           // Author to add
    unsigned long lenAuthorUL,  // length of authors name
    struct pubmedStruct *pubST  // Has authors list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-12 TOC: Sec-01 Sub-01: pubSTAddFullAuthor
   '  - Add an full author name to the list of authors
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds a mesh term (MH) to a list of mesh terms
| Note:
|  - This adds ", " to the end.
|  - Call finishPubmedSTMesh() to remove trailing ", "
\--------------------------------------------------------*/
void pubmedSTAddMesh(
    char *meshCStr,             // Mesh term to add
    unsigned long lenMeshUL,    // Length of mesh term
    struct pubmedStruct *pubST  // Has mesh term list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-13 TOC: Sec-01 Sub-01: pubSTAddMesh
   '  - Add an mesh term to a list of mesh terms
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds key word (OT) to pubST's list of key words
| Note:
|  - This adds ", " to end.
|  - After completing the list call
|    finishPubmedSTKeyWords() to remove the trailing ", "
\--------------------------------------------------------*/
void pubmedSTAddKeyWord(
    char *keyWordCStr,          // Keyword to add
    unsigned long lenKeyWordUL, // Length key word
    struct pubmedStruct *pubST  // Has keyword list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-14 TOC: Sec-01 Sub-01: pubSTAddKeyWord
   '  - Add an key word to a list of key words
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds article type (PT) to an article type list
| Note:
|  - This adds ", " to end.
|  - Remove trailng ", " with finishPubmedSTArticleType()
\--------------------------------------------------------*/
void pubmedSTAddArticleType(
    char *articleTypeCStr,         // Has new article type
    unsigned long lenArticleTypeUL,//Length of article type
    struct pubmedStruct *pubST     // Has article type list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-15 TOC: Sec-01 Sub-01: pubSTAddArticleType
   '  - Add an article type to a list of article types
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds title (TI) to pubST's title buffer
\--------------------------------------------------------*/
void pubmedSTAddTitle(
    char *titleCStr,           // Title to add
    unsigned long lenTitleUL,  // Length of the title
    struct pubmedStruct *pubST // Has title buffer
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-16 TOC: Sec-01 Sub-01: pubSTAddTitle
   '  - Add an title to a pubmedStruct structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Adds the journal name to the pubmed entry struct
\--------------------------------------------------------*/
void pubmedSTAddJournal(
    char *journalCStr,           // Jounral to add
    unsigned long lenJounralUL,  // Length of the journal
    struct pubmedStruct *pubST // Has journal buffer
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-17 TOC: Sec-01 Sub-01: pubSTAddJounral
   '  - Add an journal name to a pubmedStruct structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*--------------------------------------------------------\
| Output:Removes trailing ", " from a pubmedStruct tag list
\--------------------------------------------------------*/
void finshPubmedSTTag(
    struct pubmedStruct *pubST
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-18 TOC: Sec-01 Sub-01: finishPubmedSTTag
   '  - Removes the ", " off the end of a tags list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Removes trailing ", " from a key words list
\--------------------------------------------------------*/
void finshPubmedSTKeyWords(
    struct pubmedStruct *pubST // has keyword list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-19 TOC: Sec-01 Sub-01: finishPubmedSTKeyWords
   '  - Removes the ", " off the end of a key words list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Removes trailing ", " from a Mesh list
\--------------------------------------------------------*/
void finshPubmedSTMesh(
    struct pubmedStruct *pubST // has mesh term list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-20 TOC: Sec-01 Sub-01: finishPubmedSTMesh
   '  - Removes the ", " off the end of a mesh list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Removes trailng ", " from a article type list
\--------------------------------------------------------*/
void finshPubmedSTArticleType(
    struct pubmedStruct *pubST // has article type list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-21 TOC: Sec-01 Sub-01: finishPubmedSTArticleType
   '  - Removes trailng ", " off an article type list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Removes trailing " and " from an author list
\--------------------------------------------------------*/
void finshPubmedSTAuthors(
    struct pubmedStruct *pubST // has author list
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-22 TOC: Sec-01 Sub-01: finishPubmedSTAuthors
   '  - Removes the " and " off the end of the author list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubSt->padCStr to be newPadCStr
|    o pubSt->lenPadUC to be length of newPadCStr
\--------------------------------------------------------*/
void pubmedSTSetPad(
  char *newPadCStr, // Padding to add to lines
  struct pubmedStruct *pubST // Has padding value to change
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-23 TOC: Sec-01 Sub-01: pubmedSTSetPad
   '  - Set the padding on a pubmedStruct variable
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubSt->lineBreakCStr to hold lineBreakCStr
|    o pubSt->lenBreakUC to be length of lineBreakCStr
\--------------------------------------------------------*/
void pubmedSTSetBreak(
  char *lineBreakCStr,       // New line break
  struct pubmedStruct *pubST // Has line break to change
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-24 TOC: Sec-01 Sub-01: pubmedSTSetBreak
   '  - Set the line break values in a pubmedStruct
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output: Sets all varialbles in pubOutST to defaults
\--------------------------------------------------------*/
void blankPubOutST(
  struct pubOut *pubOutST // Will be set to default values
);/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-25 TOC: Sec-01 Sub-01: blankPubOutST
  '  - Sets a pubOut structure to default values
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o buffCStr to hold the name of the language
|    o lenLanUC to hold the length of the copied language
|      name (contents of buffCStr)
\--------------------------------------------------------*/
void pubmedLang(
    char *buffCStr, // Buffer to store language name
    char *langCStr, // Thre letter code for the language
    unsigned char *lenLangUC // Will Hold language length
); /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-26 TOC: Sec-1 Sub-01: pubmedLang
   '  - Convert a pubmed language code to language name
   '  o fun-26 sec-01: Languages that start with A
   '  o fun-26 sec-02: Languages that start with B
   '  o fun-26 sec-03: Languages that start with C
   '  o fun-26 sec-04: Languages that start with D
   '  o fun-26 sec-05: Languages that start with E
   '  o fun-26 sec-06: Languages that start with F
   '  o fun-26 sec-07: Languages that start with G
   '  o fun-26 sec-08: Languages that start with H
   '  o fun-26 sec-09: Languages that start with I
   '  o fun-26 sec-10: Languages that start with J
   '  o fun-26 sec-11: Languages that start with K
   '  o fun-26 sec-12: Languages that start with L
   '  o fun-26 sec-13: Languages that start with M
   '  o fun-26 sec-14: Languages that start with N
   '  o fun-26 sec-16: Languages that start with P
   '  o fun-26 sec-18: Languages that start with R
   '  o fun-26 sec-19: Languages that start with S
   '  o fun-26 sec-20: Languages that start with T
   '  o fun-26 sec-21: Languages that start with U
   '  o fun-26 sec-22: Languages that start with V
   '  o fun-26 sec-23: Languages that start with W
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif


