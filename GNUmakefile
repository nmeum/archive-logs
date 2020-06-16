PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man
DOCDIR ?= $(PREFIX)/share/doc/archive-logs

CFLAGS ?= -O0 -g -Werror
CFLAGS += -std=c99
CFLAGS += -Wpedantic -Wall -Wextra \
	      -Wmissing-prototypes -Wpointer-arith \
	      -Wstrict-prototypes -Wshadow -Wformat-nonliteral

# glibc needs _XOPEN_SOURCE for nftw(3)
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500

ifeq ($(HAVE_SENDFILE),1)
	CPPFLAGS += -DHAVE_SENDFILE
else
	HEADERS = compat/sendfile.h
endif

archive-logs: archive-logs.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< -o $@
check: archive-logs
	@./tests/run_tests.sh

install: archive-logs archive-logs.1 README.md
	install -Dm755 archive-logs "$(DESTDIR)$(BINDIR)/archive-logs"
	install -Dm644 archive-logs.1 "$(DESTDIR)$(MANDIR)/man1/archive-logs"
	install -Dm644 README.md "$(DESTDIR)$(DOCDIR)/README.md"

.PHONY: check install
