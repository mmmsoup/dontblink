dontblink: dontblink.c
	gcc -o dontblink dontblink.c -lX11 -lpthread `imlib2-config --cflags` `imlib2-config --libs`
