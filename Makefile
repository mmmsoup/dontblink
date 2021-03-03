all:
	gcc -o dontblink \
	main.c args.c setroot.c util.c xlistener.c \
	-lX11 -lpthread \
	`imlib2-config --cflags` `imlib2-config --libs`
