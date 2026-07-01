// MODIFIED FROM SKELETAL.CPP BY SAM SIEWART
// Changed to allow for continuous video feed
// Also saved jpg frames
// To encode: ffmpeg -r 30 -i frames2/frame_%04d.jpg -b:v 10M Finger_Detection.mpeg

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    Mat gray, binary, mfblur;
    VideoCapture cap(0);
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    if (!cap.isOpened()) {
        cout << "Can't open camera\n";
        return -1;
    }

    Mat src;
    int frameCount = 0;
    while (cap.read(src)) {
        if(src.empty())
        {
            cout << "can not open frame\n";
            return -1;
        }

        // show original source image
        imshow("source", src);

        cvtColor(src, gray, cv::COLOR_BGR2GRAY);

        // show graymap of source image
        imshow("graymap", gray);

        threshold(gray, binary, 57, 255, cv::THRESH_BINARY);
        // binary = 255 - binary;

        // show bitmap of source image
        imshow("binary", binary);

        // To remove median filter, just replace blurr value with 1
        medianBlur(binary, mfblur, 3);

        // show median blur filter of source image
        imshow("mfblur", mfblur);

        // This section of code was adapted from the following post, which was
        // based in turn on the Wikipedia description of a morphological skeleton
        //
        // http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
        //
        Mat skel(mfblur.size(), CV_8UC1, Scalar(0));
        Mat temp;
        Mat eroded;
        Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));
        bool done;
        int iterations=0;

        do
        {
        erode(mfblur, eroded, element);
        dilate(eroded, temp, element);
        subtract(mfblur, temp, temp);
        bitwise_or(skel, temp, skel);
        eroded.copyTo(mfblur);

        done = (countNonZero(mfblur) == 0);
        iterations++;

        } while (!done && (iterations < 100));

        // cout << "iterations=" << iterations << endl;

        imshow("skeleton", skel);

        // save frame
        // Convert gray images to 3 channels for display in color grid
        Mat gray_color, binary_color, skel_color;
        cvtColor(gray, gray_color, cv::COLOR_GRAY2BGR);
        cvtColor(binary, binary_color, cv::COLOR_GRAY2BGR);
        cvtColor(skel, skel_color, cv::COLOR_GRAY2BGR);
        // Arrange in 2x2 grid
        Mat top, bottom, grid;
        hconcat(src, gray_color, top);
        hconcat(binary_color, skel_color, bottom);
        vconcat(top, bottom, grid);
        char filename[64];
        sprintf(filename, "frames2/frame_%04d.jpg", frameCount++);
        imwrite(filename, grid);

        if (waitKey(30) == 27) break; // Wait 30ms or exit
    }
    return 0;
}
