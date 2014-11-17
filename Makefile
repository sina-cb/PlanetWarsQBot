CC=g++
CFLAGS=-c -Wall -I/usr/include/boost
LIBS=-lboost_thread 
LDFLAGS=-L/usr/lib/ $(LIBS)
SOURCES=QBot.cc PlanetWars.cc Colony.cpp CentralGovernment.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=QBot.bin

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

debug: CC += -DDEBUG -g
debug: $(SOURCES) $(EXECUTABLE)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o QBot
