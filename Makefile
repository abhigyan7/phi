#
# Compiler flags
#
CC     = gcc
CFLAGS = -Wall -Werror -Wextra -ledit

#
# Project files
#
SRCS = arithmetics.c expressions.c lenv.c lval.c main.c mpc.c semantics.c builtins.c bool.c ordering.c
OBJS = $(SRCS:.c=.o)
EXE  = lisp

#
# Debug build settings
#
DBGDIR = bin/debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#
RELDIR = bin/release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O3 -DNDEBUG

.PHONY: all clean debug prep release remake

# Default build
all: prep release

#
# Debug rules
#
debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJS)
	$(CC) $(CFLAGS) $(DBGCFLAGS) -o $(DBGEXE) $^

$(DBGDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBGCFLAGS) -o $@ $<

#
# Release rules
#
release: $(RELEXE)

$(RELEXE): $(RELOBJS)
	$(CC) $(CFLAGS) $(RELCFLAGS) -o $(RELEXE) $^

$(RELDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELCFLAGS) -o $@ $<

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR) $(RELDIR)

remake: clean all

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS)

