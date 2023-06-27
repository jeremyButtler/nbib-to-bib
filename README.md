# Use:

nbibToBib converts an nbib reference file to a bibtex file.

Note updates I have made are at the bottom of this readme

## Alternatives:

 - bibutils [https://github.com/biodranik/bibutils](https://github.com/biodranik/bibutils)
   - At a glance, this looks like rather robust code, but
     it does not have the same level of flexiability for
     deciding which entries to print out. However, it does
     have a bit more flexibility in controlling if you
     prefere braces or "" and does have some white space
     settings.
   - One upside is that this code was designed to be a
     general conversion tool, so it supports more than just
     bibtex conversion. The downside ofto this is that the
     user needs to run two programs. One to convert the
     nbib to xml and another to conver the xml to a bibtex
     file. Best to make a bash script for this.
 - nbib2bib [https://github.com/thiagonds65/nbib2bib](https://github.com/thiagonds65/nbib2bib)
   - This is a python script which converts all nbib files
     in a directory to bibtex files.
   - This works, but has no flexibility for choosing the
     output format.
   - It uses the doi entry to get the bibtex file, which
     allows it to get the month entry that both nbibToBib
     and bibutils miss for PMID 22078026. However, this
     also causes it to run slowly, 6 seconds for all eight 
     of its test cases. In comparision both nbibToBib and
     bibutils takes less than a second for all eight test
     cases.
 - Finally their are plenty of PMID to bibtex programs
   that can convert a pmid to a bibtex entry for you.
   - For example [https://gist.github.com/tommycarstensen/ec3c57761f3846c339de925b66f4ac1b](https://gist.github.com/tommycarstensen/ec3c57761f3846c339de925b66f4ac1b) took 1 second to output the bibtex
       entries for all of nbib2bib's test cases.
   
## Description:

Entries extracted from the pubmed file are:
  author names (full only), title, journal name, year,
  journal id, month, day, volume, issue, doi,
  page number (non-electronic prioritized), edition, PMID,
  PMC id, ISBN, ISSN (linking/paper prioritized), abstract,
  abbreviated journal name, article type, language,
  mesh terms, and keywords.

The default printed extracted entries are:
  author names (full only), title, journal name, year,
  journal id, month, day, volume, issue, doi,
  page number (non-electronic prioritized), edition, PMID,
  PMC id, ISSN (linking/paper prioritized), abstract,
  and keywords.

Other entries printed include the full doi address as the
  url and some of my own tags
  (file, tags, supplemental, and notesp)

I should note that this extracts the date of publication
  using the DP (date of publication) entry instead of the
  DEP (date of electronic publication) entry. There are
  times were the DP entry will have less information than
  the DEP entry.

The .nbib file can have multiple articles, so long as
  each articles entry is separated by a blank line. See
  test/test-case.nbib for an example. You can merge
  individual nbib files into a single nbib file with new
  lines at the end of each reference using sed.

```
sed '/SO/{a\

};' *.nbib > merged.nbib;
```


Nbib files can be downloaded from a pubmed search by
  save (under search box)->format->selecting the PubMed
  format->create file. The file will be saved as a .txt
  file, but is an .nbib file.

```
I suspect most people using this code will not need this
  diagram, but I included it just in case someone might
  find it useful.

Visual of downloading a .nbiv file from pubmed.

-----------------------------------------------------------
                    Step 1: Select save
-----------------------------------------------------------

    +-----\\
    | NIH  ++ National Library of Medicine
    +-----//  National Center for Biotechnology information
    
           +-------------------------------+
    PubMed | Search term                   |
           +-------------------------------+
           Advanced  Create alert  Creat RSS
    
           +------+
**CLICK**->| Save |
           +------+
    
-----------------------------------------------------------
                    Step 2: Select PubMed (nbib) output
-----------------------------------------------------------

    +-----\\
    | NIH  ++ National Library of Medicine
    +-----//  National Center for Biotechnology information
    
           +-------------------------------+
    PubMed | Search term                   |
           +-------------------------------+
           Advanced  Create alert  Creat RSS
    
           +------+
           | Save |
           +------+
    
           Save citations to file
    
                     +--------------------------+
           Seletion: | All results on this page |
                     +--------------------------+
    
                     +-----------------+
           Format:   | Summary (text)  |
**SELECT**---------->| PubMed          |
                     | PMID            |
                     | Abstract (text) |
                     | CSV             |
                     +-----------------+
    
           +-------------+
           | Create file |
           +-------------+

-----------------------------------------------------------
                    Step 3: Select create file
-----------------------------------------------------------

    +-----\\
    | NIH  ++ National Library of Medicine
    +-----//  National Center for Biotechnology information
    
           +-------------------------------+
    PubMed | Search term                   |
           +-------------------------------+
           Advanced  Create alert  Creat RSS
    
           +------+
           | Save |
           +------+
    
           Save citations to file
    
                     +--------------------------+
           Seletion: | All results on this page |
                     +--------------------------+
    
                     +-----------------+
           Format:   | PubMed          |
                     +-----------------+
    
           +-------------+
**CLICK**->| Create file |
           +-------------+
```

## Building:

NbibToBib depends on unac to remove accents from the
  first authors last name when building the citation key
  for the bibtex file. This feature can be removed by
  running make normaccent. However, this means that the
  citation key will only use the year and pubmed id.

The make file will download both unac and libiconv-1.9.1,
  which unac depends on with curl and git. The unac_version
  function is removed from the unac.c/h files (prevents 
  errors). libiconv and unac are then used to build a
  static build of nbibToBib.

You can change the gcc complier used to build nbibToBi
  with make CC=newGCC. The default is cc.

Dependencies:

- unac (Downloaded during installation)
  - [https://github.com/QuickDict/unac](https://github.com/QuickDict/unac)
  - Removed with make normaccent
- libconiv-1.9.1 (Download during installion)
  - [ftp://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.9.1.tar.gz](ftp://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.9.1.tar.gz)
  - Removed with make normaccent
- unix cmd programs used in build (removed by
  make normaccent)
  - curl: Used to download libiconv-1.9.1
  - git: Used to download unac
  - sed: Removes unac_version function from unac.c/h
  - tar: Extract libiconv files
- Modern gcc compiler

```
# Linux Install

make
make clean        # remove the libiconv install files
cp nbibToBib /path/to/install
chmod a+x /path/to/install/nbibToBib
```

```
# Openbsd Install

make CC=egcc  # cc is a bit to old for libiconv & unac
make clean
cp nbibToBib /path/to/install
chmod a+x /path/to/install/nbibToBib
```

```
# Linux or openbsd no dependency install

make normaccent # for a no dependecy build
cp nbibToBib /path/to/install
chmod a+x /path/to/install/nbibToBib
```

## Running:

```
nbibToBib -nbib file.nbib > file.bib

# Show the help message
nbibToBib -h | less

# Show the message to change entries printed to bibtex
nbibToBib -bib-help | less
```

The help message for nbibToBib can be printed with 
  -h. A further help message showing the fine tuning
  options, which control what is printed out can be called
  with -bib-help.

Each of the parameters printed by -bib-help have two forms.
  The first is the -p form, which sets the entry to be
  printed out. The second is the -no form, which sets the
  entry to not be printed out. However, there are no
  entry parameters for entries required by the bibtex
  format (title, author, journal, and year). These required
  entries are always printed out.

These values can also be changed at compile time be editing
  nbibToBib-settings.h. To enable an option change
  defPXBl, were x is the entry to enable to 1. To disable
  an option change defPXBl, were x is the entry to disable
  to 0 (these entries are on lines 14 to 43). This will
  mess up the help message a bit, but works. The -bib-help
  message is printed out by Fun-06 in nbibToBib.c
  (this is at the bottom of the document).

The output line wrapping of the bibtex file can be set with
  -line-wrap. This variable only affects the title, author,
  journal name, keywords, mesh terms, article type, tags,
  and abstract entries.

The -tag option adds a entry into the tags={}, entry. This
  is my way of adding in key words into the file without
  adding to the authors keywords.

You have two options for how lines end. The unix option
  (-break-unix), which uses a new line only or the
  windows option (-break-win), which uses a carriage return
  and new line. It may be possible that the current windows
  may allow the unix option to be used. Unix can handle
  both options.

The input file is specified by -pubmed and the output file
  is specified by -bib.

## Updates

### June 23rd 2023 (version 20230623)

- Fixed an error were a comma was not put after the
  citation key
  - This error can be corrected with
    ```
    sed 's/^\(@.*={.*\)/\1,/' file.bib > tmp.bib;
    mv tmp.bib file.bib;
    ```
