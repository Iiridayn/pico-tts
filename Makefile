CC = gcc # C compiler
CFLAGS = -fPIC -Wall -Wextra -Wno-unused -Wno-implicit-fallthrough -O2 -g -include stdint.h # C flags
#CFLAGS += -D_WIN32
LDFLAGS = -shared  # linking flags
TARGET_LIB = libsvoxpico.so # target lib
LIB_DIR = svox/pico/lib

all: ${TARGET_LIB} pico-tts

SRCS=$(wildcard $(LIB_DIR)/*.c)
OBJS=$(SRCS:.c=.o)

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< > $@
include $(SRCS:.c=.d)

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

dev: pico-tts.c
	$(CC) -g -Wall -Wextra -O2 -g -I $(LIB_DIR) -lm -L. -l svoxpico -Wl,-rpath=. -o $@ $^

pico-tts: pico-tts.c
	$(CC) -g -Wall -Wextra -O2 -g -I $(LIB_DIR) -lm -L. -l svoxpico -DNDEBUG -o $@ $^

install: pico-tts
	install -s ${TARGET_LIB} $(DESTDIR)/usr/lib/
	install -s pico-tts $(DESTDIR)/usr/bin/
	install -D -m 0644 -t $(DESTDIR)/usr/share/pico-tts svox/pico/lang/*

clean:
	rm ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) dev pico-tts
