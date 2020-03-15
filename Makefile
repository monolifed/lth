CFLAGS= -Wall -Wextra -pedantic -std=c11 -O2 -DUSE_GLFW3 -D_LARGEFILE64_SOURCE=1
#CFLAGS= -std=c11 -O2 -DUSE_GLFW3

LIBS=-lglfw -lEGL -lGLESv2 -lm -lopenal
OBJS= app.o audio.o entity.o font.o gfx.o lighting.o \
	loader.o main.o sprite.o str.o things.o

default: all

all: LastTrain

clean:
	-$(RM) *.o LastTrain

LastTrain: $(OBJS)
	$(CC) $(CFLAGS) -o LastTrain $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
