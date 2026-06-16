// EXAMPLE CODE FROM https://docs.opencv.org/4.1.1/d2/d2c/tutorial_sobel_derivatives.html
// Then modified to help me tune in the edge detection params better (which I then set as the defaults)

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
  cv::CommandLineParser parser(argc, argv,
                               "{@input   |BBB_Frame_100.jpeg|input image}"
                               "{ksize   k|1|ksize (hit 'K' to increase its value, 'k' to decrease it)}"
                               "{scale   s|2|scale (hit 'S' to increase its value, 's' to decrease it)}"
                               "{delta   d|-1|delta (hit 'D' to increase its value, 'd' to decrease it)}"
                               "{help    h|false|show help message}");
  parser.printMessage();
  cout << "\nPress 'ESC' to exit program.\nPress 'R' to reset values\n";
  // First we declare the variables we are going to use
  Mat image,src, src_gray;
  Mat grad;
  const String window_name = "Sobel Edge Detection";
  int ksize = parser.get<int>("ksize");
  int scale = parser.get<int>("scale");
  int delta = parser.get<int>("delta");
  int ddepth = CV_16S;
  String imageName = parser.get<String>("@input");
  // As usual we load our source image (src)
  image = imread( imageName, IMREAD_COLOR ); // Load an image
  // Check if image is loaded fine
  if( image.empty() )
  {
    printf("Error opening image: %s\n", imageName.c_str());
    return 1;
  }
  for (;;)
  {
    // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
    GaussianBlur(image, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
    // Convert the image to grayscale
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    Sobel(src_gray, grad_x, ddepth, 1, 0, ksize, scale, delta, BORDER_DEFAULT);
    Sobel(src_gray, grad_y, ddepth, 0, 1, ksize, scale, delta, BORDER_DEFAULT);
    // converting back to CV_8U
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
    imshow(window_name, grad);
    char key = (char)waitKey(0);
    if(key == 27)
    {
      String output_path = imageName.substr(0, imageName.find_last_of('.')) + "_sobel.jpg";
      imwrite(output_path, grad);
      cout << "Saved to: " << output_path << "\n";
      return 0;
    }
    if (key == 'k')
    {
      ksize = ksize > -1 ? ksize-2 : -1;
      cout << "ksize: " << ksize << endl;
    }
    if (key == 'K')
    {
      ksize = ksize < 30 ? ksize+2 : -1;
      cout << "ksize: " << ksize << endl;
    }
    if (key == 's')
    {
      scale--;
      cout << "scale: " << scale << endl;
    }
    if (key == 'S')
    {
      scale++;
      cout << "scale: " << scale << endl;
    }
    if (key == 'd')
    {
      delta--;
      cout << "delta: " << delta << endl;
    }
    if (key == 'D')
    {
      delta++;
      cout << "delta: " << delta << endl;
    }
    if (key == 'r' || key == 'R')
    {
      scale = parser.get<int>("scale");
      ksize = parser.get<int>("ksize");
      delta = parser.get<int>("delta");
    }
  }
  return 0;
}