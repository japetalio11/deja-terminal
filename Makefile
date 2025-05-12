CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lncurses

SRCS = main.c title_screen.c network.c
OBJS = $(SRCS:.c=.o)
TARGET = deadly_escape

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule for object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
