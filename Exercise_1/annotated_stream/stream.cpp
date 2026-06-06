// Based on example by Sam Siewert: ECV-ECEE-5763/computer_vision_cv4_tested/simpler-capture-4/capture.cpp

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

// See www.asciitable.com
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

// From exercise 1 for the image output
#define OUTPUT_WIDTH (320)
#define OUTPUT_HEIGHT (240)

int main()
{
   VideoCapture cam0(0);
   namedWindow("video_display");
   char winInput;
   int saved_frames = 0;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   // setting cam res to the output resolution I want
   cam0.set(CAP_PROP_FRAME_WIDTH, OUTPUT_WIDTH);
   cam0.set(CAP_PROP_FRAME_HEIGHT, OUTPUT_HEIGHT);

   while (1)
   {
      Mat frame;
      cam0.read(frame);

      // Annotations

      // Crosshair in middle
      int cx = frame.cols / 2; // forcing these to int will round down
      int cy = frame.rows / 2;
      line(frame, Point(cx, 0), Point(cx, frame.rows), Scalar(0, 255, 255), 1);
      line(frame, Point(0, cy), Point(frame.cols, cy), Scalar(0, 255, 255), 1);

      // Border of 4 pixels
      rectangle(frame, Point(0, 0), Point(frame.cols, frame.rows), Scalar(0, 0, 255), 4); // Doesn't specify color, I think red would look good

      imshow("video_display", frame);

      if ((winInput = waitKey(10)) == ESCAPE_KEY)
      //if ((winInput = waitKey(0)) == ESCAPE_KEY)
      {
          break;
      }
      else if(winInput == 's') // hotkey to save a frame
      {
        ostringstream filename;
        filename << "frame_" << saved_frames << ".jpg";
        imwrite(filename.str(), frame);
        cout << "saved " << filename.str() << endl;
        saved_frames++; // don't overwrite for next frame
      }
      else if(winInput == 'n')
      {
	  cout << "input " << winInput << " ignored" << endl;
      }
      
   }

   destroyWindow("video_display"); 
};
