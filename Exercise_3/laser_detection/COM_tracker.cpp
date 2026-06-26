// To encode all these frames to MPEG4:
// ffmpeg -r 30 -i vidframes_tracked/frame%04d.ppm Dark-Room-Laser-Spot-tracked.mpeg

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <limits>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    string videoPath = "Dark-Room-Laser-Spot-REDONLY.mpeg";
    int THRESH = 235; // Based on GIMP histogram
    string outDir = "vidframes_tracked";

    VideoCapture vcap(videoPath);
    if (!vcap.isOpened()) {
        cout << "Can't open video" << endl;
        return -1;
    }

    Mat frame;
    int framecnt = 0;

    while (vcap.read(frame))
    {
        framecnt++;
        int rows = frame.rows, cols = frame.cols;

        // scan every pixel in img. if above threshold and on edge, update min/max row/col
        int minRow = numeric_limits<int>::max(), maxRow = -1;
        int minCol = numeric_limits<int>::max(), maxCol = -1;
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                uchar val = frame.at<Vec3b>(y, x)[2]; // all vid data is in red
                if (val > THRESH) {
                    if (y < minRow) minRow = y;
                    if (y > maxRow) maxRow = y;
                    if (x < minCol) minCol = x;
                    if (x > maxCol) maxCol = x;
                }
            }
        }

        // COM is avg of extents
        int xbar, ybar;
        bool found = (maxRow >= 0 && maxCol >= 0);
        if (found) {
            xbar = (minCol + maxCol) / 2;
            ybar = (minRow + maxRow) / 2;
        } else {
            xbar = -1; ybar = -1; // idk where spot is
        }

        // crosshair and save ppms
        Mat annotated;
        annotated = frame.clone();

        if (xbar >= 0) {
            line(annotated, Point(0, ybar), Point(annotated.cols - 1, ybar), Scalar(255, 255, 255), 1, LINE_AA);
            line(annotated, Point(xbar, 0), Point(xbar, annotated.rows - 1), Scalar(255, 255, 255), 1, LINE_AA);
        }
        char filename[96];
        sprintf(filename, "%s/frame%04d.ppm", outDir.c_str(), framecnt);
        imwrite(filename, annotated);
    }
    return 0;
}