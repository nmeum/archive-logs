CFLAGS ?= -O0 -g -Werror
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500
CFLAGS += -Wpedantic -Wall -Wextra \
	      -Wmissing-prototypes -Wpointer-arith \
	      -Wstrict-prototypes -Wshadow -Wformat-nonliteral

ifeq ($(HAVE_SENDFILE),1)
	CPPFLAGS += -DHAVE_SENDFILE
else
	HEADERS = compat/sendfile.h
endif

archive-logs: archive-logs.c $(HEADERS)
check: archive-logs
	@./tests/run_tests.sh

.PHONY: check
