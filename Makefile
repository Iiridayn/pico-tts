CC = gcc # C compiler
CFLAGS ::= -Wall -Wextra -O2 -g $(CFLAGS) # Make sure our defaults can be overridden
SVOXFLAGS = -fPIC -Wno-unused -Wno-implicit-fallthrough -Wno-uninitialized -include stdint.h
TARGET_LIB = libsvoxpico.so # target lib
LIB_DIR = svox/pico/lib

all: ${TARGET_LIB} pico-tts

SRCS=$(wildcard $(LIB_DIR)/*.c)
OBJS=$(SRCS:.c=.o)

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< > $@
include $(SRCS:.c=.d)

$(OBJS): %.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SVOXFLAGS) -c -o $@ $^

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -shared -o $@ $^ -lm

dev: pico-tts.c $(TARGET_LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -Wl,-rpath=. -o $@ $^ -I $(LIB_DIR) -L. -l svoxpico -lm

pico-tts: pico-tts.c $(TARGET_LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DNDEBUG -o $@ $^ $(LDFLAGS) -I $(LIB_DIR) -L. -l svoxpico -lm

install: pico-tts
	install -D -s -t $(DESTDIR)/usr/lib/ ${TARGET_LIB}
	install -D -s -t $(DESTDIR)/usr/bin/ pico-tts
	install -D -m 0644 -t $(DESTDIR)/usr/share/pico-tts svox/pico/lang/*

clean:
	$(RM) ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) dev pico-tts
