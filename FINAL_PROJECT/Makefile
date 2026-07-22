CXX = g++
CXXFLAGS = -std=c++17 -Wall  -I/usr/include/opencv4 -O2 -lpthread
OPENCV_FLAGS = `pkg-config --cflags --libs opencv4`
SRC = generate_path.cpp
OUT = generate_path

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC) $(OPENCV_FLAGS)

clean:
	rm -f $(OUT)