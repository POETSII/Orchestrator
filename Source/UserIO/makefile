OBJS = UserIO.o server.o client.o responder.o requester.o
CFLAGS =-O

CC = mpicc

UserIO:		$(OBJS)
		$(CC) $(CFLAGS) -o UserIO $(OBJS) -lm

UserIO.o:	UserIO.c
		$(CC) $(CFLAGS) -c UserIO.c

server.o:	server.c
		$(CC) $(CFLAGS) -c server.c

client.o:	client.c
		$(CC) $(CFLAGS) -c client.c

responder.o:	responder.c
		$(CC) $(CFLAGS) -c responder.c

requester.o:	requester.c
		$(CC) $(CFLAGS) -c requester.c
