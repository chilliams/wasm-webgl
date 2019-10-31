CPPFLAGS=$(shell sdl2-config --cflags) $(EXTRA_CPPFLAGS)
LDLIBS=$(shell sdl2-config --libs) -lGLEW $(EXTRA_LDLIBS)
EXTRA_LDLIBS?=-lGL

all: triangle

triangle: ../common-sdl2/shader_utils.o

html:
	mkdir -p public
	emcc -s USE_SDL=2 triangle.c ../common-sdl2/shader_utils.c -o public/triangle.html

clean:
	rm -rf public
	rm -f *.o ../common-sdl2/*.o triangle

.PHONY: all html clean
