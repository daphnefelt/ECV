CXX = g++
CXXFLAGS = -std=c++11 -Wall  -I/usr/include/opencv4
OPENCV_FLAGS = `pkg-config --cflags --libs opencv4`
SRC1 = diff.cpp
SRC2 = red_band.cpp
OUT1 = diff
OUT2 = red_band

all: $(OUT1) $(OUT2)

$(OUT1): $(SRC1)
	$(CXX) $(CXXFLAGS) -o $(OUT1) $(SRC1) $(OPENCV_FLAGS)

$(OUT2): $(SRC2)
	$(CXX) $(CXXFLAGS) -o $(OUT2) $(SRC2) $(OPENCV_FLAGS)

clean:
	rm -f $(OUT1) $(OUT2)