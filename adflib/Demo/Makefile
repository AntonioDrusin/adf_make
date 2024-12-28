# unadf

LIBDIR=../Lib
LIBNAME=libadf.a

DEPEND=makedepend

CFLAGS=-I$(LIBDIR) -O2 -Wall
LDFLAGS=-L$(LIBDIR) -ladf

EXES= unadf


all: $(EXES)

run: $(EXES)
	unadf -h
	unadf -lrcs -v 1 ../../hd.adf

lib:
	cd $(LIBDIR) && $(MAKE)

unadf: lib unadf.o
	$(CC) $(CFLAGS) -o $@ unadf.o $(LDFLAGS)

clean:
	rm *.o $(EXES) core newdev

depend:
	$(DEPEND) -v -- $(CFLAGS) --  *.[ch]


# DO NOT DELETE THIS LINE -- make depend depends on it.

unadf.o: ../Lib/adflib.h
# ../Lib/adflib.h includes:
#	adf_defs.h
#	adf_str.h
unadf.o: ../Lib/adf_defs.h ../Lib/adf_str.h
# ../Lib/adf_str.h includes:
#	stdio.h
#	adf_defs.h
#	adf_blk.h
#	adf_err.h
unadf.o: /usr/include/stdio.h
# /usr/include/stdio.h includes:
#	libio.h
#	stdio_lim.h
unadf.o: /usr/include/libio.h
# /usr/include/libio.h includes:
#	features.h
#	_G_config.h
#	stdarg.h
unadf.o: /usr/include/features.h
# /usr/include/features.h includes:
#	sys/cdefs.h
#	gnu/stubs.h
unadf.o: /usr/include/sys/cdefs.h
# /usr/include/sys/cdefs.h includes:
#	features.h
unadf.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
# /usr/include/_G_config.h includes:
#	gnu/types.h
#	stddef.h
unadf.o: /usr/include/gnu/types.h
# /usr/include/gnu/types.h includes:
#	features.h
unadf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
unadf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
unadf.o: /usr/include/stdio_lim.h ../Lib/adf_blk.h ../Lib/adf_err.h
