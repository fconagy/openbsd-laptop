# Make file implementing a sequence of login methods.

# Compiler switches.
CSWITCH=-Wall -Wno-unused-function -g

# Libraries.
LIB=-lutil

# Auth executables.
BINDIR=/usr/libexec/auth

# Manual pages.
MANDIR=$(HOME)/local/man/man8
MANPAGE=$(MANDIR)/login_seq.8

# Local auth style is with minus starting the name.
# But we use a name without the minus anyway.
SEQEXE=$(BINDIR)/login_seq
ECHOEXE=$(BINDIR)/login_echo

# Default target.
all: login_echo login_seq userok

# Test, show environment.
login_echo: login_echo.c
	cc $(CSWITCH) -o login_echo login_echo.c $(LIB)

# Sequence authenticator.
login_seq: login_seq.c
	cc $(CSWITCH) -o login_seq login_seq.c $(LIB)

# Test program.
userok: userok.c
	cc $(CSWITCH) -o userok userok.c $(LIB)

dirs:
	mkdir -p $(MANDIR)

# Install into auth directory.
install: dirs login_echo login_seq
	cp -p login_seq $(SEQEXE)
	chown root:auth $(SEQEXE)
	chmod 4555 $(SEQEXE)
	#cp -p login_echo $(ECHOEXE)
	#chown root:auth $(ECHOEXE)
	#chmod 4555 $(ECHOEXE)
	cp -p login_seq.man $(MANPAGE)

# Remove.
uninstall:
	rm $(SEQEXE)
	#rm $(ECHOEXE)
	rm $(MANPAGE)

# Cleanup.
clean:
	rm -f login_seq
	rm -f login_echo
	rm -f userok
	rm -f *.core

distclean: clean

# Create save tarball.
save: distclean
	mkdir -p $(HOME)/save/seq; \
	cd ..; \
	tar -z -cf $(HOME)/save/seq/seq.tar.gz ./seq

