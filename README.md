# Use:

pubmedToBib converts a pubmed reference file to a bibtex
  file.

I made this code to satisfy my need, but am posting it here
  in case someone else finds it useful (there are plenty of
  alternatives out there, but I am picky about my bibtex
  format).

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

The .pubmed file can have multiple articles, so long as
  each articles entry is separated by a blank line. See
  test/test-case.pubmed for an example.

PubMed files can be downloaded from a pubmed search by
  clicking save (under search box)-> format -> 
  selecting the PubMed format -> create file.

## Running:

The help message for pubmedToBib can be printed with 
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
  pubmedToBib-settings.h. To enable an option change
  defPXBl, were x is the entry to enable to 1. To disable
  an option change defPXBl, were x is the entry to disable
  to 0 (these entries are on lines 14 to 43). This will
  mess up the help message a bit, but works. The -bib-help
  message is printed out by Fun-06 in pubmedToBib.c
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

```
pubmedToBib -pubmed file.pubmed > file.bib

# Show the help message
pubmedToBib -h | less

# Show the message to change entries printed to bibtex
pubmedToBib -bib-help | less
```

## Building:

PubmedToBib depends on unac to remove accents from the
  first authors last name when building the citation key
  for the bibtex file. This feature can be removed by
  running make normaccent. However, this means that the
  citation key will only use the year and pubmed id.

During the build both unac and libiconv-1.9.1, which unac
  depends on are downloaded using curl and git. 
  Problematic lines (version number function) in the
  unac.c/h files are removed with sed. The unac.c/h files
  and libraries form libiconv are then used to build a
  static install of pubmedToBib.

You can change the gcc complied used with make by using
  make CC=newGCC.

Dependencies:

- unac (Downloaded during installation)
  - [https://github.com/QuickDict/unac](https://github.com/QuickDict/unac)
  - Removed with make normaccent
- libconiv-1.9.1 (Download during installion)
  - [ftp://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.9.1.tar.gz](ftp://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.9.1.tar.gz)
  - Removed with make normaccent
- Curl: Used to download libiconv-1.9.1
  - Removed with make normaccent
- Git: Used to download unac
  - Removed with make normaccent
- Sed: To remove the version number function from unac.c/h
  - Removed with make normaccent
- Modern gcc compiler

```
# Linux

make
# make normaccent # for a no dependecy build
make clean        # remove the libiconv install files
cp pubmedToBib /path/to/install
chmod a+x /path/to/install/pubmedToBib
```

```
# openbsd

make CC=egcc  # cc is a bit to old for libiconv & unac
# make normaccent # for a no dependecy build
make clean
cp pubmedToBib /path/to/install
chmod a+x /path/to/install/pubmedToBib
```
