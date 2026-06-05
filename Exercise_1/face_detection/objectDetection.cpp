// EXAMPLE CODE FROM https://github.com/opencv/opencv/blob/master/samples/cpp/tutorial_code/objectDetection/objectDetection.cpp
// Then modified for image inputs

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat& frame );

/** Global variables */
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;

/** @function main */
int main( int argc, const char** argv )
{
    CommandLineParser parser(argc, argv,
                             "{help h||}"
                             "{face_cascade|haarcascade_frontalface_alt.xml|Path to face cascade.}"
                             "{eyes_cascade|haarcascade_eye_tree_eyeglasses.xml|Path to eyes cascade.}"
                             "{image|faculty_img.png|Path to input image.}");

    parser.about( "\nThis program demonstrates using the cv::CascadeClassifier class to detect objects (Face + eyes) on a static img.\n"
                  "You can use Haar or LBP features.\n\n" );
    parser.printMessage();

    String face_cascade_name = samples::findFile( parser.get<String>("face_cascade") );
    String eyes_cascade_name = samples::findFile( parser.get<String>("eyes_cascade") );
    String image_path = parser.get<String>("image");

    //-- 1. Load the cascades
    if( !face_cascade.load( face_cascade_name ) )
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    if( !eyes_cascade.load( eyes_cascade_name ) )
    {
        cout << "--(!)Error loading eyes cascade\n";
        return -1;
    };

    // Load in input img
    Mat frame = imread(image_path);

    //-- 3. Apply the classifier to the frame
    detectAndDisplay( frame );

    // Save face detected img (got help from Claude with the file paths)
    String output_path = image_path.substr(0, image_path.find_last_of('.')) + "_facedetected.png";
    imwrite(output_path, frame);
    cout << "Saved to: " << output_path << "\n";

    waitKey(0); // any key closes window
    
    return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat& frame )
{
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale( frame_gray, faces );

    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 );

        Mat faceROI = frame_gray( faces[i] );

        //-- In each face, detect eyes
        std::vector<Rect> eyes;
        eyes_cascade.detectMultiScale( faceROI, eyes );

        for ( size_t j = 0; j < eyes.size(); j++ )
        {
            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 4 );
        }
    }

    //-- Show what you got
    imshow( "Capture - Face detection", frame );
}
