CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 \
	-I./include \
	-I./src/core \
	-I./src/platform \
	-I./src/ui
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

TARGET = Tetriste
SRCS = src/main.cpp \
	src/core/Game.cpp \
	src/core/Piece.cpp \
	src/platform/AssetManager.cpp \
	src/platform/UserManager.cpp \
	src/ui/Renderer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) \
		src/main.o \
		src/core/*.o \
		src/platform/*.o \
		src/ui/*.o

.PHONY: all clean
