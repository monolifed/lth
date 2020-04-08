CFLAGS:= -Wall -Wextra -pedantic -std=c11 -O2 -DUSE_GLFW3 -D_LARGEFILE64_SOURCE=1
#CFLAGS= -std=c11 -O2 -DUSE_GLFW3

OBJDIR:=obj/
APPNAME:=LastTrain
APPDIR:=build/

LIBS:=-lglfw -lEGL -lGLESv2 -lm -lopenal
OBJS:=app.o audio.o entity.o font.o gfx.o lighting.o \
	loader.o main.o sprite.o str.o things.o

OBJS:=$(addprefix $(OBJDIR), $(OBJS))
APPNAME:=$(APPDIR)$(APPNAME)

default: all

all: $(APPNAME)

clean:
	$(RM) $(OBJS) $(APPNAME)

$(APPNAME): $(OBJS) | $(APPDIR)
	$(CC) $(CFLAGS) -o $(APPNAME) $(OBJS) $(LIBS)

$(APPDIR):
	mkdir -p $(APPDIR)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
