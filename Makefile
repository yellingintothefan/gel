# Compiler and standard.
CC = gcc -std=c99

# Project name.
PROJ = gel

# Source files.
SRCS =main.c
SRCS+=Display.c
SRCS+=Face.c
SRCS+=Faces.c
SRCS+=Line.c
SRCS+=Sdl.c
SRCS+=Triangle.c
SRCS+=Vertex.c
SRCS+=Vertices.c
SRCS+=util.c

# Warnings flags.
CFLAGS = -Wshadow -Wall -Wpedantic -Wextra

# Debugging flags.
CFLAGS+= -g

# Optimization flags.
CFLAGS+= -Ofast -flto -march=native -fsingle-precision-constant
CFLAGS+= -fassociative-math -freciprocal-math -fno-signed-zeros
CFLAGS+= -frename-registers -funroll-loops -fno-trapping-math

# Linker flags.
LDFLAGS = -lm -lSDL2

# Linker.
$(PROJ): $(SRCS:.c=.o)
	$(CC) $(CFLAGS) $(SRCS:.c=.o) $(LDFLAGS) -o $(PROJ)

# Compiler template; generates dependency targets.
%.o : %.c
	$(CC) $(CFLAGS) -MMD -MP -MT $@ -MF $*.td -c $<
	@mv -f $*.td $*.d

# All dependency targets.
%.d: ;
-include *.d

clean:
	rm -f vgcore.*
	rm -f cachegrind.out.*
	rm -f callgrind.out.*
	rm -f $(PROJ)
	rm -f $(SRCS:.c=.o)
	rm -f $(SRCS:.c=.d)
