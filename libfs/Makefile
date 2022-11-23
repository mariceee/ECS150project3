# Target library
lib := libfs.a

all: $(lib)

## TODO: Phase 1
objs:= disk.o fs.o

CC:= gcc
AR :=ar
CFLAGS := -Wall -Wextra -Werror -pipe -MMD

ifneq ($(V), 1)
Q= @
endif

deps:=$(patsubst %.o, %.d, $(objs))
-include $(deps)

$(lib): $(objs)
	@echo "AR $@"
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CC $@"
	$(Q)rm -rvf $(libfs) *.a *.o *.d

.PHONY: clean
.PRECIOUS: $(objs)