CXX = g++
CXXFLAGS = -std=c++11 -Wall  -I/usr/include/opencv4
OPENCV_FLAGS = `pkg-config --cflags --libs opencv4`
SRC = diff.cpp
OUT = diff

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC) $(OPENCV_FLAGS)

clean:
	rm -f $(OUT)