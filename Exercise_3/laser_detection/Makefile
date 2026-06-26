CXX = g++
CXXFLAGS = -std=c++11 -Wall  -I/usr/include/opencv4
OPENCV_FLAGS = `pkg-config --cflags --libs opencv4`
SRC1 = diff.cpp
SRC2 = red_band.cpp
SRC3 = COM_tracker.cpp
OUT1 = diff
OUT2 = red_band
OUT3 = COM_tracker

all: $(OUT1) $(OUT2) $(OUT3)

$(OUT1): $(SRC1)
	$(CXX) $(CXXFLAGS) -o $(OUT1) $(SRC1) $(OPENCV_FLAGS)

$(OUT2): $(SRC2)
	$(CXX) $(CXXFLAGS) -o $(OUT2) $(SRC2) $(OPENCV_FLAGS)

$(OUT3): $(SRC3)
	$(CXX) $(CXXFLAGS) -o $(OUT3) $(SRC3) $(OPENCV_FLAGS)

clean:
	rm -f $(OUT1) $(OUT2) $(OUT3)