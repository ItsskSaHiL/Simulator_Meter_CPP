all: $(TARGET)

CXX = g++
CXXFLAGS = -g -Wall -std=c++17 $(shell pkg-config --cflags Qt5Widgets Qt5Gui Qt5Core)
LDFLAGS = $(shell pkg-config --libs Qt5Widgets Qt5Gui Qt5Core)

SOURCES = main.cpp simulator_core.cpp mcu_emulator.cpp metering_engine.cpp protocol_handler.cpp component_library.cpp property_editor.cpp measurement_tools.cpp extended_mcu_support.cpp
HEADERS = simulator_core.h mcu_emulator.h metering_engine.h protocol_handler.h component_library.h property_editor.h measurement_tools.h extended_mcu_support.h
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = smart_meter_simulator

.PHONY: all clean debug install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

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