ODIR = obj
EXECNAME = game_c

#DEPS = 
_OBJ = main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

CC = gcc
CFLAGS = -Wall -O2 `sdl2-config --cflags`
#CFLAGS = -Wall -g `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

# make objects. '$@' = left of ':', '$^' = first item on left of ':'
$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

# link objects into executable '$^' = right side of ':'
$(EXECNAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(EXECNAME) $(ODIR)/*.o
