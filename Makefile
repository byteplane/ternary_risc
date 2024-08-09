CC = gcc
IDIR = ../include
CFLAGS = -I$(IDIR) -O3

LIBS = -lSDL2

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

sdl_test: sdl_test.o
	$(CC) -o sdl_test sdl_test.o $(CFLAGS) $(LIBS)

