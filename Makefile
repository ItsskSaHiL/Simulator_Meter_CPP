all: main

CXX = clang++
override CXXFLAGS += -g -Wall -Werror

SRCS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.cpp' -print | sed -e 's/ /\\ /g')
HEADERS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.h' -print)

main: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o "$@"

main-debug: $(SRCS) $(HEADERS)
	NIX_HARDENING_ENABLE= $(CXX) $(CXXFLAGS) -O0  $(SRCS) -o "$@"

clean:
	rm -f main main-debug
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
QTFLAGS = $(shell pkg-config --cflags Qt5Widgets Qt5Core Qt5Gui)
QTLIBS = $(shell pkg-config --libs Qt5Widgets Qt5Core Qt5Gui)

SRCDIR = .
SOURCES = main.cpp simulator_core.cpp mcu_emulator.cpp metering_engine.cpp protocol_handler.cpp
HEADERS = simulator_core.h mcu_emulator.h metering_engine.h protocol_handler.h

OBJECTS = $(SOURCES:.cpp=.o)
TARGET = smart_meter_simulator

.PHONY: all clean debug install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(QTLIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) -c $< -o $@

main.o: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) main.moc

debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

run: $(TARGET)
	./$(TARGET)

# Handle Qt MOC processing
main.moc: main.cpp
	moc -o main.moc main.cpp

# Development utilities
format:
	clang-format -i *.cpp *.h

lint:
	cppcheck --enable=all --std=c++17 *.cpp *.h

help:
	@echo "Available targets:"
	@echo "  all     - Build the simulator (default)"
	@echo "  clean   - Remove build files"
	@echo "  debug   - Build with debug information"
	@echo "  run     - Build and run the simulator"
	@echo "  install - Install to /usr/local/bin"
	@echo "  format  - Format code with clang-format"
	@echo "  lint    - Run static analysis with cppcheck"
