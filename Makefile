CC = gcc
CFLAGS = -g -Og  -lreadline -I./inc
LDFLAGS = -g
SRCS = main.c reader.c env.c mikal_type.c eval.c buildin_func.c gc.c

OBJS = $(SRCS:%.c=%.o)
BINS = $(OBJS:%.o=%)

all: mikal

mikal: $(OBJS)
	$(CC) $(CFLAGS) -o run $(OBJS)

.c.o: $(SRCS)
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f *.o run
	rm checkresult
	rm *.out
