CFLAGS:= -Wall -Wextra -pedantic -std=c11 -O2 -D_LARGEFILE64_SOURCE=1
#CFLAGS= -std=c11 -O2 -DUSE_GLFW3

OBJDIR:=obj
APPNAME:=LastTrain
APPDIR:=build

OBJS:=app.o audio.o entity.o font.o gfx.o lighting.o \
	loader.o main.o sprite.o str.o things.o

OBJS:=$(addprefix $(OBJDIR)/, $(OBJS))
APPNAME:=$(APPDIR)/$(APPNAME)

ifeq ($(OS),Windows_NT)
	RM:=del /Q
	APPNAME:=$(APPNAME).exe
	CC:=gcc
	MAKEDIR:=mkdir
	CFLAGS:=$(CFLAGS) -I. -Llibs
	CLEAN:=util.bat CLEAN
	POST:=util.bat POST
	LIBS:=-lglfw3 -lopengl32 -lopenal32 -luser32 -lgdi32 -lm
else
	MAKEDIR:=mkdir -p
	CLEAN:=$(RM) $(OBJS) $(APPNAME)
	LIBS:=-lglfw -lGL -lopenal -lm
endif



default: all

all: $(APPNAME)

clean:
	$(CLEAN)

$(APPNAME): $(OBJS) | $(APPDIR)
	$(CC) $(CFLAGS) -o $(APPNAME) $(OBJS) $(LIBS)
	$(POST)

$(APPDIR):
	$(MAKEDIR) $(APPDIR)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	$(MAKEDIR) $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
