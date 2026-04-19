# ================================================================
# Makefile - Bot Bahasa Indonesia Berbasis Aturan
# C89 + POSIX + SQLite3
# ================================================================

CC      = gcc
CFLAGS  = -std=c89 -Wall -Wextra \
          -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lsqlite3 -lm
TARGET  = bot
OBJS    = utama.o basisdata.o bahasa.o logika.o keluaran.o
HEADERS = bot.h

# ================================================================
# Aturan Build
# ================================================================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) bot.db

run: $(TARGET)
	./$(TARGET)

# import datasheet ke database
import: $(TARGET) datasheet_bot.sql
	sqlite3 bot.db < datasheet_bot.sql

# jalankan bot dengan database terisi
demo: import run

.PHONY: all clean run import demo
