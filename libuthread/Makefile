CC = gcc
CFLAGS = -Wall -Werror -MMD -MP -D_XOPEN_SOURCE=700 -Wno-deprecated-declarations

# Target library 
lib := libuthread.a

OBJDIR = obj
#preempt.c sem.c 
SRCS = queue.c context.c uthread.c preempt.c sem.c settings.c
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Default target 
all: $(lib)

# Rule to build the target library
$(lib): $(OBJS)
	ar rcs $(lib) $(OBJS)

# Compile source files into object files
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Make sure object directory exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Cleanup
clean:
	rm -rf $(OBJDIR) $(lib)

# Include dependency files
-include $(DEPS)
