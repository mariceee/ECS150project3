# Target library
lib := libfs.a

MAKEFLAGS+=-rR
all: $(lib)

## TODO: Phase 1
objs:= disk.o fs.o

CC:= gcc

CFLAGS := -Wall -Wextra -Werror -pipe -MMD

deps:=$(patsubst %.o, %.d, $(objs))
-include $(deps)

%.a: $(objs)
	@ ar rcs $@ $^

%.o: %.c
	@ $(CC) $(CFLAGS) -c -o $@ $<

clean:
	@ rm -rvf $(targets) $(objs) $(deps) *.d

.PHONY: clean
.PRECIOUS: $(objs)