CC=gcc
RM=rm -f

#INCLUDE = -I/cygdrive/c/MinGW/include
CFLAGS += -Wall $(INCLUDE) -DTRUE=1 -DFALSE=0 -DBUSPIRATE -O2

TARGET = tjtag

OBJ = buspirate.o serial.o tjtag.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(TARGET)

tjtag: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

clean:
	$(RM) $(OBJ) $(TARGET)
