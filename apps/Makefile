# Target programs
programs := uthread_yield.x uthread_hello.x sem_simple.x sem_prime.x sem_count.x queue_tester.x queue_tester_example.x basic_preemption_with_semaphores.x intermediate_preemption_with_semaphores.x

# User-level thread library
UTHREADLIB := libuthread
UTHREADPATH := ../$(UTHREADLIB)
libuthread := $(UTHREADPATH)/$(UTHREADLIB).a

# Object and executable directories
OBJDIR := obj
EXECDIR := executables

# Executables paths
execs := $(patsubst %.x,$(EXECDIR)/%.x,$(programs))

# Default rule
all: $(execs)

# Avoid builtin rules and variables
MAKEFLAGS += -rR

# Don't print the commands unless explicitly requested with `make V=1`
ifneq ($(V),1)
Q = @
V = 0
endif

# Current directory
CUR_PWD := $(shell pwd)

# Define compilation toolchain
CC	= gcc

# General gcc options
CFLAGS	:= -Wall -Wextra -Werror -D_XOPEN_SOURCE=700 -Wno-deprecated-declarations
CFLAGS	+= -pipe

## Debug flag
ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

## Include path
CFLAGS 	+= -I$(UTHREADPATH)

## Dependency generation
CFLAGS	+= -MMD

# Linker options
LDFLAGS := -L$(UTHREADPATH) -luthread


# Ensure directories exist
$(shell mkdir -p $(OBJDIR) $(EXECDIR))

# Application objects to compile
objs := $(patsubst %.x,$(OBJDIR)/%.o,$(programs))


# Include dependencies
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

# Rule for libuthread.a
$(libuthread): FORCE
	@echo \"MAKE\t$@\"
	$(Q)$(MAKE) V=$(V) D=$(D) -C $(UTHREADPATH)

# Generic rule for linking final applications
$(EXECDIR)/%.x: $(OBJDIR)/%.o $(libuthread)
	@echo \"LD\t$@\"
	$(Q)$(CC) -o $@ $< $(LDFLAGS)

# Generic rule for compiling objects
$(OBJDIR)/%.o: %.c
	@echo \"CC\t$@\"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# Cleaning rule
clean: FORCE
	@echo \"CLEAN\t$(CUR_PWD)\"
	$(Q)$(MAKE) V=$(V) D=$(D) -C $(UTHREADPATH) clean
	$(Q)rm -rf $(OBJDIR) $(EXECDIR)

# Keep object files around
.PRECIOUS: $(OBJDIR)/%.o
.PHONY: FORCE
FORCE:

queueTest:
	./$(EXECDIR)/queue_tester.x

yieldTest:
	./$(EXECDIR)/uthread_yield.x

bufferTest:
	rm -f bufferOutput.txt
	touch bufferOutput.txt
	./$(EXECDIR)/sem_buffer.x > bufferOutput.txt 2>&1
	cat bufferOutput.txt
	rm -f bufferOutput.txt
	
countTest:
	./$(EXECDIR)/sem_count.x
	
primeTest:
	rm -f primeOutput.txt
	touch primeOutput.txt
	./$(EXECDIR)/sem_prime.x > primeOutput.txt 2>&1
	cat primeOutput.txt
	rm -f primeOutput.txt

basicPreemptSem:
	./$(EXECDIR)/basic_preemption_with_semaphores.x

interPreemptSem:
	./$(EXECDIR)/intermediate_preemption_with_semaphores.x



