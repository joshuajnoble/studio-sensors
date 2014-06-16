LIB=-L/usr/local/lib
INC=-I/usr/local/include

C=g++
CFLAGS=-c -Wall
LDFLAGS=lbcm2835
SOURCES=studio_sensor.cpp spi.cpp bcm2835.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LIB) $(INC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
