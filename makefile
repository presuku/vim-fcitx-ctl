TARGET=plugin/fcitx-ctl.so

CC=gcc

ifdef DEBUG
DEBUG_FLAG = -g3 -ggdb3 -DDEBUG
  ifdef USE_ASAN
  ASAN_CFLAGS = -fsanitize=address -fno-omit-frame-pointer
  ASAN_LFLAGS = -lasan
  endif
else
DEBUG_FLAG =
endif

CFLAGS   = $(DEBUG_FLAG) $(ASAN_CFLAGS) -Wall -fPIC
CFLAGS  += $(shell pkg-config --cflags dbus-1 fcitx fcitx-utils)
LDFLAGS  = $(ASAN_LFLAGS)
LDFLAGS += $(shell pkg-config --libs dbus-1 fcitx fcitx-utils)
LDFLAGS_SO = -shared -Wl,-ldl

.PHONY: all archive clean

all: $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

$(TARGET): plugin/fcitx-ctl.o
	$(CC) plugin/fcitx-ctl.o $(LDFLAGS) $(LDFLAGS_SO) -o $(TARGET)

archive:
	rm -rf vim-fcitx-ctl
	mkdir -p vim-fcitx-ctl
	mkdir -p vim-fcitx-ctl/plugin
	cp Makefile README.md vim-fcitx-ctl
	cp plugin/fcitx-ctl.c plugin/fcitx-ctl.vim vim-fcitx-ctl/plugin
	zip -r vim-fcitx-ctl.zip vim-fcitx-ctl

clean:
	$(RM) -rf $(TARGET) plugin/fcitx-ctl.o vim-fcitx-ctl vim-fcitx-ctl.zip

