// MODIFIED FROM EXAMPLE AT https://docs.opencv.org/4.1.1/d9/db0/tutorial_hough_lines.html

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <string>
using namespace cv;
using namespace std;

void detectLines(const Mat& src) {
    Mat gray, dst, cdst, cdstP;
    if (src.channels() == 3) // make gray first
        cvtColor(src, gray, COLOR_BGR2GRAY);
    else
        gray = src;

    // Edge detection
    Canny(src, dst, 50, 200, 3);
    // Copy edges to the images that will display the results in BGR
    cvtColor(dst, cdst, COLOR_GRAY2BGR);
    cdstP = cdst.clone();
    // Standard Hough Line Transform
    vector<Vec2f> lines; // will hold the results of the detection
    HoughLines(dst, lines, 1, CV_PI/180, 200, 0, 0); // runs the actual detection
    // Draw the lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( cdst, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
    }
    // Probabilistic Line Transform
    vector<Vec4i> linesP; // will hold the results of the detection
    HoughLinesP(dst, linesP, 1, CV_PI/180, 100, 200, 10 ); // runs the actual detection
    // Draw the lines
    for( size_t i = 0; i < linesP.size(); i++ )
    {
        Vec4i l = linesP[i];
        line( cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
    }
    // Show results
    imshow("Source", src);
    // imshow("Hough Line Transform", cdst);
    imshow("Probabilistic Line Transform", cdstP);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "Usage:\n"
             << argv[0] << " --video <file>\n"
             << argv[0] << " --camera <index>\n";
        return 1;
    }

    string mode = argv[1];
    VideoCapture cap;
    if (mode == "--video") {
        cap.open(argv[2]);
    } else if (mode == "--camera") {
        int camIdx = atoi(argv[2]);
        cap.open(camIdx);
        cap.set(CAP_PROP_FRAME_WIDTH, 640);
        cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    } else {
        cout << "Not a known mode, sorry\n";
        return 1;
    }

    if (!cap.isOpened()) {
        cout << "Can't open video file or camera\n";
        return -1;
    }
    Mat frame;
    while (cap.read(frame)) {
        detectLines(frame);
        if (waitKey(1) == 27) break; // ESC to exit
    }
    return 0;
}