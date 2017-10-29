ODIR = obj
EXECNAME = game

#DEPS = 
_OBJ = main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

CC = g++
CFLAGS = -Wall -std=c++11 -O3 `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_image

# make objects. '$@' = left of ':', '$^' = first item on left of ':'
$(ODIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

# link objects into executable '$^' = right side of ':'
$(EXECNAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(EXECNAME) $(ODIR)/*.o
