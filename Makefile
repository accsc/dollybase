#
# Top-level Makefile for dollybase
#
# Usage:
#   ./configure          Detect build environment
#   make                 Build library + interpreter
#   make clean           Remove all build artifacts
#   make test            Run library regression tests
#   make install         Install library headers + interpreter
#

-include config.mk

# Defaults if configure has not been run
ifndef CC
  CC = gcc
endif
ifndef CFLAGS
  CFLAGS = -w
endif
ifndef NCURSES_LIBS
  NCURSES_LIBS = -lncurses
endif
ifndef PREFIX
  PREFIX = /usr/local
endif

# Directories
LIBDIR = libdbase_4
INTDIR = int

# Library sources (matching libdbase_4/Makefile)
# Note: imports/common.c is flattened to .libs/common.o
LIB_SRCS = str_funcs.c index.c low.c recs.c deletes.c seeks.c appends.c \
           mate.c creates.c printer.c export.c memofields.c locker.c \
           labels.c relations.c

LIB_SRCS_FULL = $(addprefix $(LIBDIR)/,$(LIB_SRCS))
LIB_SRCS_FULL += $(LIBDIR)/imports/common.c

LIB_OBJS = $(addprefix $(LIBDIR)/.libs/,$(LIB_SRCS:.c=.o))
LIB_STATIC = $(LIBDIR)/.libs/libdbase_0.4_s.a

# Interpreter sources
INT_SRCS = $(INTDIR)/prg.c $(INTDIR)/tokenizer.c $(INTDIR)/parser.c \
           $(INTDIR)/executor.c $(INTDIR)/exprvalue.c $(INTDIR)/variables.c \
           $(INTDIR)/workarea.c $(INTDIR)/ui.c

INT_OBJS = $(patsubst %.c,%.o,$(INT_SRCS))

# Link flags for the interpreter (ncurses is required)
INT_LIBS = $(LIB_STATIC) -lm $(NCURSES_LIBS) -lmenu -lform

# --- Targets ---

.PHONY: all clean test install lib int

all: lib int
	@echo ""
	@echo "Build complete. Run './int/prg' to start the interpreter."

lib: $(LIB_STATIC)

$(LIB_STATIC): $(LIB_OBJS)
	@ar q $@ $^
	@ranlib $@

# Compile library sources — include path points to libdir for libdbase.h
$(LIBDIR)/.libs/%.o: $(LIBDIR)/%.c
	@mkdir -p $(LIBDIR)/.libs
	$(CC) $(CFLAGS) -fPIC -I$(LIBDIR) -c $< -o $@

# Special rule for imports/common.c (nested path, flattened to .libs/common.o)
$(LIBDIR)/.libs/common.o: $(LIBDIR)/imports/common.c
	@mkdir -p $(LIBDIR)/.libs
	$(CC) $(CFLAGS) -fPIC -I$(LIBDIR) -c $< -o $@

int: $(INTDIR)/prg

$(INTDIR)/prg: $(INT_OBJS) $(LIB_STATIC)
	$(CC) $(LDFLAGS) $^ $(INT_LIBS) -o $@

# Compile interpreter sources — include libdir for libdbase.h
%.o: $(INTDIR)/%.c
	$(CC) $(CFLAGS) -I$(LIBDIR) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(LIB_STATIC)
	rm -f $(INT_OBJS) $(INTDIR)/prg $(INTDIR)/simple_use $(INTDIR)/test_prg $(INTDIR)/test_prg_dbg
	rm -f config.mk

test: all
	$(MAKE) -C $(LIBDIR) test

install: all
	@mkdir -p $(PREFIX)/include $(PREFIX)/bin
	install -m 644 $(LIBDIR)/libdbase.h $(PREFIX)/include/
	install -m 755 $(INTDIR)/prg $(PREFIX)/bin/prg
