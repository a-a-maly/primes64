CXX=g++
CXXFLAGS=-O3 -march=native -Wall -W
LD=g++
LDFLAGS=-O3 -march=native -Wall -W -static

HEADERS=primes.h bitset.h
SOURCES=main.cpp primes.cpp pprime.cpp
OBJECTS=$(SOURCES:%.cpp=%.o)
TARGET=primes64

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJECTS)

%.o: primes.h
primes.o: bitset.h

.PHONY: all clean
