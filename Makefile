CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

TARGET = Tetriste
SRCS = main.cpp Game.cpp Piece.cpp AssetManager.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
