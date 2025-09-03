# Compiler
CXX = g++
CXXFLAGS = -Wall -g
LDFLAGS = -lreadline

# Target executable
TARGET = myshell

# Source and object files
SRCS = main.cpp back-foreground.cpp cd.cpp pinfo.cpp redirection.cpp search.cpp autocomplete.cpp history.cpp ls.cpp pipe.cpp
OBJS = $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET)

# Link objects to create executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)
