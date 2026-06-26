// MODIFIED FROM EXAMPLE AT https://docs.opencv.org/4.1.1/d1/de6/samples_2cpp_2tutorial_code_2ImgTrans_2houghcircles_8cpp-example.html

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;
int main(int argc, char** argv)
{
    // Connect to camera stream
    VideoCapture cap(0);
    if(!cap.isOpened()) {
        cout << "Can't open camera stream" << endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    Mat src;

    while(true){
        // Check if image is loaded fine
        cap >> src;
        if(src.empty())
            break;

        Mat gray;
        cvtColor(src, gray, COLOR_BGR2GRAY);
        medianBlur(gray, gray, 5);
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                    gray.rows/16,  // change this value to detect circles with different distances to each other
                    100, 30, 1, 30 // change the last two parameters
                // (min_radius & max_radius) to detect larger circles
        );
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Vec3i c = circles[i];
            Point center = Point(c[0], c[1]);
            // circle center
            circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);
            // circle outline
            int radius = c[2];
            circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
        }
        imshow("detected circles", src);

        // when esc, then break
        if (waitKey(1) == 27) {
            break;
        }
    }
    
    return 0;
}