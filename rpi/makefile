LIB=-L
INC=-I

CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lrt
SOURCES=spi.cpp common_dht_read.c studio_sensor.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=studio_sensor

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LIB) $(INC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
