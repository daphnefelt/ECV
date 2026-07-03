// Baseline code from Sam Siewart
// Used Claude to help me see how to convert to OpenCV4, this was a very nontrivial task
// To run test example: ./descriptor_extractor_matcher ORB ORB BruteForce-Hamming CrossCheckFilter IMG_0644.jpeg IMG_0645.jpeg 3

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace cv;
using namespace std;

static void help(char** argv)
{
    cout << "\nThis program demonstrates keypoint finding and matching between 2 images "
            "using the features2d framework (OpenCV4 port).\n"
         << "\n"
         << "Case1: second image is obtained from the first (given) image using a randomly generated homography\n"
         << argv[0] << " [detectorType] [descriptorType] [matcherType] [matcherFilterType] [image]\n"
         << "Example of case1:\n"
         << "./descriptor_extractor_matcher ORB ORB BruteForce-Hamming CrossCheckFilter cola.jpg\n"
         << "\n"
         << "Case2: both images are given. If ransacReprojThreshold>=0 then a homography matrix is calculated\n"
         << argv[0] << " [detectorType] [descriptorType] [matcherType] [matcherFilterType] [image1] [image2] [ransacReprojThreshold]\n"
         << "Example of case2:\n"
         << "./descriptor_extractor_matcher ORB ORB BruteForce-Hamming CrossCheckFilter cola1.jpg cola2.jpg 3\n"
         << "\n"
         << "detectorType/descriptorType: ORB, SIFT, BRISK, AKAZE\n"
         << "matcherType: BruteForce, BruteForce-Hamming, FlannBased\n"
         << "matcherFilterType: NoneFilter, CrossCheckFilter\n" << endl;
}

enum { NONE_FILTER = 0, CROSS_CHECK_FILTER = 1 };

static int getMatcherFilterType(const string& str)
{
    if (str == "NoneFilter") return NONE_FILTER;
    if (str == "CrossCheckFilter") return CROSS_CHECK_FILTER;
    CV_Error(cv::Error::StsBadArg, "Invalid filter name");
    return -1;
}

static Ptr<Feature2D> createFeature2D(const string& name)
{
    if (name == "ORB")   return ORB::create();
    if (name == "SIFT")  return SIFT::create();
    if (name == "BRISK") return BRISK::create();
    if (name == "AKAZE") return AKAZE::create();
    CV_Error(cv::Error::StsBadArg, "Unknown detector/descriptor type: " + name +
              " (supported: ORB, SIFT, BRISK, AKAZE)");
    return Ptr<Feature2D>();
}

static void simpleMatching(const Ptr<DescriptorMatcher>& descriptorMatcher,
                            const Mat& descriptors1, const Mat& descriptors2,
                            vector<DMatch>& matches12)
{
    descriptorMatcher->match(descriptors1, descriptors2, matches12);
}

static void crossCheckMatching(const Ptr<DescriptorMatcher>& descriptorMatcher,
                                const Mat& descriptors1, const Mat& descriptors2,
                                vector<DMatch>& filteredMatches12, int knn = 1)
{
    filteredMatches12.clear();
    vector<vector<DMatch>> matches12, matches21;
    descriptorMatcher->knnMatch(descriptors1, descriptors2, matches12, knn);
    descriptorMatcher->knnMatch(descriptors2, descriptors1, matches21, knn);
    for (size_t m = 0; m < matches12.size(); m++)
    {
        bool findCrossCheck = false;
        for (size_t fk = 0; fk < matches12[m].size(); fk++)
        {
            DMatch forward = matches12[m][fk];
            for (size_t bk = 0; bk < matches21[forward.trainIdx].size(); bk++)
            {
                DMatch backward = matches21[forward.trainIdx][bk];
                if (backward.trainIdx == forward.queryIdx)
                {
                    filteredMatches12.push_back(forward);
                    findCrossCheck = true;
                    break;
                }
            }
            if (findCrossCheck) break;
        }
    }
}

static void warpPerspectiveRand(const Mat& src, Mat& dst, Mat& H, RNG& rng)
{
    H.create(3, 3, CV_32FC1);
    H.at<float>(0,0) = rng.uniform( 0.8f, 1.2f);
    H.at<float>(0,1) = rng.uniform(-0.1f, 0.1f);
    H.at<float>(0,2) = rng.uniform(-0.1f, 0.1f) * src.cols;
    H.at<float>(1,0) = rng.uniform(-0.1f, 0.1f);
    H.at<float>(1,1) = rng.uniform( 0.8f, 1.2f);
    H.at<float>(1,2) = rng.uniform(-0.1f, 0.1f) * src.rows;
    H.at<float>(2,0) = rng.uniform(-1e-4f, 1e-4f);
    H.at<float>(2,1) = rng.uniform(-1e-4f, 1e-4f);
    H.at<float>(2,2) = rng.uniform( 0.8f, 1.2f);

    warpPerspective(src, dst, H, src.size());
}

static const string winName = "correspondences";

static void doIteration(const Mat& img1, Mat& img2, bool isWarpPerspective,
                         vector<KeyPoint>& keypoints1, const Mat& descriptors1,
                         const Ptr<Feature2D>& detector, const Ptr<Feature2D>& descriptorExtractor,
                         const Ptr<DescriptorMatcher>& descriptorMatcher, int matcherFilter,
                         double ransacReprojThreshold, RNG& rng)
{
    CV_Assert(!img1.empty());
    Mat H12;
    if (isWarpPerspective)
        warpPerspectiveRand(img1, img2, H12, rng);
    else
        CV_Assert(!img2.empty());

    cout << endl << "< Extracting keypoints from second image..." << endl;
    vector<KeyPoint> keypoints2;
    detector->detect(img2, keypoints2);
    cout << keypoints2.size() << " points" << endl << ">" << endl;

    cout << "< Computing descriptors for keypoints from second image..." << endl;
    Mat descriptors2;
    descriptorExtractor->compute(img2, keypoints2, descriptors2);
    cout << ">" << endl;

    cout << "< Matching descriptors..." << endl;
    vector<DMatch> filteredMatches;
    switch (matcherFilter)
    {
    case CROSS_CHECK_FILTER:
        crossCheckMatching(descriptorMatcher, descriptors1, descriptors2, filteredMatches, 1);
        break;
    default:
        simpleMatching(descriptorMatcher, descriptors1, descriptors2, filteredMatches);
    }
    cout << ">" << endl;

    vector<int> queryIdxs(filteredMatches.size()), trainIdxs(filteredMatches.size());
    for (size_t i = 0; i < filteredMatches.size(); i++)
    {
        queryIdxs[i] = filteredMatches[i].queryIdx;
        trainIdxs[i] = filteredMatches[i].trainIdx;
    }

    if (!isWarpPerspective && ransacReprojThreshold >= 0)
    {
        cout << "< Computing homography (RANSAC)..." << endl;
        vector<Point2f> points1; KeyPoint::convert(keypoints1, points1, queryIdxs);
        vector<Point2f> points2; KeyPoint::convert(keypoints2, points2, trainIdxs);
        H12 = findHomography(Mat(points1), Mat(points2), cv::RANSAC, ransacReprojThreshold);
        cout << ">" << endl;
    }

    Mat drawImg;
    if (!H12.empty()) // filter outliers
    {
        vector<char> matchesMask(filteredMatches.size(), 0);
        vector<Point2f> points1; KeyPoint::convert(keypoints1, points1, queryIdxs);
        vector<Point2f> points2; KeyPoint::convert(keypoints2, points2, trainIdxs);
        Mat points1t;
        perspectiveTransform(Mat(points1), points1t, H12);

        double maxInlierDist = ransacReprojThreshold < 0 ? 3 : ransacReprojThreshold;
        for (size_t i1 = 0; i1 < points1.size(); i1++)
        {
            if (norm(points2[i1] - points1t.at<Point2f>((int)i1, 0)) <= maxInlierDist) // inlier
                matchesMask[i1] = 1;
        }
        drawMatches(img1, keypoints1, img2, keypoints2, filteredMatches, drawImg,
                    Scalar(0, 255, 0), Scalar(255, 0, 0), matchesMask);
    }
    else
    {
        drawMatches(img1, keypoints1, img2, keypoints2, filteredMatches, drawImg);
    }

    imshow(winName, drawImg);
}

int main(int argc, char** argv)
{
    if (argc != 6 && argc != 8)
    {
        help(argv);
        return -1;
    }
    bool isWarpPerspective = (argc == 6);
    double ransacReprojThreshold = -1;
    if (!isWarpPerspective)
        ransacReprojThreshold = atof(argv[7]);

    cout << "< Creating detector, descriptor extractor and descriptor matcher ..." << endl;
    Ptr<Feature2D> detector = createFeature2D(argv[1]);
    Ptr<Feature2D> descriptorExtractor = createFeature2D(argv[2]);
    Ptr<DescriptorMatcher> descriptorMatcher = DescriptorMatcher::create(argv[3]);
    int matcherFilterType = getMatcherFilterType(argv[4]);
    cout << ">" << endl;
    if (detector.empty() || descriptorExtractor.empty() || descriptorMatcher.empty())
    {
        cout << "Can not create detector or descriptor extractor or descriptor matcher of given types" << endl;
        return -1;
    }

    cout << "< Reading the images..." << endl;
    Mat img1 = imread(argv[5]), img2;
    if (!isWarpPerspective)
        img2 = imread(argv[6]);
    cout << ">" << endl;
    if (img1.empty() || (!isWarpPerspective && img2.empty()))
    {
        cout << "Can not read images" << endl;
        return -1;
    }

    const Size targetSize(240 * 2, 320 * 2);
    resize(img1, img1, targetSize, 0, 0, INTER_AREA);
    if (!isWarpPerspective)
        resize(img2, img2, targetSize, 0, 0, INTER_AREA);

    cout << endl << "< Extracting keypoints from first image..." << endl;
    vector<KeyPoint> keypoints1;
    detector->detect(img1, keypoints1);
    cout << keypoints1.size() << " points" << endl << ">" << endl;

    cout << "< Computing descriptors for keypoints from first image..." << endl;
    Mat descriptors1;
    descriptorExtractor->compute(img1, keypoints1, descriptors1);
    cout << ">" << endl;

    namedWindow(winName, WINDOW_AUTOSIZE);
    RNG rng = theRNG();
    doIteration(img1, img2, isWarpPerspective, keypoints1, descriptors1,
                detector, descriptorExtractor, descriptorMatcher, matcherFilterType,
                ransacReprojThreshold, rng);
    for (;;)
    {
        char c = (char)waitKey(0);
        if (c == 27) // esc
        {
            cout << "Exiting ..." << endl;
            break;
        }
        else if (isWarpPerspective)
        {
            doIteration(img1, img2, isWarpPerspective, keypoints1, descriptors1,
                        detector, descriptorExtractor, descriptorMatcher, matcherFilterType,
                        ransacReprojThreshold, rng);
        }
    }
    return 0;
}