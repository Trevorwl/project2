CC = gcc
CFLAGS = -Wall -Werror -MMD -MP -D_XOPEN_SOURCE=700 -Wno-deprecated-declarations

lib := libuthread.a

SRCS = queue.c context.c uthread.c preempt.c sem.c settings.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

all: $(lib)


$(lib): $(OBJS)
	ar rcs $(lib) $(OBJS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.d $(lib)

-include $(DEPS)

