CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -Werror -I.
ARFLAGS = rcs
LIBNAME = libuthread.a
OBJS = queue.o uthread.o sem.o context.o preempt.o

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(LIBNAME)
