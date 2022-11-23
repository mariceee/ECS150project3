# Target library
lib := libfs.a

MAKEFLAGS+=-rR
all: $(lib)

## TODO: Phase 1
objs:= disk.o fs.o

CC:= gcc

CFLAGS := -Wall -Wextra -Werror -pipe -MMD

ifneq ($(V), 1)
Q= @
endif

deps:=$(patsubst %.o, %.d, $(objs))
-include $(deps)

%.a: $(objs)
	$(Q)ar rcs $@ $^

%.o: %.c
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(Q)rm -rvf $(targets) $(objs) $(deps) *.d

.PHONY: clean
.PRECIOUS: $(objs)