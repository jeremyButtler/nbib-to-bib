#include <stdio.h>

#define defVersion 20230623 // Started 20230412
#define defGetPdf 0   // Download pdf if found a PMC id
                      // Pubmed blocks curl; does not work
#define defLineWrap 59 // Default line wrapping limit

// The variables to adjust the padding size and type
// (PAD_C_STR & LEN_PAD_UC) can be found at the top of
// cnvtPubmedToBib.c

// Printing settings
// Recomended
#define defPCiteKeyBl 1   // 1: Print out citation key
#define defPJournalIDBl 1 // 1: Print out the journal id
#define defPMonthBl 1     // 1: Print out publication month
#define defPDayBl 1       // 1: Print out publication day
#define defPVolBl 1       // 1: Print out volume number
#define defPIssueBl 1     // 1: Print out issue number
#define defPDoiBl 1       // 1: Print out doi number
#define defPPgBl 1        // 1: Print out page number

//  These might be helpfull
#define defPEditionBl 1   // 1: Print out the edition
#define defPPmidBl 1      // 1: Print out the pubmed id
#define defPPmcBl 1       //.1: Print pubmed centeral id
#define defPIsbnBl 0      // 1: Print out the isbn number
#define defPIssnBl 1      // 1: Print out the issn number
#define defPUrlBl 1       // 1:Print url; full doi address
#define defPAbstractBl 1  // 1: Print out abstract

// These are .pubmed entires
#define defPAbvJournalBl 0// 1 :Print abbrivated journal
#define defPArticleTypeBl 0 // 1: Print out article types
#define defPLangBl 0      // 1: Print out the language
#define defPMeshBl 0      // 1: Print out the mesh terms
#define defPKeyWordsBl 1  // 1: Print authors key words

// These are unqiue to my system
#define defPFileNameBl 1  // 1: Print generated file name
#define defPTagsBl 1      // 1: Print out tags
#define defPSupBl 1       // 1: Print out supplement tag
#define defPNotesBl 1     // 1: Print out the notes tag
