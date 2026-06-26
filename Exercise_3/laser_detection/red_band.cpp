// Using the red channel because the laser is red
// To encode all these frames to MPEG4:
// ffmpeg -r 30 -i vidframes/frame%04d.pgm Dark-Room-Laser-Spot-REDONLY.mpeg

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    string videoPath = "Dark-Room-Laser-Spot.mpeg";

    VideoCapture vcap(videoPath);
    if (!vcap.isOpened()) {
        cout << "Can't find video" << endl;
        return -1;
    }

    double fps = vcap.get(CAP_PROP_FPS); // Needed to know this to know how to encode the new video
    cout << "frame rate: " << fps << " fps" << endl;

    Mat frame;
    vector<Mat> ch(3);
    char filename[64];
    int framecnt = 0;

    while (vcap.read(frame))
    {
        framecnt++;

        split(frame, ch);
        Mat grayscale_img = ch[2]; // only the red channel

        sprintf(filename, "vidframes/frame%04d.pgm", framecnt); // format for ffmpeg, stored in folder
        imwrite(filename, grayscale_img); // write out the PGM
    }
    return 0;
}