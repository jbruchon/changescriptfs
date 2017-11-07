CC=gcc
CFLAGS=-O2 -g
#CFLAGS=-O2 -flto -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
#CFLAGS=-Og -g3
BUILD_CFLAGS = -std=gnu99 -I. -D_FILE_OFFSET_BITS=64 -pipe -fstrict-aliasing
#BUILD_CFLAGS += -Wall -Wextra -Wstrict-aliasing -Wcast-align -pedantic -Wno-unused-parameter
BUILD_CFLAGS += -Wall -Wextra -Wwrite-strings -Wcast-align -Wstrict-aliasing -pedantic -Wstrict-overflow -Wstrict-prototypes -Wpointer-arith -Wundef
BUILD_CFLAGS += -Wshadow -Wfloat-equal -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wformat=2 -Winit-self
#LDFLAGS=-s
#LDFLAGS=-flto -s -Wl,--gc-sections
LDFLAGS=
FUSE_CFLAGS=$(shell pkg-config fuse --cflags)
FUSE_LDFLAGS=$(shell pkg-config fuse --libs)
FUSE_LIBS=-lfuse

prefix=/usr
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
datarootdir=${prefix}/share
datadir=${datarootdir}
sysconfdir=${prefix}/etc

OBJS_LIB=jody_string.o
OBJS_MOUNT=changescriptfs.o $(OBJS_LIB)

BUILD_CFLAGS += $(CFLAGS_EXTRA)

all: mount.changescriptfs manual

mount.changescriptfs: $(OBJS_MOUNT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(FUSE_CFLAGS) $(BUILD_CFLAGS) $(FUSE_LDFLAGS) -o mount.changescriptfs $(OBJS_MOUNT) $(FUSE_LIBS)

manual:
	gzip -9 < mount.changescriptfs.8 > mount.changescriptfs.8.gz

.c.o:
	$(CC) -c $(BUILD_CFLAGS) $(FUSE_CFLAGS) $(CFLAGS) $<

clean:
	rm -f *.o *~ mount.changescriptfs debug.log *.?.gz

distclean:
	rm -f *.o *~ mount.changescriptfs debug.log *.?.gz changescriptfs*.pkg.tar.*

install: all
	install -D -o root -g root -m 0644 mount.changescriptfs.8.gz $(DESTDIR)/$(mandir)/man8/mount.changescriptfs.8.gz
	install -D -o root -g root -m 0755 -s mount.changescriptfs $(DESTDIR)/$(bindir)/mount.changescriptfs

package:
	+./chroot_build.sh
