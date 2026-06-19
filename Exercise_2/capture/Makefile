CXX = g++
CXXFLAGS = -std=c++11 -Wall  -I/usr/include/opencv4
OPENCV_FLAGS = `pkg-config --cflags --libs opencv4`
SRC1 = capture_timed.cpp
SRC2 = edge_detection.cpp
OUT1 = capture_timed
OUT2 = edge_detection

all: $(OUT1) $(OUT2)

$(OUT1): $(SRC1)
	$(CXX) $(CXXFLAGS) -o $(OUT1) $(SRC1) $(OPENCV_FLAGS)

$(OUT2): $(SRC2)
	$(CXX) $(CXXFLAGS) -o $(OUT2) $(SRC2) $(OPENCV_FLAGS)

clean:
	rm -f $(OUT1) $(OUT2)