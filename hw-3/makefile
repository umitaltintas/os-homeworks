# Variables
CXX=g++
CXXFLAGS=-std=c++17 -I include
TARGET1=fileSystemOperation
TARGET2=makeFileSystem
SOURCES1=src/fileSystemOperation.cpp src/FileSystem.cpp
SOURCES2=src/makeFileSystem.cpp src/FileSystem.cpp
OBJECTS1=$(SOURCES1:.cpp=.o)
OBJECTS2=$(SOURCES2:.cpp=.o)

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJECTS1)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET2): $(OBJECTS2)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f src/*.o $(TARGET1) $(TARGET2)
