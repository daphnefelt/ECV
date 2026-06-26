// Code inspired by diff-interactive-updated/capture.cpp by Sam Siewert

#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    string videoPath = "Dark-Room-Laser-Spot-with-Clutter.mpeg";

    VideoCapture vcap(videoPath);
    if (!vcap.isOpened()) {
        cout << "Can't find the video" << endl;
        return -1;
    }

    Mat frame, frame_prev, diff;
    vector<Mat> ch_cur(3), ch_prev(3), ch_diff(3); // R, G, and B for each
    vcap.read(frame_prev);

    while (1){
        if (!vcap.read(frame)) break;

        // split both frames into RGB and get diffs
        split(frame, ch_cur);
        split(frame_prev, ch_prev);
        absdiff(ch_cur[0], ch_prev[0], ch_diff[0]); // B
        absdiff(ch_cur[1], ch_prev[1], ch_diff[1]); // G
        absdiff(ch_cur[2], ch_prev[2], ch_diff[2]); // R

        merge(ch_diff, diff); // puts back into a single img

        // for the display on my Jetson Nano, the image was too big. Shrinking it down
        Mat frame_small, diff_small;
        resize(frame, frame_small, Size(), 0.4, 0.4);
        resize(diff, diff_small, Size(), 0.4, 0.4);
        imshow("Original", frame_small);
        imshow("Frame Difference", diff_small);

        // Save any frames with big R diffs
        Mat side_by_side;
        hconcat(frame_small, diff_small, side_by_side);
        double r_diff_sum = sum(ch_diff[2])[0];
        // if (r_diff_sum > 100000){ // get good examples
        if (r_diff_sum < 50000){ // get bad examples
            string filename = "diff_" + to_string(r_diff_sum) + ".jpg";
            imwrite(filename, side_by_side);
        }

        char c = cv::waitKey(100); // sample rate
        if( c == 'q' ) break;

        frame_prev = frame.clone();
    }
}