# remove -g when done debuging
CC=cc

LDFLAGS=\
    -L/usr/local/lib\
    -L/usr/lib32\
    -L/usr/lib64\
    -L/usr/lib
  # Some good defaults for library paths on unix OS's
  # /usr/local/lib is openbse, while the others are linux

CFLAGS=\
    -Wall\
	-O3
CPPFLAGS=\
    -I/usr/local/include\
    -I/usr/include
    # These are just some good basic defaults
    # This is here so the user can easily change this
    # without changin the other flags
    # (/usr/local/include is openbsd, other is linux)

# libraries used (as flags)
LDLIBS=#-liconv
    #-lcurl
    #-luac
# NCBI detects and refuses conections for curl

# My gcc command (except for libraries)
gccArgs=\
    cStrFun.c\
    cnvtPubmedToBib.c\
    pubmedToBib.c\
    -o pubmedToBib

all: getLibiconvStatic getUnac
	# the first install works for debain, while the 
	# second works for openbsd
	$(CC)\
        -static\
        -I.\
        -L.\
        $(CFLAGS)\
        unac.c\
        $(gccArgs) ||\
      $(CC)\
        -static\
        -Iinclude\
        -Llib\
        -liconv\
        $(CFLAGS)\
        unac.c\
        $(gccArgs)

make normaccent:
	$(CC)\
      -static\
      -DNORMACCENT\
      $(CFLAGS)\
      $(gccArgs)

clean:
	printf "Removing complied files from static libiconv\n"
	rm -r lib include share bin

# Get a static build of libiconv
getLibiconvStatic:
	printf "Downloading iconv for unac\n"
	# Check if libiconv already exits, if not download
	ls libiconv-1.9.1.tar.gz ||\
      curl\
        --output libiconv-1.9.1.tar.gz\
   ftp://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.9.1.tar.gz
	tar -xzf libiconv-1.9.1.tar.gz
	printf "Configuring libiconv\n"
	ls lib || (cd libiconv-1.9.1 &&\
      ./configure\
        CC=$(CC)\
        --with-iconv=yes\
        --enable-static\
        --disable-rpath\
        --prefix=$$(printf "%s/.." $$(pwd))\
        --exec-prefix=$$(printf "%s/.." $$(pwd)))
	ls lib || make -C libiconv-1.9.1 ||\
      rm -r -f libiconv-1.9.1
	ls lib || make -C libiconv-1.9.1 install &&\
      rm -r -f libiconv-1.9.1
	#rm libiconv-1.9.1.tar.gz
		# no longer need

getUnac:
	printf "Downloading unac\n"
	ls unac.c ||\
      ls unac ||\
      git clone https://github.com/QuickDict/unac
	printf "Copying needed files from unac\n"
	ls unac.c || cp unac/unac.c ./
	ls unac.h || cp unac/unac.h ./
	rm -r -f unac
	# remove problematic lines
	sed '/unac_version/,/}/d;' unac.c > tmp.c
	sed '/unac_version(void)/d;' unac.h > tmp.h
	mv tmp.c unac.c
	mv tmp.h unac.h


# This is here if hte user has installed unac
linkunac:
	$(CC)\
      -DLINKUNAC\
      -lunac\
      $(LDFLAGS)\
      $(CFLAGS)\
      $(CPPFLAGS)\
      $(LDLIBS)\
      $(gccArgs)

# These were past install variations
## if the user had unac installed
#
## In this version the user provided the unac.c/h files
#linkincov:
#	$(CC)\
#      $(LDFLAGS)\
#      -liconv\
#      $(CFLAGS)\
#      $(CPPFLAGS)\
#      $(LDLIBS)\
#      unac.c\
#      $(gccArgs)
#
#pdf:
#	$(CC)\
#      -DPDF\
#      -lcurl\
#      -liconv\
#      $(LDFLAGS)\
#      $(CFLAGS)\
#      $(CPPFLAGS)\
#      $(LDLIBS)\
#      unac.c\
#      getPMCPdf.c\
#      $(gccArgs)
#
## This was my static biuld using curl. It needed https
## support, which means tracking additional dependencies
##staticPdf: getLibiconv getUnac getCurl
#	#$(CC) -static -Iinclude -Llib $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) $(LDLIBS) $(gccArgs)
#
## This is to get arounc unac issues for openbsd
#sourceUNAC: getLibiconv getUnac
#	printf "Building pubmedToBib\n"
#
#getCurl:
#	env AUTOMAKE_VERSION=1.16 AUTOCONV_VERSION=2.71 autoreconf-2.71 -f -i
#	# Need to figure out library for https: suspect nghttp2
#	# --enable-ares was suggested for static build
#	#--disable-pthreads
#	./configure\
#        CC=$(CC)\
#        --without-brotli\
#        --without-hyper\
#        --without-libgsasl\
#        --without-libidn2\
#        --without-libpsl\
#        --without-libssh\
#        --without-libssh2\
#        --without-zstd\
#        --prefix=$$(printf "%s/.." $$(pwd))\
#        --enable-http\
#        --enable-ftp\
#        --disable-ldap\
#        --without-nghttp2\
#        --enable-static\
#        --disable-shared\
#        --without-ssl\
#        --disable-imap\
#        --disable-smtp\
#        --without-zlib\
#        --enable-file\
#        --exec-prefix=$$(printf "%s/.." $$(pwd))
#
