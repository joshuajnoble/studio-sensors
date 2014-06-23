LIB=-L
INC=-I

C=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=studio_sensor.cpp spi.cpp pi_mmio.c common_dht_read.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LIB) $(INC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
