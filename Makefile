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

dev: pico-tts.c $(TARGET_LIB)
	$(CC) -g -Wall -Wextra -O2 -g -Wl,-rpath=. -o $@ $^ -I $(LIB_DIR) -L. -l svoxpico -lm

pico-tts: pico-tts.c $(TARGET_LIB)
	$(CC) -g -Wall -Wextra -O2 -g -DNDEBUG -o $@ $^ -I $(LIB_DIR) -L. -l svoxpico -lm

install: pico-tts
	install -D -s -t $(DESTDIR)/usr/lib/ ${TARGET_LIB}
	install -D -s -t $(DESTDIR)/usr/bin/ pico-tts
	install -D -m 0644 -t $(DESTDIR)/usr/share/pico-tts svox/pico/lang/*

clean:
	rm ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) dev pico-tts
