// Based on edgecam.cpp example by Sam Siewert 
// Modified to make just one window, where 'n' is for None
// Also added in my fps calculations from capture_timed.cpp

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;


// See www.asciitable.com
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

Mat edge_frame, timg_gray, timg_grad;
Mat frame;

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;
const int kernel_size = 3;
const char* window_name = "Edge Viewer";


void CannyThreshold(int, void*)
{
    cvtColor(frame, timg_gray, COLOR_BGR2GRAY);

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, edge_frame, Size(3,3) );

    /// Canny detector
    Canny( edge_frame, edge_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all(0);

    frame.copyTo( timg_grad, edge_frame);

    imshow( window_name, timg_grad );

}


void SobelThreshold(int, void*)
{
  // Generate grad_x and grad_y
  Mat grad_x, grad_y;
  Mat abs_grad_x, abs_grad_y;
  Mat src, src_gray;
  int ddepth = CV_16S;
  int scale = 1;
  Mat grad;
  int delta = lowThreshold;

  cvtColor(frame, timg_gray, COLOR_BGR2GRAY);

  /// Reduce noise with a kernel 3x3
  blur( timg_gray, edge_frame, Size(3,3) );

  /// Gradient X
  //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
  Sobel( timg_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_x, abs_grad_x );

  /// Gradient Y
  //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
  Sobel( timg_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_y, abs_grad_y );

  /// Total Gradient (approximate)
  addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, timg_grad );

  imshow( window_name, timg_grad );
}



int main( int argc, char** argv )
{
   VideoCapture cam0(0);
   char winInput;
   struct timespec curr_t;
   double curr_time, start_time, prev_time;

   clock_gettime(CLOCK_MONOTONIC, &curr_t);
   curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
   start_time=curr_time;
   prev_time=curr_time;
   int frameCnt=0;
   float fps=0.0;
   
   // 1 for Canny, 2 for Sobel, 0 for None
   int mode=0;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

   namedWindow( window_name, WINDOW_AUTOSIZE );
   createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, nullptr );


   while (1)
   {
      clock_gettime(CLOCK_MONOTONIC, &curr_t);
      prev_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;

      cam0.read(frame);

      // fps calculation
      frameCnt++;
      if (frameCnt % 150 == 0) // every 150 frames is around 5s
      {
        fps = 150.0 / (prev_time-start_time);
        start_time=prev_time;
      }

      std::string fps_str = std::to_string(fps) + " FPS";
      putText(frame, fps_str, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(163,63,136), 2);

      switch(mode){
        case 1: CannyThreshold(0, 0); break;
        case 2: SobelThreshold(0, 0); break;
        case 0: imshow( window_name, frame); break;
      }

      if ((winInput = waitKey(1)) == ESCAPE_KEY)
      //if ((winInput = waitKey(0)) == ESCAPE_KEY)
      {
          break;
      }
      else if(winInput == 'c') mode=1;
      else if(winInput == 's') mode=2;
      else if(winInput == 'n') mode=0;

      clock_gettime(CLOCK_MONOTONIC, &curr_t);
      curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
      // printf("Edge time=%lf msec\n", (curr_time-prev_time)*1000.0);
   }
}
