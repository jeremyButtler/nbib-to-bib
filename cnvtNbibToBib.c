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

#include "cnvtNbibToBib.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
' SOF: Start Of Functions
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
'  - fun-09 moveToStartPubEntryNoLen:
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
'  - fun-17 pubSTAddJournal:
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
| Output:
|  - Modifies:
|    o pubmedST to hold extracted information
|    o pubmedFILE to point next PubMed entry to convert
|  - Returns:
|    o 0 for EOF
|    o 1 if succeded
|    o 2 for blanlk line
|    o 4 for an invalid file
|    o 8 failed to make citekey
\--------------------------------------------------------*/
unsigned char processPubMed(
    FILE *pubmedFILE, // pointer to pubmed file to process
    struct pubmedStruct *pubmedST // holds data from file
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-01 TOC: Sec-01 Sub-01: processPubMed
   '  - Processes one section (one paper) of a .pubmed file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   char hasEntriesBl = 0; // Mark if has entries
   char *tmpCStr = 0;
   char buffCStr[1024];
       // No PubMed from ncbi has an entry is greater
       // than 80 char per line. So this should be safe

   // These are for keeping trakc of multi-line entries
   char onAbstractBl = 0; 
      // Keep track if processing an abstract
      // (can be multi-line)
   char onJournalBl = 0; // For the journal entry
   char onTitleBl = 0; // keep track of title lines

   unsigned char errUC = 0; // Reports any errors
   unsigned char retErrUC = 0; // Return value
   unsigned long lenBuffUL = 0;

   blankPubmedST(0, 0, 0, 0, pubmedST);
     // Make sure all values are set to their defaults
     // 0's are to avoid setting non-entry specific
     // variables, like line wrapping.

   if(pubmedFILE == 0) return 4;  // Invalid file

   while(fgets(buffCStr, 1024, pubmedFILE))
   { // While have pubmed entries to read
       tmpCStr = buffCStr;
       while(*tmpCStr == ' ' || *tmpCStr =='\t') ++tmpCStr;

       // Check if done with the entry
       if(*tmpCStr < 33)
       { // IF on a blank line (done with entry)
           if(hasEntriesBl == 0) retErrUC = 2;
           else retErrUC = 1;
           break;
       } // IF on a blank line (done with entry)

       hasEntriesBl = 1;
       lenBuffUL = (unsigned short) strlen(buffCStr);       
       getPubmedEntry(
         buffCStr,
         pubmedST,
         lenBuffUL,
         &onAbstractBl,
         &onJournalBl,
         &onTitleBl
       ); // Add the line to the pubmed structuer
   } // While have pubmed entries to read

   errUC = makeBibEntryName(pubmedST);

   // Error in making the reference name
   if(errUC != 0) return 8;

   findPublicationType(pubmedST);

   // Clean up the list endings
   finshPubmedSTKeyWords(pubmedST);
   finshPubmedSTMesh(pubmedST);
   finshPubmedSTArticleType(pubmedST);
   finshPubmedSTAuthors(pubmedST);

   return retErrUC;
} // processPubMed

/*--------------------------------------------------------\
| Output: prints pubmedStruct to a bibtext file
\--------------------------------------------------------*/
void pubmedSTToBib(
    struct pubmedStruct *pubST, // has .pubmed entries
    struct pubOut *pubOutST,    // tells entries to print
    FILE *bibFILE               // File to output data to
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-02 TOC: pubmedSTToBib
   '  - Outputs data in pubST to a bibtex file
   '  o fun-05 sec-01: Output format
   '  o fun-05 sec-02: Print out bibtex entry type and key
   '  o fun-05 sec-03: Print out the title
   '  o fun-05 sec-04: Print out the author list
   '  o fun-05 sec-05:jounral,year,month,volume,& issue
   '  o fun-05 sec-06: Check if printing the edition
   '  o fun-05 sec-07: Check which page count to print out
   '  o fun-05 sec-08:
   '    - doi,url,ISSN,PMID,PMCID,language & article type
   '  o fun-05 sec-09: Check if have a PMC id
   '  o fun-05 sec-10: Check if have a isbn number
   '  o fun-05 sec-11: Check if printing out the file name
   '  o fun-05 sec-12: Print out the tags
   '  o fun-05 sec-13: Check if printing wrapped key words
   '  o Fun-05 sec-14: Check if printing wrapped mesh terms
   '  o fun-05 sec-15: out, notes, abstract, and ending }
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-01 Sub-01: Output format
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   /* Bibtex Format (... means text from variables)
      Note:
        - The Edition, ISBN, & PMC entries will only be printed when
          they were extracted.
        - The title,
            
     @Article{author1-year-pmid-pmidNumber,
       Author={...},
       Title={...},
       Journal={...},
       Year={...},
       Month={...},
       Volume={...},
       Number={...},
       Edition{...},
       doi={...},
       url={https://dx.doi.org/...},
       ISSN={...},
       PMID={...},
       PMC={...},
       ISBN={...},
       File={...},
       Supplement={},
       Tags={...},
       Notes={
       },
       Abstract={
           ....
       },
    }
   */

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-02 Sub-01: Print bibtex entry type and key
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->citeKeyBl & 1)
     fprintf(
       bibFILE,
       "@%s={%s",
       pubST->bibTypeCStr,
       pubST->citeKeyCStr
     );

   else
     fprintf(
       bibFILE,
       "@%s={",
       pubST->bibTypeCStr
     );

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-03 Sub-01: Print out the title
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->wrapUS != 0 &&
      pubST->lenTitleStrUL + 11 > pubST->wrapUS
   )
       fprintf(
           bibFILE,
           "%s  Title={%s%s%s%s  },",
            pubST->lineBreakCStr,
            pubST->lineBreakCStr,
            pubST->padCStr,
            pubST->titleCStr,
            pubST->lineBreakCStr
       ); // need to break title into multiple lines

   else
       fprintf(
         bibFILE,
         "%s  Title={%s},",
         pubST->lineBreakCStr,
         pubST->titleCStr
       );

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-04 Sub-01: Print out the author list
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->wrapUS != 0 &&
      pubST->lenAuthorsStrUL + 12 > pubST->wrapUS
   )
     fprintf(
         bibFILE,
         "%s  Author={%s%s%s%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr,
         pubST->padCStr,
         pubST->authorsCStr,
         pubST->lineBreakCStr
     ); //need to break author list into multiple lines

   else
     fprintf(
       bibFILE,
       "%s  Author={%s},",
       pubST->lineBreakCStr,
       pubST->authorsCStr
     );

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-05 Sub-01:jounral,year,month,volume,& issue
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->wrapUS != 0 &&
      pubST->lenJournalStrUL + 13 > pubST->wrapUS
   ) 
    fprintf(
      bibFILE,
      "%s  Journal={%s%s%s%s  },",
      pubST->lineBreakCStr,
      pubST->lineBreakCStr,
      pubST->padCStr,
      pubST->journalCStr,
      pubST->lineBreakCStr
    ); // Print out a multiline journal entry

   else
     fprintf(
       bibFILE,
       "%s  Journal={%s},", 
       pubST->lineBreakCStr,
       pubST->journalCStr
     ); // Printint the journal name (required)

   if(pubOutST->abvJournalBl & 1)
     fprintf(
       bibFILE,
       "%s  AbrevJournal={%s},", 
       pubST->lineBreakCStr,
       pubST->abrevJournalCStr
     ); // Printint the journal name (required)

   if(pubOutST->journalIDBl & 1)
     fprintf(
       bibFILE,
       "%s  JournalId={%s},", 
       pubST->lineBreakCStr,
       pubST->journalIdCStr
     ); // Printint the journal name

   fprintf(
     bibFILE,
     "%s  Year={%s},", 
     pubST->lineBreakCStr,
     pubST->yearCStr
   ); // Printint the publication year (required bibtex)

   if(pubOutST->monthBl & 1 && pubST->monthCStr[0] !='\0')
     fprintf(
       bibFILE,
       "%s  Month={%s},", 
       pubST->lineBreakCStr,
       pubST->monthCStr
     ); // Printing the pulication month

   if(pubOutST->dayBl & 1 && pubST->dayCStr[0] != '\0')
     fprintf(
       bibFILE,
       "%s  Day={%s},", 
       pubST->lineBreakCStr,
       pubST->dayCStr
     ); // Printing the publication day

   if(pubOutST->volBl & 1)
     fprintf(
       bibFILE,
       "%s  Volume={%s},",
       pubST->lineBreakCStr,
       pubST->volumeNumCStr
     ); // Printing the volume number

   if(pubOutST->issueBl & 1)
     fprintf(
       bibFILE,
       "%s  Number={%s},",
       pubST->lineBreakCStr,
       pubST->issueCStr
     ); // Printing out the issue number

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-06 Sub-01: Check if printing the edition
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->editionCStr[0] != '\0' &&
      pubOutST->editionBl  & 1
   )
       fprintf(
           bibFILE,
           "%s  Edition={%s},",
           pubST->lineBreakCStr,
           pubST->editionCStr
       );

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-07 Sub-01: which page count to print out
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->pgBl & 1)
   { // If I am printing out a page number

     if(pubST->pageNumCStr[0] != '\0')
       fprintf(
         bibFILE,
         "%s  Pages={%s},",
         pubST->lineBreakCStr,
         pubST->pageNumCStr
       );

     else
       fprintf(
         bibFILE,
         "%s  Pages={%s},",
         pubST->lineBreakCStr,
         pubST->electronicPageCStr
       );
   } // If I am printing out a page number

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-08 Sub-01:
   ^  - doi,url,ISSN,PMID,PMCID,language & article type
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/


   if(pubOutST->articleTypeBl & 1)
   { // If I am printing out the article type list
     if(pubST->wrapUS != 0 &&
        pubST->lenArticleTypeStrUL + 17 > pubST->wrapUS
     )
       fprintf(
         bibFILE,
         "%s  ArticleType={%s%s%s%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr,
         pubST->padCStr,
         pubST->articleTypeCStr,
         pubST->lineBreakCStr
       );

     else
       fprintf(
         bibFILE,
         "%s  ArticleType={%s},",
         pubST->lineBreakCStr,
         pubST->articleTypeCStr
       );
   } // If I am printing out the article type list

    if(pubOutST->doiBl & 1)
      fprintf(
        bibFILE,
        "%s  doi={%s},",
        pubST->lineBreakCStr,
        pubST->doiCStr
      );

    if(pubOutST->urlBl & 1)
      fprintf(
        bibFILE,
        "%s  url={https://dx.doi.org/%s},",
        pubST->lineBreakCStr,
        pubST->doiCStr
      );

   if(pubOutST->issnBl & 1)
     fprintf(
         bibFILE,
         "%s  ISSN={%s},",
         pubST->lineBreakCStr,
         pubST->issnCStr
     );

   if(pubOutST->pmidBl & 1)
     if(pubST->pmidCStr[0] != '\0')
       fprintf(
         bibFILE,
         "%s  PMID={%s},",
         pubST->lineBreakCStr,
         pubST->pmidCStr
       );

    if(pubOutST->langBl & 1)
      fprintf(
        bibFILE,
        "%s  language={%s},",
        pubST->lineBreakCStr,
        pubST->langCStr
      ); // If printing out the publication language

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-09 Sub-01: Check if have a PMC id
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->pmcidCStr[0] != '\0' && pubOutST->pmcBl & 1)
     fprintf(
       bibFILE,
       "%s  PMC={%s},",
       pubST->lineBreakCStr,
       pubST->pmcidCStr
     );
    
   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-10 Sub-01: Check if have a isbn number
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubST->isbnCStr[0] != '\0' && pubOutST->isbnBl & 1)
     fprintf(
       bibFILE,
       "%s  ISBN={%s},",
       pubST->lineBreakCStr,
       pubST->isbnCStr
     );

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-11 Sub-01: printing out the file name?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->fileNameBl & 1)
     fprintf(
       bibFILE,
       "%s  File={%s},",
       pubST->lineBreakCStr,
       pubST->fileCStr
     ); // if printing out the file name

    if(pubOutST->supBl & 1)
      fprintf(
        bibFILE,
        "%s  Supplement={},",
        pubST->lineBreakCStr
      ); // Print out the supplement tag

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-12 Sub-01: Print out the tags
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->tagsBl & 1)
   { // If printing out the tags

     if(pubST->wrapUS != 0 &&
        pubST->lenTagsStrUL + 10 > pubST->wrapUS
     )
       fprintf(
         bibFILE,
         "%s  Tags={%s%s%s%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr,
         pubST->padCStr,
         pubST->tagsCStr,
         pubST->lineBreakCStr
       );

     else
       fprintf(
         bibFILE,
         "%s  Tags={%s},",
         pubST->lineBreakCStr,
         pubST->tagsCStr
       );
   } // If printing out the tags

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-13 Sub-01: printing wrapped key words?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->keyWordsBl & 1)
   { // If printing out the key words
     if(pubST->wrapUS != 0 &&
        pubST->lenKeyWordsStrUL + 14 > pubST->wrapUS
     )
       fprintf(
         bibFILE,
         "%s  Keywords={%s%s%s%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr,
         pubST->padCStr,
         pubST->keyWordsCStr,
         pubST->lineBreakCStr
       ); // need to break the title into multiple lines

     else
       fprintf(
         bibFILE,
         "%s  Keywords={%s},",
         pubST->lineBreakCStr,
         pubST->keyWordsCStr
       );
   } // If printing out the key words

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-14 Sub-01: printing wrapped mesh terms?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->meshBl & 1)
   { // If printing out the mesh terms

     if(pubST->wrapUS != 0 &&
        pubST->lenMeshStrUL + 13 > pubST->wrapUS
     )
       fprintf(
         bibFILE,
         "%s  Meshterms={%s%s%s%s  },",
          pubST->lineBreakCStr,
          pubST->lineBreakCStr,
          pubST->padCStr,
          pubST->meshCStr,
          pubST->lineBreakCStr
       ); // need to break the title into multiple lines

     else
       fprintf(
         bibFILE,
         "%s  Meshterms={%s},",
          pubST->lineBreakCStr,
          pubST->meshCStr
       );
   } // If printing out the mesh terms

     /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-05 Sec-15 Sub-01: notes, abstract, and ending }
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   if(pubOutST->notesBl & 1)
       fprintf(
         bibFILE,
         "%s  NotesP={%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr
       ); // Print out the notes tag

   if(pubOutST->abstractBl & 1)
   { // If printing out the abstract
     if(pubST->wrapUS != 0 &&
        pubST->lenAbstractStrUL + 16 > pubST->wrapUS
     )
       fprintf(
         bibFILE,
         "%s  Abstract={%s%s%s%s  },",
         pubST->lineBreakCStr,
         pubST->lineBreakCStr,
         pubST->padCStr,
         pubST->abstractCStr,
         pubST->lineBreakCStr
       );

     else
       fprintf(
         bibFILE,
         "%s  Abstract={%s},",
         pubST->lineBreakCStr,
         pubST->abstractCStr
       );
   } // IF printing out the abstract

   // Print out the closing statement
   fprintf(
     bibFILE,
     "%s}%s",
     pubST->lineBreakCStr,
     pubST->lineBreakCStr
   );

   fflush(bibFILE);
   return;
} // pubmedSTToBib

/*--------------------------------------------------------\
| Output: Sets all variables in pubmedST to defaults.
\--------------------------------------------------------*/
void blankPubmedST(
    char blankTagsBl,  // 1: wipe tags; 0: do not
    char blankWrapBl,  // 1: set line wrap to defaults
    char blankBreakBl, //1:Set line break string to default
    char blankPadBl,   //1:Set padding to defaults ("    ")
    struct pubmedStruct *pubmedST // Structure to blank
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-03 TOC: Sec-1 Sub-1: blankPubmedST
   '  - Blanks a pubmedStruct structer to 0's and '\0'
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    // See if resting the line wrap settings
    if(blankWrapBl & 1) pubmedST->wrapUS = defLineWrap;
    if(blankBreakBl & 1) pubmedSTSetBreak("\n", pubmedST);
    if(blankPadBl & 1) pubmedSTSetPad("    ", pubmedST);

    // variables unique to the bibtex output
    pubmedST->citeKeyCStr[0] = '\0';
    pubmedST->lenCiteKeyStrUC = 0;

    pubmedST->bibTypeCStr[0] = '\0';
    pubmedST->lenBibTypeStrUC = 0;

    pubmedST->fileCStr[0] = '\0';   // Location of pdf
    pubmedST->lenFileStrUC = 0;

    // Tags that the user might input
    if(blankTagsBl & 1)
    { // If blanking the user supplied tags
        pubmedST->tagsCStr[0] = '\0';
        pubmedST->lenTagsStrUL = 0;
        pubmedST->lenTagLineOnUS = pubmedST->lenPadUC;
    } // If blanking the user supplied tags

    // pubmed id variables
    pubmedST->pmidCStr[0] = '\0';
    pubmedST->lenPMIDStrUC = 0;
    pubmedST->pmcidCStr[0] = '\0';
    pubmedST->lenPMCIDStrUC = 0;

    // Abstract variables
    pubmedST->abstractCStr[0] = '\0';
    pubmedST->lenAbstractStrUL = 0;
    pubmedST->lenAbstractLineOnUS = pubmedST->lenPadUC;
      // Setting wrap to a base values so that I have
      // the correct counts for printing. Otherwise the
      // first line would be off by LEN_PAD_UC.
      // This would only happen for multiline entries

   // publication date variables
   pubmedST->yearCStr[0] = '\0';
   pubmedST->lenYearStrUC = 0;
   pubmedST->monthCStr[0] = '\0';
   pubmedST->lenMonthStrUC = 0;
   pubmedST->dayCStr[0] = '\0';
   pubmedST->lenDayStrUC = 0;

   // edition data variables
   pubmedST->editionCStr[0] = '\0';
   pubmedST->lenEditionStrUC = 0;

   // list of author names
   pubmedST->authorsCStr[0] = '\0';
   pubmedST->lenAuthorsStrUL = 0;
   pubmedST->lenAuthorsLineOnUS = pubmedST->lenPadUC;

   // ISBN variables
   pubmedST->isbnCStr[0] = '\0';
   pubmedST->lenIsbnStrUC = '\0';

   // ISSN variables
   pubmedST->issnCStr[0] = '\0';
   pubmedST->lenIssnStrUC = '\0';
   pubmedST->printISSNBl = 0;
   pubmedST->electronicISSNBl = 0;
   pubmedST->linkISSNBl = 0;

   // List of aritcle type variables
   pubmedST->articleTypeCStr[0] = '\0';
   pubmedST->lenArticleTypeStrUL = 0;
   pubmedST->lenArticleTypeLineOnUS = pubmedST->lenPadUC;
 
   // journal abbreviation
   pubmedST->abrevJournalCStr[0] = '\0';
   pubmedST->lenAbrevJournalStrUC = 0;

   // journal name (non-abbrevation) variables
   pubmedST->journalCStr[0] = '\0';
   pubmedST->lenJournalStrUL = 0; 
   pubmedST->lenJournalLineOnUS = pubmedST->lenPadUC;

   // Journal ID
   pubmedST->journalIdCStr[0] = '\0';
   pubmedST->lenJournalIdStrUC = 0;

   // Paper title variables
   pubmedST->titleCStr[0] = '\0';
   pubmedST->lenTitleStrUL = 0;
   pubmedST->lenTitleLineOnUS = pubmedST->lenPadUC;

   // paper language
   pubmedST->langCStr[0] = '\0';
   pubmedST->lenLangStrUC = 0;

   // doi variables
   pubmedST->doiCStr[0] = '\0';
   pubmedST->lenDoiStrUC = 0;

   // electronic page number
   pubmedST->electronicPageCStr[0] = '\0';
   pubmedST->lenEPgStrUC = 0;

   // mesh term variables
   pubmedST->meshCStr[0] = '\0';
   pubmedST->lenMeshStrUL = 0;
   pubmedST->lenMeshLineOnUS = pubmedST->lenPadUC;

   // key word variables
   pubmedST->keyWordsCStr[0] = '\0';
   pubmedST->lenKeyWordsStrUL = 0;
   pubmedST->lenKeyWordsLineOnUS = pubmedST->lenPadUC;

   // page number (non-electronic) variables
   pubmedST->pageNumCStr[0] = '\0';
   pubmedST->lenPgStrUC = 0;

   // volume number variablesk
   pubmedST->volumeNumCStr[0] = '\0';
   pubmedST->lenVolStrUC = 0;

   return;
} // blankPubmedST

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubmedST->citeKeyCStr to hold the citation key
|    o pubmedST->fileCStr citation key with .pdf ending
\--------------------------------------------------------*/
unsigned char makeBibEntryName(
    struct pubmedStruct *pubmedST
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-04 TOC: makeBibEntryName
   '  - Make name for the cite key/file name.
   '    Format is: author1-year-pmid-pmidNumber.
   '  o fun-04 sec-01: Remove accents from 1st authors name
   '  o fun-04 sec-02: Build the cite key
   '  o fun-04 sec-03: Get length of cite key & add to file
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ' Fun-04 Sec-01 Sub-01: Remove accents from 1st authors
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   char *nameCStr = 0;
   size_t lenCnvtStrUL = 0;  // Lenght of converted string

   // here so the user can remove unac as a dependency
   #ifndef NORMACCENT
       int lenFirstNameI = 0;
       int errI = 0;

       nameCStr = pubmedST->authorsCStr;

       // Find end of the last name
       while(*nameCStr > 32)
       { // While have names to copy over
           ++nameCStr;
           ++lenFirstNameI;

           if(*nameCStr == ',') break;
       } // While have names to copy over

       nameCStr = 0;  // blank for unac

       // https://www.nongnu.org/unac/unac-man3.en.html
       errI =
           unac_string(
               "UTF-8",       // Encoding of string
               pubmedST->authorsCStr,// string of authors
               lenFirstNameI, // length to convert
               &nameCStr,     // Holds normalized string
               &lenCnvtStrUL
       ); // remove any accents from the authors names

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ' Fun-04 Sec-02 Sub-01: Build the cite key
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       if(errI != 0) return 2; // name normalization error

       memcpy(
         pubmedST->citeKeyCStr,
         nameCStr,
         lenCnvtStrUL
       );

       if(nameCStr != 0) free(nameCStr);

       // Add in the spacer between first author and year
       nameCStr = &pubmedST->citeKeyCStr[lenCnvtStrUL];
       *nameCStr = '-';
       ++nameCStr;

   #else // unac disablled
       nameCStr = pubmedST->citeKeyCStr;
   #endif

   // Add in the year
   memcpy(
     nameCStr,
     pubmedST->yearCStr,
     pubmedST->lenYearStrUC
   );

   nameCStr += pubmedST->lenYearStrUC;

   // Add in the pmid ending
   memcpy(nameCStr, "-pmid-", 6);
   nameCStr += 6;

   memcpy(
     nameCStr,
     pubmedST->pmidCStr,
     pubmedST->lenPMIDStrUC
   );

   // Make the name into a c-string
   nameCStr += pubmedST->lenPMIDStrUC;
   *nameCStr = '\0';

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ' Fun-04 Sec-03 Sub-01: length of cite key & build file
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   // Get the id length
   pubmedST->lenCiteKeyStrUC =
       lenCnvtStrUL
       + 1                // "-" between name and year
       + pubmedST->lenYearStrUC
       + 6                // "-pmid-" 
       + pubmedST->lenPMIDStrUC;

   // Make the file name and set its length
   nameCStr =
     cStrCpInvsDelm(
       pubmedST->fileCStr,
       pubmedST->citeKeyCStr
   );

   cStrCpInvsDelm(nameCStr, ".pdf");

   pubmedST->lenFileStrUC = pubmedST->lenCiteKeyStrUC + 4;

   return 0;
} // makeBibEntryName

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-05 TOC: findPublicatonType
   '  - Finds the bibtex publication type of a pubmed file.
   '  - This has only a limited number of entries
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(
      strstr(
        pubmedST->articleTypeCStr,
        "Unpublished Work"
      ) != 0
    ) { // If this has not been published yet
        memcpy(pubmedST->bibTypeCStr, "Unpublished\0", 12);
        pubmedST->lenBibTypeStrUC = 11;
        return;
    } // If this has not been published yet

    if(strstr(pubmedST->articleTypeCStr, "Preprint") != 0)
    { // If this has not been published yet
        memcpy(pubmedST->bibTypeCStr, "Article\0", 8);
        pubmedST->lenBibTypeStrUC = 7;
        return;
    } // If this has not been published yet

    if(strstr(pubmedST->articleTypeCStr, "Article") != 0)
    { // If is an article of some kind
        memcpy(pubmedST->bibTypeCStr, "Article\0", 8);
        pubmedST->lenBibTypeStrUC = 7;
        return;
    } // If is an article of some kind

    if(strstr(pubmedST->articleTypeCStr, "article") != 0)
    { // If is an article of some kind
        memcpy(pubmedST->bibTypeCStr, "Article\0", 8);
        pubmedST->lenBibTypeStrUC = 7;
        return;
    } // If is an article of some kind

    if(strstr(pubmedST->articleTypeCStr, "Book") != 0)
    { // If is an book of some kind
        memcpy(pubmedST->bibTypeCStr, "Book\0", 5);
        pubmedST->lenBibTypeStrUC = 4;
        return;
    } // If is an book of some kind

    if(strstr(pubmedST->articleTypeCStr, "book") != 0)
    { // If is an book of some kind
        memcpy(pubmedST->bibTypeCStr, "Book\0", 5);
        pubmedST->lenBibTypeStrUC = 4;
        return;
    } // If is an book of some kind

    if(strstr(pubmedST->articleTypeCStr, "Goverment") != 0)
    { // If is an goverment report
        memcpy(pubmedST->bibTypeCStr, "Techreport\0", 11);
        pubmedST->lenBibTypeStrUC = 10;
        return;
    } // If is an goverment report

    if(
      strstr(
        pubmedST->articleTypeCStr,
        "Technical Report"
      ) != 0
    ) { // If is an goverment report
        memcpy(pubmedST->bibTypeCStr, "Techreport\0", 11);
        pubmedST->lenBibTypeStrUC = 10;
        return;
    } // If is an goverment report

    if(
      strstr(
        pubmedST->articleTypeCStr,
        "Public Service Announcement"
      ) != 0
    ) { // Not sure were to catagorize this
        memcpy(pubmedST->bibTypeCStr, "Misc\0", 5);
        pubmedST->lenBibTypeStrUC = 4;
        return;
    } // Not sure were to catagorize this

    // Else I am unsure how to classify it. People can
    memcpy(pubmedST->bibTypeCStr, "Misc\0", 5);
    pubmedST->lenBibTypeStrUC = 4;
    return;

    /*Bibtex publication types:
     @article: any article published in a periodical
       (journal/magazine)
     @book: a book
     @booklet: like a book but without a designated
       publisher
     @conference: a conference paper
     @inbook: a section or chapter in a book
     @incollection: an article in a collection
     @inproceedings: a conference paper (same as
       conference entry)
     @manual: a technical manual
     @masterthesis: a Masters thesis
     @misc: used if nothing else fits
     @phdthesis: a PhD thesis
     @proceedings: the whole conference proceedings
     @techreport: a technical report, government report or
       white paper
     @unpublished: a work that has not yet been officially
       published
    */

    /*For pubmed types (thier are a lot) see:
        https://www.nlm.nih.gov/mesh/pubtypes.html
    */
} // findPublicationType

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-06 TOC: getPubmedEntry
   '  - Finds out wich entry was read in and updates
   '    pubmedST if the information was useful
   '  o fun-06 sec-01: Working on abstract/title?
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

   char *tmpCStr = 0;

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-06 Sec-01 Sub-01: Working on abstract or title?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   switch(*onAbstractBl)
   { // If was on an abstract entry
       case 1:
       // Case: Was on an abstract entry
           // If on an abstract entry; make sure case fires
           if(*pubLineCStr < 33) *pubLineCStr = 'A';
           else *onAbstractBl = 0;

           break;
       // Case: Was on an abstract entry

       default: break;
   } // If was on an abstract entry

   switch(*onTitleBl)
   { // If was working on a title entry
     case 1:
     // Case: on the title entry
       if(*pubLineCStr < 33)
       { // If on a a continued title
         // There are four spaces, so I can input "TI- "
         *pubLineCStr = 'T';
         *(pubLineCStr + 1) = 'I';
         *(pubLineCStr + 2) = '-';
       } // If on a a continued title

       else *onTitleBl = 0;

       break;
   } // If was working on a title entry

   switch(*onJournalBl)
   { // Switch; if continuing a journal entry
     case 1:
     // Case: on the title entry
       if(*pubLineCStr < 33)
       { // If on a a continued journal entry
         // There are four spaces, so I can input "JT- "
         *pubLineCStr = 'J';
         *(pubLineCStr + 1) = 'T';
         *(pubLineCStr + 2) = '-';
       } // If on a a continued title

       else *onJournalBl = 0;

       break;
   } // Switch; if continuing a journal entry

   /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
   ^ Fun-06 Sec-02 Sub-01: On the abstract entry?
   \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

   switch(*pubLineCStr)
   { // Switch; check the first character

       case 'A':
       // Case for entries starting with A
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Check if is an entry I do not care about
           // These are AID; AU (author names), and AD
           // (author location).
           if(*onAbstractBl == 0 && *pubLineCStr != 'B')
               return;

           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
               return;

           if(*onAbstractBl == 0)
           { // If I am on the first abstract entry
               *onAbstractBl = 1;
               pubLineCStr =
                   moveToStartOfPubEntry(
                     pubLineCStr,
                     &lenPubLineStrUL
               );
           } // If I am on the first abstract entry

           else
           { // Else I need to move past the white space
               while(*pubLineCStr < 33)
               { // While I need to move past the padding
                   ++pubLineCStr;
                   --lenPubLineStrUL;
               } // While I need to move past the padding
           } // Else I need to move past the white space

           // make sure no trailing new lines
           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // Add the abstract entry
           pubmedSTAddAbstract(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           return;
       // Case for entries starting with A


       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-03 Sub-01: Is publishing date?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'D':
       // Case for entries starting with D
           // Check if I am interesed in this entry
           if(*(pubLineCStr + 1) != 'P') return;

           if(*(pubLineCStr+2)>32 && *(pubLineCStr+2)!='-')
               return;

           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           getDPPubDate(pubLineCStr, pubmedST);
           return; // done
       // Case for entries starting with D

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-04 Sub-01: Is edition?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'E':
       // Case for entries starting with E
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Check if I am interesed in this entry
           if(*pubLineCStr != 'N') return;

           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
               return;

           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr, 
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);
           
           pubmedST->lenEditionStrUC =
             (unsigned char) lenPubLineStrUL; 

           strcpy(pubmedST->editionCStr, pubLineCStr);
           return; // done
       // Case for entries starting with E

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-05 Sub-01: is full author name?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'F':
       // Case for entries starting with F
           ++pubLineCStr;
           --lenPubLineStrUL;

           if(*pubLineCStr != 'A') return;
           if(*(pubLineCStr + 1) != 'U') return;
           if(*(pubLineCStr+2)>32 && *(pubLineCStr+2)!='-')
               return;

           // Move to start & remove trailing newlines
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // Add the author to the list of authors
           pubmedSTAddFullAuthor(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           return; // finished
       // Case for entries starting with F

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-06: if line is ISSN or journal issue
       ^  o fun-06 sec-06 sub-1: If line is jounal issue
       ^  o fun-06 sec-06 sub-2: If line is the ISSN
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'I':
       // case for entries starting with I
           ++pubLineCStr;
           --lenPubLineStrUL;

           /**********************************************\
           * Fun-06 Sec-06 Sub-01: Jounal issue
           \**********************************************/

           if(*pubLineCStr == 'P')
           { // If this is the issue entry
               if(*(pubLineCStr + 1) > 32 &&
                  *(pubLineCStr + 1) != '-'
               ) return;

               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               );
               
               pubmedST->lenIssueStrUC = lenPubLineStrUL;
               strcpy(pubmedST->issueCStr, pubLineCStr);
               return;
           } // If this is the issue entry

           /**********************************************\
           * Fun-06 Sec-06 Sub-02: If line is the ISSN
           \**********************************************/

           if(*pubLineCStr != 'S') return;

           if(*(pubLineCStr + 1) == 'B')
           { // If this may be the ISBN
               if(*(pubLineCStr + 2) != 'N') return;

               if(*(pubLineCStr + 3) > 32 &&
                  *(pubLineCStr + 3) != '-'
               ) return;

               // prepare the isbn for copying
               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               );
               
               pubmedST->lenIsbnStrUC = lenPubLineStrUL;
               strcpy(pubmedST->isbnCStr, pubLineCStr);
               return;
           } // If this may be the ISBN

           if(*(pubLineCStr + 1) > 32 &&
              *(pubLineCStr + 1) != '-'
           ) return;

           // These checks are to make sure that I do not
           // overwite my target ISSN (linking ISSN, then
           // Print, then Electronic, then anything

           if(pubmedST->linkISSNBl & 1) return;

           if(strstr(pubLineCStr, "Linking") != 0)
               pubmedST->linkISSNBl = 1;

           else if(pubmedST->printISSNBl & 1) return;

           else if(strstr(pubLineCStr, "Print") != 0)
               pubmedST->printISSNBl = 1;

           else if(pubmedST->electronicISSNBl & 1) return;

           else if(strstr(pubLineCStr, "Electronic") != 0)
               pubmedST->electronicISSNBl = 1;

           // Copy over the found issn
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);
           // Find the end of the issn
           tmpCStr = strstr(pubLineCStr, " ");
           *tmpCStr = '\0';

           pubmedST->lenIssnStrUC =
               (unsigned char) (tmpCStr - pubLineCStr); 

           strcpy(pubmedST->issnCStr, pubLineCStr);
           return;
       // case for entries starting with I

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-07 Sub-01: is this a journal name
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'J':
       // Case for the journal title (JT)
           ++pubLineCStr;
           --lenPubLineStrUL;

           if(*pubLineCStr == 'I')
           { // IF this might be a journal id
               // Check if is a journal id
               if(*(pubLineCStr + 1) != 'D') return;

               if(*(pubLineCStr + 2) > 32 &&
                  *(pubLineCStr + 2) != '-'
               ) return;

               // prepare journal name for copying
               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               ); // remove trailing '\n' or "\r\n"

               pubmedST->lenJournalIdStrUC =
                   (unsigned char) lenPubLineStrUL;

               strcpy( 
                   pubmedST->journalIdCStr,
                   pubLineCStr
               ); // Copy the journal id over

               return;
           } // IF this might be a journal id

           // if is not the journal title
           if(*pubLineCStr != 'T') return;

           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
               return;

           // prepare journal name for copying
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           pubmedSTAddJournal(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           *onJournalBl = 1; // Mark on a journal entry
           return;
       // Case for the journal title (JT)

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-08: is language or doi/epage number?
       ^  o fun-06 sec-08 sub-01: Check if is the language
       ^  o fun-06 sec-08 sub-02: Check if is the doi
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       /**************************************************\
       * Fun-06 Sec-08 Sub-01: Check if is the language
       \**************************************************/

       case 'L':
       // case for LID (doi/epage number) and LA (language)
           ++pubLineCStr;
           --lenPubLineStrUL;

           if(*pubLineCStr == 'A')
           { // If is the language of the publication
               if(*(pubLineCStr + 1) > 32 &&
                  *(pubLineCStr + 1) != '-'
               ) return;

               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               pubmedLang(
                   pubmedST->langCStr,
                   pubLineCStr,
                   &pubmedST->lenLangStrUC
               ); // Convert the language code

               return;
           } // If is the language of the publication

           /**********************************************\
           * Fun-06 Sec-08 Sub-02: Check if is the doi
           \**********************************************/

           // Check if on LID entry (ignore other entries)
           if(*pubLineCStr != 'I') return;
           if(*(pubLineCStr + 1) != 'D') return;
           if(*(pubLineCStr+2)>32 && *(pubLineCStr+2)!='-')
                   return;

           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           if(strstr(pubLineCStr, "[doi]") != 0)
           { // If this was a doi
               // Remove the space after the doi entry
               tmpCStr = strstr(pubLineCStr, " ");
               *tmpCStr = '\0';

               pubmedST->lenDoiStrUC =
                   (unsigned char) (tmpCStr - pubLineCStr); 

               strcpy(pubmedST->doiCStr, pubLineCStr);
           } // If this was a doi

           else
           { // Else was an electronic page number
               strcpy(
                 pubmedST->electronicPageCStr,
                 pubLineCStr
               );

               pubmedST->lenEPgStrUC =
                 (unsigned char) lenPubLineStrUL;
           } // Else was an electronic page number

           return;
       // case for LID (doi/epage number) and LA (language)

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-09 Sub-01: Check if line is a mesh term
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'M':
       // Case for mesh terms (MH)
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Check if interested in this entry
           if(*pubLineCStr != 'H') return;
           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
                   return;

           // Prepare the buffer for copying the mesh term
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // Copy the mesh term to the list of mesh terms
           pubmedSTAddMesh(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           return;
       // Case for mesh terms (MH)

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-10 Sub-01: Check if line is a keyword
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'O':
       // Case for the authors key words
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Check if interested in this entry
           if(*pubLineCStr != 'T') return;
           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
                   return;

           // Prepare to copy the key word
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // Copy the key word to the list of key words
           pubmedSTAddKeyWord(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           return;
       // Case for the authors key words

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-11:
       ^  - is a PMID, PMCID, page number, or article type?
       ^  o fun-06 sec-11 sub-01: Check if is a PMCID
       ^  o fun-06 sec-11 sub-02: Check if is a PMID
       ^  o fun-06 sec-11 sub-03: is an page number entry?
       ^  o fun-06 sec-11 sub-04: is an article type entry?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
      
       /**************************************************\
       * Fun-06 Sec-11 Sub-01: Check if is a PMCID
       \**************************************************/

       case 'P':
       // Case for ids, page nubmers, and article type
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Check if is a PMID or PMCID entry
           if(*pubLineCStr == 'M')
           { // If is an id entry
               ++pubLineCStr;
               --lenPubLineStrUL;

               if(*pubLineCStr == 'C')
               { // If dealing with a pubmed centerla id
                   if(*(pubLineCStr+1) > 32 &&
                      *(pubLineCStr+1) != '-'
                   ) return;

                   pubLineCStr =
                       moveToStartOfPubEntry(
                           pubLineCStr,
                           &lenPubLineStrUL
                   ); // Move off the tag to the entry

                   trimNewLineAtEnd(
                     pubLineCStr,
                     &lenPubLineStrUL
                   );

                   strcpy(
                     pubmedST->pmcidCStr,
                     pubLineCStr
                   );

                   pubmedST->lenPMCIDStrUC=
                       (unsigned char) lenPubLineStrUL;

                   return;
               } // If dealing with a pubmed centerla id

               /******************************************\
               * Fun-06 Sec-11 Sub-02: Check if is a PMID
               \******************************************/

               // Check if is not a PMCID
               if(*pubLineCStr != 'I') return;
               if(*(pubLineCStr + 1) != 'D') return;

               if(*(pubLineCStr + 2) > 32 &&
                  *(pubLineCStr + 2) != '-'
               ) return;
               
               // Only a PMCID is left
               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               );

               strcpy(pubmedST->pmidCStr, pubLineCStr);

               pubmedST->lenPMIDStrUC =
                 (unsigned char) lenPubLineStrUL;

               return;
           } // If is an id entry

           /**********************************************\
           * Fun-06 Sec-11 Sub-03: is an page number entry?
           \**********************************************/

           if(*pubLineCStr == 'G')
           { // If is a page number
               if(*(pubLineCStr + 1) > 32 &&
                  *(pubLineCStr + 1) != '-'
               ) return;

               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               );

               strcpy(pubmedST->pageNumCStr, pubLineCStr);

               pubmedST->lenPgStrUC =
                 (unsigned char) lenPubLineStrUL;

               return;
           } // If is a page number

           /**********************************************\
           * Fun-06 Sec-11 Sub-04: is article type entry?
           \**********************************************/

           // check if is not an PT
           if(*pubLineCStr != 'T') return;
           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
               return;

           ++pubLineCStr;
           --lenPubLineStrUL;

           // Prepare the buffer to be copied
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // copy the buffer to the article type list
           pubmedSTAddArticleType(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           return;
       // Case for ids, page nubmers, and article type

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-12 Sub-01: title/journal abbreviation
       ^  o fun-06 sec-12 sub-01: Is journal abbreviation?
       ^  o fun-06 sec-12 sub-02: Is a paper title?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       /**************************************************\
       * Fun-06 Sec-12 Sub-01: Is a journal abbreviation?
       \**************************************************/

       case 'T':
       // Case for title (TI) or journal abrevation (TA)
           ++pubLineCStr;
           --lenPubLineStrUL;

           if(*pubLineCStr == 'A')
           { // If found the journal abrevation
               if(*(pubLineCStr + 1) > 32 &&
                  *(pubLineCStr + 1) != '-'
               ) return;

               pubLineCStr =
                 moveToStartOfPubEntry(
                   pubLineCStr,
                   &lenPubLineStrUL
               );

               trimNewLineAtEnd(
                 pubLineCStr,
                 &lenPubLineStrUL
               );

               pubmedST->lenAbrevJournalStrUC =
                   (unsigned char) lenPubLineStrUL; 

               strcpy(
                 pubmedST->abrevJournalCStr,
                 pubLineCStr
               );

               return;
           } // If found the journal abrevation

           /**********************************************\
           * Fun-06 Sec-12 Sub-02: Is an paper title?
           \**********************************************/

           // If not the title of the paper
           if(*pubLineCStr != 'I') return;
           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
                   return;

           // Prepare title for copying
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // Copy the title
           pubmedSTAddTitle(
             pubLineCStr,
             lenPubLineStrUL,
             pubmedST
           );

           // Mark that I am working on a title
           *onTitleBl = 1;
           return;
       // Case for title (TI) or journal abrevation (TA)

       /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
       ^ Fun-06 Sec-13 Sub-01: Is a volume number?
       \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

       case 'V':
       // Case for the volume number
           ++pubLineCStr;
           --lenPubLineStrUL;

           // Remove any non-volume number entries
           if(*pubLineCStr != 'I') return;
           if(*(pubLineCStr+1)>32 && *(pubLineCStr+1)!='-')
                   return;

           // Prepare volume number for copying
           pubLineCStr =
             moveToStartOfPubEntry(
               pubLineCStr,
               &lenPubLineStrUL
           );

           trimNewLineAtEnd(pubLineCStr, &lenPubLineStrUL);

           // copy the volume number
           pubmedST->lenVolStrUC =
             (unsigned char) lenPubLineStrUL; 

           strcpy(pubmedST->volumeNumCStr, pubLineCStr);
           return;
       // Case for the volume number

       default: return;  // I do not care about this entry
   } // Switch; check the first character

   return; // This should never fire
} // getPubmedEntry

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
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-07 TOC: Sec-01 Sub-01: getDPPubDate
  '  - Extract year, month, and day of publication from a
  '    pubmed DP entry
  '  o fun-07 sec-01: Extract the year from the buffer
  '  o fun-07 sec-02: Extract the month from the buffer
  '  o fun-07 sec-03: Extract the day from the buffer
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-07 Sec-01 Sub-01: Extract the year from the buffer
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  pubmedST->lenYearStrUC = 0;

  while(*buffCStr > 32)
  { // While I am not on white space (on the year entry)
    pubmedST->yearCStr[pubmedST->lenYearStrUC] = *buffCStr;
    ++pubmedST->lenYearStrUC;
    ++buffCStr;
  } // While I am not on white space (on the year entry)

  if(*buffCStr == '\0' || *buffCStr == '\n')
  { // If their was only a year
    pubmedST->lenMonthStrUC = 0;
    pubmedST->monthCStr[0] = '\0';
    pubmedST->lenDayStrUC = 0;
    pubmedST->dayCStr[0] = '\0';

    return;
  } // If their was only a year

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-07 Sec-02 Sub-01: Extract the month from the buffer
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  pubmedST->lenMonthStrUC = 0;
  ++buffCStr; // Get off the white space

  while(*buffCStr > 32)
  { // While I am not on white space (on the year entry)
    pubmedST->monthCStr[pubmedST->lenMonthStrUC]=*buffCStr;
    ++pubmedST->lenMonthStrUC;
    ++buffCStr;
  } // While I am not on white space (on the year entry)

  if(*buffCStr == '\0' || *buffCStr == '\n')
  { // If their was no day
    pubmedST->lenDayStrUC = 0;
    pubmedST->dayCStr[0] = '\0';

    return;
  } // If their was no day

  /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
  ^ Fun-07 Sec-03 Sub-01: Extract the day from the buffer
  \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

  pubmedST->lenDayStrUC = 0;
  ++buffCStr; // Get off the white space

  while(*buffCStr > 32)
  { // While I am not on white space (on the year entry)
    pubmedST->dayCStr[pubmedST->lenDayStrUC] = *buffCStr;
    ++pubmedST->lenDayStrUC;
    ++buffCStr;
  } // While I am not on white space (on the year entry)

  return; // finished
} // getDPPubDate

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ' Fun-08 TOC: Sec-01 Sub-01: moveToStartOfPubEntry
   '  - Move to the start of the pubmed entry & adjust the
   '    the entry length
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    while(*entryCStr != '-')
    { // While I have to find the -
        ++entryCStr;
        --(*lenEntryUL);
    } // While I have to find the -

    ++entryCStr; // Get off the dash
    --(*lenEntryUL);

    while(*entryCStr < 33)
    { // While i have not found the start of the entry
        --(*lenEntryUL);
        ++entryCStr;
    } // While i have not found the start of the entry

    return entryCStr;
} // moveToStartOfPubEntry

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ' Fun-09 TOC: Sec-01 Sub-01: moveToStartPubEntryNoLen
   '  - Move to the start of the pubmed entry
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    while(*entryCStr != '-') ++entryCStr;

    ++entryCStr; // Get off the dash

    while(*entryCStr < 33) ++entryCStr;
    return entryCStr;
} // moveToStartOfPubEntry

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-10 TOC: Sec-01 Sub-01: pubmedSTAddTag
   '  - Add a tag to tag list in a pubmedStruct tag list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->wrapUS == 0)
    { // If not applying wrapping
        pubST->lenTagsStrUL += lenTagUL;
        memcpy(
            pubST->tagsCStr + pubST->lenTagsStrUL,
            tagCStr,
            lenTagUL
        ); // Copy the tag to the list

        pubST->lenTagsStrUL += 2;
         memcpy(
           pubST->tagsCStr + pubST->lenTagsStrUL,
           ", ",
           2
         );

        pubST->lenTagsStrUL += 2;

        return;
    } // If not applying wrapping

    cStrCpNoWrapBuff(
        pubST->tagsCStr,
        tagCStr,
        (unsigned short) lenTagUL,
        pubST->padCStr,
        pubST->lenPadUC,
        ", ",
        2,
        &pubST->lenTagLineOnUS,
        pubST->wrapUS,
        pubST->lineBreakCStr,
        pubST->lenBreakUC,
        &pubST->lenTagsStrUL
    ); // Add the tag to the list of tags

    return;
} // pubmedSTAddTag

/*--------------------------------------------------------\
| Output: Adds abstract entry to pubST (AB tag)
\--------------------------------------------------------*/
void pubmedSTAddAbstract(
    char *abstractCStr,          // Abstract pubST
    unsigned long lenAbstractUL, // lenth of abstract
    struct pubmedStruct *pubST   // Holds abstract
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-11 TOC: Sec-01 Sub-01: pubmedSTAddAbstract
   '  - Add an abstract entry to pubmedStruct structuer
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->abstractCStr[pubST->lenAbstractStrUL],
           abstractCStr,
           lenAbstractUL
       ); // Copy the abstract
 
       pubST->lenAbstractStrUL += lenAbstractUL;
       pubST->abstractCStr[pubST->lenAbstractStrUL] = '\0';

       return;
   } // If the user does not want line wrapping

   cStrWrapCp(
       pubST->abstractCStr,
       abstractCStr,      // A line in the abstract
       pubST->padCStr,
       pubST->lenPadUC,
       &pubST->lenAbstractLineOnUS, // current line length
       pubST->wrapUS,               // Max line length
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenAbstractStrUL // Characters in abstract
   ); // Copying the abstract entry (will add '\0' to end)

   return;
} // pubSTAddAbstract

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-12 TOC: Sec-01 Sub-01: pubSTAddFullAuthor
   '  - Add an full author name to the list of authors
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->authorsCStr[pubST->lenAuthorsStrUL],
           authorCStr,
           lenAuthorUL
       ); // Copy the author name
 
       pubST->lenAuthorsStrUL += lenAuthorUL;

       memcpy(
         &pubST->meshCStr[pubST->lenMeshStrUL],
         " and ",
         5
       );

       pubST->lenAuthorsStrUL += 5; // account for " and "

       return;
   } // If the user does not want line wrapping

   cStrCpNoWrapBuff(
       pubST->authorsCStr,   // Holds author list
       authorCStr,           // Has the author name to copy
       (unsigned short) lenAuthorUL,
       pubST->padCStr,
       pubST->lenPadUC,
       " and ",              // bibtext uses author1 and 2
       5,                    // Just a command and a space
       &pubST->lenAuthorsLineOnUS,
       pubST->wrapUS,        // Tells when to wrap a line
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenAuthorsStrUL // # chars in authors name
   ); // Add the authors name to the list (already formated correctly)

   return;
} // pubSTAddFullAuthor

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-13 TOC: Sec-01 Sub-01: pubSTAddMesh
   '  - Add an mesh term to a list of mesh terms
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

     if(pubST->wrapUS == 0)
     { // If the user does not want line wrapping
         memcpy( 
            &pubST->meshCStr[pubST->lenMeshStrUL],
             meshCStr,
             lenMeshUL
         ); // Copy the mesh term
 
         pubST->lenMeshStrUL += lenMeshUL;

         memcpy(
            &pubST->meshCStr[pubST->lenMeshStrUL],
            "; ",
            2
         );

         pubST->lenMeshStrUL += 2; // account for "; "

         return;
     } // If the user does not want line wrapping

     cStrCpNoWrapBuff(
         pubST->meshCStr,   // Holds all mesh terms
         meshCStr,          // Has the mesh term to copy
         (unsigned short) lenMeshUL,
         pubST->padCStr,
         pubST->lenPadUC,
         "; ",              // I like , separators
         2,                 // Just a command and a space
         &pubST->lenMeshLineOnUS,// # chars in current line
         pubST->wrapUS,     // Tells when to wrap a line
         pubST->lineBreakCStr,
         pubST->lenBreakUC,
         &pubST->lenMeshStrUL // # chars in mesh term list
     ); // Add the mesh term to the mesh term list

     return;
} // pubSTAddMesh

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-14 TOC: Sec-01 Sub-01: pubSTAddKeyWord
   '  - Add an key word to a list of key words
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->keyWordsCStr[pubST->lenKeyWordsStrUL],
           keyWordCStr,
           lenKeyWordUL
       ); // Copy the key word
 
       pubST->lenKeyWordsStrUL += lenKeyWordUL;

       memcpy(
         &pubST->keyWordsCStr[pubST->lenKeyWordsStrUL],
         ", ",
         2
       );

       pubST->lenKeyWordsStrUL += 2; // account for ", "

       return;
   } // If the user does not want line wrapping

   cStrCpNoWrapBuff(
       pubST->keyWordsCStr, // Holds all key words
       keyWordCStr,         // Has the key word to copy
       (unsigned short) lenKeyWordUL,
       pubST->padCStr,
       pubST->lenPadUC,
       ", ",                // I like , separators
       2,                   // Just a command and a space
       &pubST->lenKeyWordsLineOnUS, // Number chars il line
       pubST->wrapUS,       // Tells when to wrap a line
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenKeyWordsStrUL// # chars in key words list
   ); // Add the key word to the key word list

   return;
} // pubSTAddKeyWord

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-15 TOC: Sec-01 Sub-01: pubSTAddArticleType
   '  - Add an article type to a list of article types
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->articleTypeCStr[
             pubST->lenArticleTypeStrUL
           ],
           articleTypeCStr,
           lenArticleTypeUL
       ); // Copy the article type
 
       pubST->lenArticleTypeStrUL += lenArticleTypeUL;

       memcpy(
         &pubST->articleTypeCStr[
           pubST->lenArticleTypeStrUL
         ],
         ", ",
         2
       );

       pubST->lenArticleTypeStrUL += 2; // account for ", "

       return;
   } // If the user does not want line wrapping

   cStrCpNoWrapBuff(
       pubST->articleTypeCStr, // Holds all types
       articleTypeCStr,    // Has the article type to copy
       (unsigned short) lenArticleTypeUL,
       pubST->padCStr,
       pubST->lenPadUC,
       ", ",                   // I like , separators
       2,                      // Just a , and a space
       &pubST->lenArticleTypeLineOnUS,// # chars in line
       pubST->wrapUS,          // When to break a line
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenArticleTypeStrUL // # chars in type
   ); // Add the article type to the list

   return;
} // pubSTAddArticleType

/*--------------------------------------------------------\
| Output: Adds title (TI) to pubST's title buffer
\--------------------------------------------------------*/
void pubmedSTAddTitle(
    char *titleCStr,           // Title to add
    unsigned long lenTitleUL,  // Length of the title
    struct pubmedStruct *pubST // Has title buffer
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-16 TOC: Sec-01 Sub-01: pubSTAddTitle
   '  - Add an title to a pubmedStruct structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->titleCStr[pubST->lenTitleStrUL],
           titleCStr,
           lenTitleUL
       ); // Copy the title
 
       pubST->lenTitleStrUL += lenTitleUL;
       pubST->titleCStr[pubST->lenTitleStrUL] = '\0';

       return;
   } // If the user does not want line wrapping

   cStrWrapCp(
       pubST->titleCStr,  // Will hold the title
       titleCStr,         // A line in the abstract
       pubST->padCStr,
       pubST->lenPadUC,
       &pubST->lenTitleLineOnUS, // current line length
       pubST->wrapUS,            // Max line length
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenTitleStrUL     // Characters in title
   ); // Copying the title entry

   return;
} // pubSTAddTitle

/*--------------------------------------------------------\
| Output: Adds the journal name to the pubmed entry struct
\--------------------------------------------------------*/
void pubmedSTAddJournal(
    char *journalCStr,           // Journal to add
    unsigned long lenJournalUL,  // Length of the journal
    struct pubmedStruct *pubST // Has journal buffer
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-17 TOC: Sec-01 Sub-01: pubSTAddJournal
   '  - Add an journal name to a pubmedStruct structure
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   if(pubST->wrapUS == 0)
   { // If the user does not want line wrapping
       memcpy( 
           &pubST->journalCStr[pubST->lenJournalStrUL],
           journalCStr,
           lenJournalUL
       ); // Copy the journal
 
       pubST->lenJournalStrUL += lenJournalUL;
       pubST->journalCStr[pubST->lenJournalStrUL] = '\0';

       return;
   } // If the user does not want line wrapping

   cStrWrapCp(
       pubST->journalCStr,  // Will hold the journal
       journalCStr,         // A line in the abstract
       pubST->padCStr,
       pubST->lenPadUC,
       &pubST->lenJournalLineOnUS, // current line length
       pubST->wrapUS,            // Max line length
       pubST->lineBreakCStr,
       pubST->lenBreakUC,
       &pubST->lenJournalStrUL  // # Characters in journal
   ); // Copying the journal entry

   return;
} // pubSTAddJournal


/*--------------------------------------------------------\
| Output:Removes trailing ", " from a pubmedStruct tag list
\--------------------------------------------------------*/
void finshPubmedSTTag(
    struct pubmedStruct *pubST
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-18 TOC: Sec-01 Sub-01: finishPubmedSTTag
   '  - Removes the ", " off the end of a tags list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->tagsCStr[0] == '\0') return; // Nothing to do
    pubST->lenTagsStrUL -= 2;// move  back to start of ", "
    pubST->tagsCStr[pubST->lenTagsStrUL] = '\0';
    return;
} // finishPubmedSTTag

/*--------------------------------------------------------\
| Output: Removes trailing ", " from a key words list
\--------------------------------------------------------*/
void finshPubmedSTKeyWords(
    struct pubmedStruct *pubST // has keyword list
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-19 TOC: Sec-01 Sub-01: finishPubmedSTKeyWords
   '  - Removes the ", " off the end of a key words list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->keyWordsCStr[0] == '\0') return;
    pubST->lenKeyWordsStrUL -= 2;
    pubST->keyWordsCStr[pubST->lenKeyWordsStrUL] = '\0';
    return;
} // finishPubmedSTKeyWords

/*--------------------------------------------------------\
| Output: Removes trailing ", " from a Mesh list
\--------------------------------------------------------*/
void finshPubmedSTMesh(
    struct pubmedStruct *pubST // has mesh term list
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-20 TOC: Sec-01 Sub-01: finishPubmedSTMesh
   '  - Removes the ", " off the end of a mesh list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->meshCStr[0] == '\0') return; // Nothing to do
    pubST->lenMeshStrUL -= 2;// move  back to start of "; "
    pubST->meshCStr[pubST->lenMeshStrUL] = '\0';

    return;
} // finishPubmedSTMesh

/*--------------------------------------------------------\
| Output: Removes trailng ", " from a article type list
\--------------------------------------------------------*/
void finshPubmedSTArticleType(
    struct pubmedStruct *pubST // has article type list
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-21 TOC: Sec-01 Sub-01: finishPubmedSTArticleType
   '  - Removes trailng ", " off an article type list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->articleTypeCStr[0] == '\0') return;
    pubST->lenArticleTypeStrUL -= 2;

    pubST->articleTypeCStr[pubST->lenArticleTypeStrUL] =
      '\0';

    return;
} // finishPubmedSTArticleType

/*--------------------------------------------------------\
| Output: Removes trailing " and " from an author list
\--------------------------------------------------------*/
void finshPubmedSTAuthors(
    struct pubmedStruct *pubST // has author list
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-22 TOC: Sec-01 Sub-01: finishPubmedSTAuthors
   '  - Removes the " and " off the end of the author list
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if(pubST->authorsCStr[0] == '\0') return;
    pubST->lenAuthorsStrUL -= 5;
    pubST->authorsCStr[pubST->lenAuthorsStrUL] = '\0';

    return;
} // finishPubmedSTArticleType

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubSt->padCStr to be newPadCStr
|    o pubSt->lenPadUC to be length of newPadCStr
\--------------------------------------------------------*/
void pubmedSTSetPad(
  char *newPadCStr, // Padding to add to lines
  struct pubmedStruct *pubST // Has padding value to change
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-23 TOC: Sec-01 Sub-01: pubmedSTSetPad
   '  - Set the padding on a pubmedStruct variable
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   strcpy(pubST->padCStr, newPadCStr);
   pubST->lenPadUC = strlen(newPadCStr);
   return;
} // pubmedSTSetPad

/*--------------------------------------------------------\
| Output:
|  - Modifies:
|    o pubSt->lineBreakCStr to hold lineBreakCStr
|    o pubSt->lenBreakUC to be length of lineBreakCStr
\--------------------------------------------------------*/
void pubmedSTSetBreak(
  char *lineBreakCStr,       // New line break
  struct pubmedStruct *pubST // Has line break to change
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
   ' Fun-24 TOC: Sec-01 Sub-01: pubmedSTSetBreak
   '  - Set the line break values in a pubmedStruct
   \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

   strcpy(pubST->lineBreakCStr, lineBreakCStr);
   pubST->lenBreakUC = strlen(lineBreakCStr);
   return;
} // pubmedSTSetBreak

/*--------------------------------------------------------\
| Output: Sets all varialbles in pubOutST to defaults
\--------------------------------------------------------*/
void blankPubOutST(
  struct pubOut *pubOutST // Will be set to default values
){/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
  ' Fun-25 TOC: Sec-01 Sub-01: blankPubOutST
  '  - Sets a pubOut structure to default values
  \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  // These the the more cirtical entries
  pubOutST->citeKeyBl = defPCiteKeyBl;
  pubOutST->journalIDBl = defPJournalIDBl;
  pubOutST->monthBl = defPMonthBl;
  pubOutST->dayBl = defPDayBl;
  pubOutST->volBl = defPVolBl;
  pubOutST->issueBl = defPIssueBl;
  pubOutST->doiBl = defPDoiBl;
  pubOutST->pgBl = defPPgBl;

  // These entires could be uesfull
  pubOutST->editionBl = defPEditionBl;
  pubOutST->pmidBl = defPPmidBl;
  pubOutST->pmcBl = defPPmcBl;
  pubOutST->isbnBl = defPIsbnBl;
  pubOutST->issnBl = defPIssnBl;
  pubOutST->urlBl = defPUrlBl;
  pubOutST->abstractBl = defPAbstractBl;

  // These are addititonal extracted entries
  pubOutST->abvJournalBl = defPAbvJournalBl;
  pubOutST->articleTypeBl = defPArticleTypeBl;
  pubOutST->langBl = defPLangBl;
  pubOutST->meshBl = defPMeshBl;
  pubOutST->keyWordsBl = defPKeyWordsBl;

  // These are my unique entries
  pubOutST->fileNameBl = defPFileNameBl;
  pubOutST->tagsBl = defPTagsBl;
  pubOutST->supBl = defPSupBl;
  pubOutST->notesBl = defPNotesBl;

  return;
} // blankPubOutST

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
){ /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
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

    switch(*langCStr)
    { // Switch: Check the first letter of language code

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-01 Sub-01: Languages that start with A
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'a':
        // Case Is a language that starts with a
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which A language is this
                case 'f':
                // case af
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'r':
                            strcpy(buffCStr, "Afrikaans");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which af language this is
                // case af

                case 'l':
                // case al
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'b':
                            strcpy(buffCStr, "Albanian");
                            *lenLangUC = 8;
                            return;
                        default: return; // Unkown code
                    } // Check which af language this is
                // case al

                case 'm':
                // case am
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'h':
                            strcpy(buffCStr, "Amharic");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which af language this is
                // case am

                case 'r':
                // case ar
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'h':
                            strcpy(buffCStr, "Amharic");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which af language this is
                // case ar

                case 'z':
                // case az
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'e':
                            strcpy(buffCStr, "Azerbaijani");
                            *lenLangUC = 11;
                            return;
                        default: return; // Unkown code
                    } // Check which af language this is
                // case az

                default: return;  // Unkown language code
            } // Switch: check which A language this code is
        // Case Is a language that starts with a

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-02 Sub-01: Languages that start with B
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'b':
        // Case Is a language that starts with b
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which B language this is
                case 'o':
                // case bo
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which bo language this is
                        case 's':
                            strcpy(buffCStr, "Bosnian");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which bo language this is
                // case bo

                case 'u':
                // case bu
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which bu language this is
                        case 'l':
                            strcpy(buffCStr, "Bulgarian");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which bu language this is
                // case bu

                default: return;  // Unkown language code
            } // Switch: check which B language this is
        // Case Is a language that starts with b

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-03 Sub-01: Languages that start with C
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'c':
        // Case Is a language that starts with c
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which C language this is
                case 'a':
                // case ca
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ca language this is
                        case 't':
                            strcpy(buffCStr, "Catalan");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which ca language this is
                // case ca

                case 'h':
                // case ch
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ch language this is
                        case 'i':
                            strcpy(buffCStr, "Chinese");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which ch language this is
                // case ch

                case 'z':
                // case cz
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which cz language this is
                        case 'e':
                            strcpy(buffCStr, "Czech");
                            *lenLangUC = 5;
                            return;
                        default: return; // Unkown code
                    } // Check which cz language this is
                // case cz

                default: return;  // Unkown language code
            } // Switch: check which C language this is
        // Case Is a language that starts with c

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-04 Sub-01: Languages that start with D
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'd':
        // Case Is a language that starts with d
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which D language this is
                case 'a':
                // case da
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which da language this is
                        case 'n':
                            strcpy(buffCStr, "Danish");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which da language this is
                // case da

                case 'u':
                // case du
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which du language this is
                        case 't':
                            strcpy(buffCStr, "Dutch");
                            *lenLangUC = 5;
                            return;
                        default: return; // Unkown code
                    } // Check which du language this is
                // case du

                default: return;  // Unkown language code
            } // Switch: check which D language this is
        // Case Is a language that starts with d

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-05 Sub-01: Languages that start with E
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'e':
        // Case Is a language that starts with e
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which E language this is
                case 'n':
                // case en
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which en language this is
                        case 'g':
                            strcpy(buffCStr, "English");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which en language this is
                // case en

                case 'p':
                // case ep
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ep language this is
                        case 'o':
                            strcpy(buffCStr, "Esperanto");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which ep language this is
                // case ep

                case 's':
                // case es
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which es language this is
                        case 't':
                            strcpy(buffCStr, "Extonian");
                            *lenLangUC = 8;
                            return;
                        default: return; // Unkown code
                    } // Check which es language this is
                // case es

                default: return;  // Unkown language code
            } // Switch: check which E language this is
        // Case Is a language that starts with e

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-06 Sub-01: Languages that start with F
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'f':
        // Case Is a language that starts with f
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which F language this is
                case 'i':
                // case fi
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which fi language this is
                        case 'n':
                            strcpy(buffCStr, "Finnish");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which fi language this is
                // case fi

                case 'r':
                // case fr
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which fr language this is
                        case 't':
                            strcpy(buffCStr, "French");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which fr language this is
                // case fr

                default: return;  // Unkown language code
            } // Switch: check which F language this is
        // Case Is a language that starts with f

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-07 Sub-01: Languages that start with G
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'g':
        // Case Is a language that starts with g
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which G language this is
                case 'e':
                // case ge
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'o':
                            strcpy(buffCStr, "Georgian");
                            *lenLangUC = 8;
                            return;
                        case 'r':
                            strcpy(buffCStr, "German");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which ge language this is
                // case ge

                case 'l':
                // case gl
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which gl language this is
                        case 'a':
                            strcpy(
                              buffCStr,
                              "Scottish Gaelic"
                            );

                            *lenLangUC = 15;
                            return;
                        default: return; // Unkown code
                    } // Check which gl language this is
                // case gl

                case 'r':
                // case gr
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which gr language this is
                        case 'e':
                            strcpy(
                              buffCStr,
                              "Greek, Modern"
                            );

                            *lenLangUC = 13;
                            return;
                        default: return; // Unkown code
                    } // Check which gr language this is
                // case gr

                default: return;  // Unkown language code
            } // Switch: check which G language this is
        // Case Is a language that starts with g

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-08 Sub-01: Languages that start with H
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'h':
        // Case Is a language that starts with h
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which H language this is
                case 'e':
                // case he
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which af language this is
                        case 'b':
                            strcpy(buffCStr, "Hebrew");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which he language this is
                // case he

                case 'i':
                // case hi
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which hi language this is
                        case 'n':
                            strcpy(buffCStr, "Hindi");
                            *lenLangUC = 5;
                            return;
                        default: return; // Unkown code
                    } // Check which hi language this is
                // case hi

                case 'r':
                // case hr
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which hr language this is
                        case 'v':
                            strcpy(buffCStr, "Croatian");
                            *lenLangUC = 8;
                            return;
                        default: return; // Unkown code
                    } // Check which hr language this is
                // case hr

                case 'u':
                // case hu
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which hu language this is
                        case 'n':
                            strcpy(buffCStr, "Hungarian");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which hu language this is
                // case hu

                default: return;  // Unkown language code
            } // Switch: check which H language this is
        // Case Is a language that starts with h

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-09 Sub-01: Languages that start with I
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'i':
        // Case Is a language that starts with i
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which I language this is
                case 'c':
                // case ic
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ic language this is
                        case 'e':
                            strcpy(buffCStr, "Icelandic");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which ic language this is
                // case ic

                case 'n':
                // case in
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which in language this is
                        case 'd':
                            strcpy(buffCStr, "Indonesian");
                            *lenLangUC = 10;
                            return;
                        default: return; // Unkown code
                    } // Check which in language this is
                // case in

                case 't':
                // case it
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which it language this is
                        case 'a':
                            strcpy(buffCStr, "Italian");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which it language this is
                // case it

                default: return;  // Unkown language code
            } // Switch: check which I language this is
        // Case Is a language that starts with i

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-10 Sub-01: Languages that start with J
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'j':
        // Case Is a language that starts with j
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which j language this is
                case 'p':
                // case jp
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which jp language this is
                        case 'n':
                            strcpy(buffCStr, "Japanese");
                            *lenLangUC = 8;
                            return;
                        default: return; // Unkown code
                    } // Check which jp language this is
                // case jp

                default: return;  // Unkown language code
            } // Switch: check which J language this is
        // Case Is a language that starts with j

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-11 Sub-01: Languages that start with K
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'k':
        // Case Is a language that starts with k
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which k language this is
                case 'i':
                // case ki
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ki language this is
                        case 'n':
                            strcpy(
                              buffCStr,
                              "Kinyarwanda"
                            );

                            *lenLangUC = 11;
                            return;
                        default: return; // Unkown code
                    } // Check which ki language this is
                // case ki

                case 'o':
                // case ko
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ko language this is
                        case 'r':
                            strcpy(buffCStr, "Korean");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which ko language this is
                // case ko

                default: return;  // Unkown language code
            } // Switch: check which K language this is
        // Case Is a language that starts with k

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-12 Sub-01: Languages that start with L
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'l':
        // Case Is a language that starts with l
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which l language this is
                case 'a':
                // case la
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which la language this is
                        case 't':
                            strcpy(buffCStr, "Latin");
                            *lenLangUC = 5;
                            return;
                        case 'v':
                            strcpy(buffCStr, "Latvian");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which la language this is
                // case la

                case 'i':
                // case li
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which li language this is
                        case 't':
                            strcpy(buffCStr, "Lithuanian");
                            *lenLangUC = 10;
                            return;
                        default: return; // Unkown code
                    } // Check which li language this is
                // case li

                default: return;  // Unkown language code
            } // Switch: check which L language this is
        // Case Is a language that starts with l

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-13 Sub-01: Languages that start with M
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'm':
        // Case Is a language that starts with m
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which m language this is
                case 'a':
                // case ma
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ma language this is
                        case 'c':
                            strcpy(buffCStr, "Macedonian");
                            *lenLangUC = 10;
                            return;
                        case 'l':
                            strcpy(buffCStr, "Malayalam");
                            *lenLangUC = 9;
                            return;
                        case 'o':
                            strcpy(buffCStr, "Maori");
                            *lenLangUC = 5;
                            return;
                        case 'y':
                            strcpy(buffCStr, "Malay");
                            *lenLangUC = 5;
                            return;
                        default: return; // Unkown code
                    } // Check which ma language this is
                // case ma

                case 'u':
                // case mu
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which mu language this is
                        case 'l':
                            strcpy(
                              buffCStr,
                              "Multiple Languages"
                            );

                            *lenLangUC = 18;
                            return;
                        default: return; // Unkown code
                    } // Check which mu language this is
                // case mu

                default: return;  // Unkown language code
            } // Switch: check which M language this is
        // Case Is a language that starts with m

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-14 Sub-01: Languages that start with N
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'n':
        // Case Is a language that starts with n
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which n language this is
                case 'o':
                // case no
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which no language this is
                        case 'r':
                            strcpy(buffCStr, "Norwegian");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which no language this is
                // case no

                default: return;  // Unkown language code
            } // Switch: check which N language this is
        // Case Is a language that starts with n

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-16 Sub-01: Languages that start with P
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'p':
        // Case Is a language that starts with p
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which p language this is
                case 'e':
                // case pe
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which pe language this is
                        case 'r':
                            strcpy(
                              buffCStr,
                              "Persian, Iranian"
                            );

                            *lenLangUC = 16;
                            return;
                        default: return; // Unkown code
                    } // Check which pe language this is
                // case pe

                case 'o':
                // case po
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which po language this is
                        case 'l':
                            strcpy(buffCStr, "Polish");
                            *lenLangUC = 6;
                            return;

                        case 'r':
                            strcpy(buffCStr, "Portuguese");
                            *lenLangUC = 10;
                            return;
                        default: return; // Unkown code
                    } // Check which po language this is
                // case po

                case 'u':
                // case pu
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which pe language this is
                        case 's':
                            strcpy(buffCStr, "Pushto");
                            *lenLangUC = 6;
                            return;
                        default: return; // Unkown code
                    } // Check which pu language this is
                // case pu

                default: return;  // Unkown language code
            } // Switch: check which P language this is
        // Case Is a language that starts with p

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-18 Sub-01: Languages that start with R
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'r':
        // Case Is a language that starts with r
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which r language this is
                case 'u':
                // case ru
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which ru language this is
                        case 'm':
                          strcpy(
                            buffCStr,
                            "Romanian, Rumanian, Moldovan"
                          );
                          *lenLangUC = 28;
                          return;

                        case 's':
                            strcpy(buffCStr, "Russian");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which ru language this is
                // case r

                default: return;  // Unkown language code
            } // Switch: check which R language this is
        // Case Is a language that starts with r

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-19 Sub-01: Languages that start with S
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 's':
        // Case Is a language that starts with s
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which sa language this is
                case 'a':
                // case sa
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which sa language this is
                        case 'n':
                            strcpy(buffCStr, "Sanskrit");
                            *lenLangUC = 8;
                            return;
                        default: return; // Unkown code
                    } // Check which sa language this is
                // case sa

                case 'l':
                // case sl
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which sl language this is
                        case 'o':
                            strcpy(buffCStr, "Slovak");
                            *lenLangUC = 6;
                            return;
                        case 'v':
                            strcpy(buffCStr, "Slovenian");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which sl language this is
                // case sl

                case 'p':
                // case sp
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which sp language this is
                        case 'a':
                            strcpy(buffCStr, "Spanish");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which sp language this is
                // case sp

                case 'r':
                // case sr
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which sr language this is
                        case 'p':
                            strcpy(buffCStr, "Serbian");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which sr language this is
                // case sr

                case 'w':
                // case sw
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which sw language this is
                        case 'e':
                            strcpy(buffCStr, "Swedish");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which sw language this is
                // case sw

                default: return;  // Unkown language code
            } // Switch: check which S language this is
        // Case Is a language that starts with s

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-20 Sub-01: Languages that start with T
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 't':
        // Case Is a language that starts with t
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which t language this is
                case 'h':
                // case th
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which th language this is
                        case 'a':
                            strcpy(buffCStr, "Thai");
                            *lenLangUC = 4;
                            return;
                        default: return; // Unkown code
                    } // Check which th language this is
                // case th

                case 'u':
                // case tu
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which tu language this is
                        case 'r':
                            strcpy(buffCStr, "Turkish");
                            *lenLangUC = 7;
                            return;
                        default: return; // Unkown code
                    } // Check which tu language this is
                // case tu

                default: return;  // Unkown language code
            } // Switch: check which T language this is
        // Case Is a language that starts with T

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-21 Sub-01: Languages that start with U
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'u':
        // Case Is a language that starts with u
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which u language this is
                case 'k':
                // case uk
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which uk language this is
                        case 'r':
                            strcpy(buffCStr, "Ukrainian");
                            *lenLangUC = 9;
                            return;
                        default: return; // Unkown code
                    } // Check which uk language this is
                // case uk

                case 'n':
                // case un
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which un language this is
                        case 'd':
                            strcpy(
                              buffCStr,
                              "Undetermined"
                            );

                            *lenLangUC = 12;
                            return;
                        default: return; // Unkown code
                    } // Check which un language this is
                // case un

                default: return;  // Unkown language code
            } // Switch: check which U language this is
        // Case Is a language that starts with U

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-22 Sub-01: Languages that start with V
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'v':
        // Case Is a language that starts with v
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which v language this is
                case 'i':
                // case vi
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which uk language this is
                        case 'e':
                            strcpy(buffCStr, "Vietnamese");
                            *lenLangUC = 10;
                            return;
                        default: return; // Unkown code
                    } // Check which ve language this is
                // case ve

                default: return;  // Unkown language code
            } // Switch: check which V language this is
        // Case Is a language that starts with V

        /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\
        ^ Fun-26 Sec-23 Sub-01: Languages that start with W
        \<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case 'w':
        // Case Is a language that starts with w
            ++langCStr;

            switch(*langCStr)
            { // Switch: check which w language this is
                case 'e':
                // case we
                    ++langCStr;
                    switch(*langCStr)
                    { // Check which we language this is
                        case 'l':
                            strcpy(buffCStr, "Welsh");
                            *lenLangUC = 5;
                            return;
                        default: return; // Unkown code
                    } // Check which we language this is
                // case we

                default: return;  // Unkown language code
            } // Switch: check which W language this is
        // Case Is a language that starts with W

        default: return; // Unkown language code
    } // Switch: Check the first letter of the language 

    strcpy(buffCStr, "Unknown");
    *lenLangUC = 7;

    return;
} // pubmedLang

/*PUBMED language table
afr 	Afrikaans
alb 	Albanian
amh 	Amharic
ara 	Arabic
arm 	Armenian
aze 	Azerbaijani
bos 	Bosnian
bul 	Bulgarian
cat 	Catalan
chi 	Chinese
cze 	Czech
dan 	Danish
dut 	Dutch
eng 	English
epo 	Esperanto
est 	Estonian
fin 	Finnish
fre 	French
geo 	Georgian
ger 	German
gla 	Scottish Gaelic
gre 	Greek, Modern
heb 	Hebrew
hin 	Hindi
hrv 	Croatian
hun 	Hungarian
ice 	Icelandic
ind 	Indonesian
ita 	Italian
jpn 	Japanese
kin 	Kinyarwanda
kor 	Korean
lat 	Latin
lav 	Latvian
lit 	Lithuanian
mac 	Macedonian
mal 	Malayalam
mao 	Maori
may 	Malay
mul 	Multiple languages
nor 	Norwegian
per 	Persian, Iranian
pol 	Polish
por 	Portuguese
pus 	Pushto
rum 	Romanian, Rumanian, Moldovan
rus 	Russian
san 	Sanskrit
slo 	Slovak
slv 	Slovenian
spa 	Spanish
srp 	Serbian
swe 	Swedish
tha 	Thai
tur 	Turkish
ukr 	Ukrainian
und 	Undetermined
vie 	Vietnamese
wel 	Welsh
*/

