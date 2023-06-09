CC = g++
CFLAGS = -O2 -g -Wall -std=c++20

SOURCES = main.cpp Movies.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = ratemovies

:$(TARGET)

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) -lncurses

clean:
	rm -rvf $(OBJECTS)