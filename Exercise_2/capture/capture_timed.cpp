// Based on capture_timped.cpp example by Sam Siewert 
// Added syslog (you can read it in and save it using "journalctl | grep frame_times > frame_times.log") w/ a final fps calc at end
// Also updated stream to include the current fps in the display window, updated every 150 frames

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <syslog.h>

using namespace cv;
using namespace std;

// See www.asciitable.com
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

int main()
{
   openlog("frame_times", LOG_PID|LOG_CONS, LOG_USER);

   VideoCapture cam0(0);
   namedWindow("video_display");
   char winInput;

   struct timespec curr_t;
   double curr_time, prev_time;
   unsigned int frameCnt=0;

   // for tracking camera frame rate
   double fps=0.0;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

   clock_gettime(CLOCK_MONOTONIC, &curr_t);
   curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
   prev_time=curr_time;
   double interval_start_time=curr_time;
   double code_start_time=curr_time;

   while (1)
   {
      Mat frame;

      // get time here
      clock_gettime(CLOCK_MONOTONIC, &curr_t);
      curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
      printf("frameCnt=%u, curr_time=%lf sec, dt=%lf msec\n", frameCnt, curr_time, (curr_time-prev_time)*1000.0);

      cam0.read(frame);

      prev_time=curr_time;
      frameCnt++;
      syslog(LOG_INFO, "frameCnt=%u, curr_time=%lf sec", frameCnt, curr_time);

      if (frameCnt % 150 == 0) // every 150 frames is around 5s
      {
            fps = 150.0 / (curr_time-interval_start_time);
            interval_start_time=curr_time;
      }

      std::string fps_str = std::to_string(fps) + " FPS";
      putText(frame, fps_str, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(163,63,136), 2);
      imshow("video_display", frame);


      if ((winInput = waitKey(1)) == ESCAPE_KEY)
      //if ((winInput = waitKey(0)) == ESCAPE_KEY)
      {
          double final_fps = (double)frameCnt / (curr_time-code_start_time);
          syslog(LOG_INFO, "FINAL FPS=%f, TOTAL TIME=%lf sec", final_fps, (curr_time-code_start_time));
          break;
      }
      else if(winInput == 'n')
      {
	  cout << "input " << winInput << " ignored" << endl;
      }
      
   }

   destroyWindow("video_display"); 
   closelog();
};
