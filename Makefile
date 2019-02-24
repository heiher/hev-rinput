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

BUILDMSG="\e[1;31mBUILD\e[0m %s\n"
LINKMSG="\e[1;34mLINK\e[0m  \e[1;32m%s\e[0m\n"
STRIPMSG="\e[1;34mSTRIP\e[0m \e[1;32m%s\e[0m\n"
CLEANMSG="\e[1;34mCLEAN\e[0m %s\n"

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
	$(ECHO_PREFIX) $(RM) -rf $(BINDIR) $(BUILDDIR)
	@printf $(CLEANMSG) $(PROJECT)

$(TARGET) : $(LDOBJS) $(THIRDPART_TARGETS)
	$(ECHO_PREFIX) mkdir -p $(dir $@)
	$(ECHO_PREFIX) $(CC) -o $@ $(LDOBJS) $(LDFLAGS)
	@printf $(LINKMSG) $@
	$(ECHO_PREFIX) $(STRIP) $@
	@printf $(STRIPMSG) $@

$(BUILDDIR)/%.dep : $(SRCDIR)/%.c
	$(ECHO_PREFIX) mkdir -p $(dir $@)
	$(ECHO_PREFIX) $(PP) $(CCFLAGS) -MM -MT$(@:.dep=.o) -MF$@ $< 2> /dev/null

$(BUILDDIR)/%.o : $(SRCDIR)/%.c
	$(ECHO_PREFIX) mkdir -p $(dir $@)
	$(ECHO_PREFIX) $(CC) $(CCFLAGS) -c -o $@ $<
	@printf $(BUILDMSG) $<

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPEND)
endif
