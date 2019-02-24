# Makefile for hev-rinput

PROJECT=hev-rinput

CROSS_PREFIX :=
PP=$(CROSS_PREFIX)cpp
CC=$(CROSS_PREFIX)gcc
STRIP=$(CROSS_PREFIX)strip
CCFLAGS=-O3 -Wall -Werror \
		-I$(THIRDPARTDIR)/ini-parser/src \
		-I$(THIRDPARTDIR)/hev-task-system/include
LDFLAGS=-L$(THIRDPARTDIR)/ini-parser/bin -lini-parser \
		-L$(THIRDPARTDIR)/hev-task-system/bin -lhev-task-system \
		-lpthread

SRCDIR=src
BINDIR=bin
BUILDDIR=build
THIRDPARTDIR=third-part

TARGET=$(BINDIR)/hev-rinput
THIRDPARTS=$(THIRDPARTDIR)/ini-parser \
	   $(THIRDPARTDIR)/hev-task-system
THIRDPART_TARGETS=$(THIRDPARTDIR)/ini-parser/bin/libini-parser.a \
	   $(THIRDPARTDIR)/hev-task-system/bin/libhev-task-system.a

-include build.mk
CCSRCS=$(filter %.c,$(SRCFILES))
ASSRCS=$(filter %.S,$(SRCFILES))
LDOBJS=$(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(CCSRCS)) \
	   $(patsubst $(SRCDIR)/%.S,$(BUILDDIR)/%.o,$(ASSRCS))
DEPEND=$(LDOBJS:.o=.dep)

BUILDMSG="\e[1;31mBUILD\e[0m $<"
LINKMSG="\e[1;34mLINK\e[0m  \e[1;32m$@\e[0m"
STRIPMSG="\e[1;34mSTRIP\e[0m \e[1;32m$@\e[0m"
CLEANMSG="\e[1;34mCLEAN\e[0m $(PROJECT)"

V :=
ECHO_PREFIX := @
ifeq ($(V),1)
	undefine ECHO_PREFIX
endif

.PHONY: all clean tp-clean

all : $(TARGET)

$(THIRDPART_TARGETS) : $(THIRDPARTS)
	@$(foreach dir,$^,$(MAKE) --no-print-directory -C $(dir);)

tp-clean : $(THIRDPARTS)
	@$(foreach dir,$^,$(MAKE) --no-print-directory -C $(dir) clean;)

clean : tp-clean
	$(ECHO_PREFIX) $(RM) $(BINDIR)/* $(BUILDDIR)/*
	@echo -e $(CLEANMSG)

$(TARGET) : $(LDOBJS) $(THIRDPART_TARGETS)
	$(ECHO_PREFIX) $(CC) -o $@ $(LDOBJS) $(LDFLAGS)
	@echo -e $(LINKMSG)
	$(ECHO_PREFIX) $(STRIP) $@
	@echo -e $(STRIPMSG)

$(BUILDDIR)/%.dep : $(SRCDIR)/%.c
	$(ECHO_PREFIX) mkdir -p $(dir $@)
	$(ECHO_PREFIX) $(PP) $(CCFLAGS) -MM -MT $(@:.dep=.o) -o $@ $<

$(BUILDDIR)/%.o : $(SRCDIR)/%.c
	$(ECHO_PREFIX) mkdir -p $(dir $@)
	$(ECHO_PREFIX) $(CC) $(CCFLAGS) -c -o $@ $<
	@echo -e $(BUILDMSG)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPEND)
endif
