TARGET=rap
CFLAGS=-Wall -g -O2
LDLIBS=

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f $(TARGET) $(TARGET).o
