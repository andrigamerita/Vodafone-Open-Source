# Pluto Makefile
# Copyright (C) 2001  Michael Richardson
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# RCSID $Id: Makefile,v 1.2 2003/11/30 14:44:26 derry Exp $



include ../../../Makefile.inc



FMANDIR=$(MANTREE)/man5
PMANDIR=$(MANTREE)/man8

FREESWANSRCDIR=../../..
FREESWANLIBDIR=${FREESWANSRCDIR}/lib
FREESWANINCLS= -I$(FREESWANLIBDIR) -I${FREESWANSRCDIR}
FREESWANLIB=$(FREESWANLIBDIR)/libfreeswan.a

JMK_CFLAGS = -g -Wall -W -Wmissing-prototypes -Wpointer-arith -Wbad-function-cast \
	-Wcast-qual -Wmissing-declarations -Wwrite-strings 
JMK_CFLAGS+= -Wstrict-prototypes 
JMK_CFLAGS+= -Werror
#JMK_CFLAGS+= -Wundef

HDRDIRS =  $(FREESWANINCLS)

CPPFLAGS = $(HDRDIRS) $(BYTE_ORDER) -DDEBUG 

ALLFLAGS = $(CPPFLAGS) $(JMK_CFLAGS)

LIBSWHACK =

BINNAMEPING  = ikeping

RM = /bin/rm
RMFLAGS = -f

.SUFFIXES:
.SUFFIXES: .c .o

# files for a (source) distribution

OBJSPING = ikeping.o $(FREESWANLIB)

all: $(BINNAMEPING)

install: all
	$(INSTALL) $(INSTBINFLAGS) $(BINNAMEPING) $(BINDIR)
	$(INSTALL) $(INSTMANFLAGS) ikeping.8 $(PMANDIR)/ipsec_ikeping.8

$(BINNAMEPING): $(OBJSPING)
	$(CC) -o $(BINNAMEPING) $(JMK_LDFLAGS) $(OBJSPING) $(LIBSPING)

distlist:
	@echo $(DIST)

# Exuberant Ctags doesn't work if LC_ALL is set to something other than C

CTAGSFLAGS = -N --format=1 # fishy options required for Exuberant Ctags

tags:	$(DISTSRC)
	LC_ALL=C ctags $(CTAGSFLAGS) $(DISTSRC)

cleanall: clean

distclean: clean

mostlyclean: clean

realclean: clean

clean:
	$(RM) $(RMFLAGS) $(OBJSPING) *.core core *~ a.out ktrace.out
	$(RM) $(RMFLAGS) $(BINNAMEPING) 

check:
	echo no checks in lib right now.

.c.o:
	$(CC) $(COPTS) $(ALLFLAGS) -c $<

# These rules are not for production use
$(FREESWANLIB):
	cd $(FREESWANLIBDIR) ; $(MAKE)

programs: ikeping

checkprograms: ikeping

check: 


ikeping.o: ${FREESWANSRCDIR}/pluto/constants.h
ikeping.o: ${FREESWANSRCDIR}/pluto/packet.h
ikeping.o: ${FREESWANSRCDIR}/lib/freeswan.h
