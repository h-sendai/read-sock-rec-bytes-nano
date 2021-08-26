PROG = read-sock-rec-bytes
CFLAGS += -g -O2 -Wall
CFLAGS += -std=gnu99
# CFLAGS += -pthread
# LDLIBS += -L/usr/local/lib -lmylib
# LDFLAGS += -pthread

all: $(PROG)
OBJS += $(PROG).o
OBJS += my_socket.o
OBJS += my_signal.o
OBJS += set_timer.o
OBJS += get_num.o
OBJS += timespecsub.o
$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
