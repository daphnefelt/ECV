// ORB keypoint detection and optical flow tracking
// usage: ./track Challenge_Set_Videos/Night_Walk/Night_Walk.MOV 30 50
// to make a video from the output frames: ffmpeg -framerate 30 -i frames/frame_%05d.jpg output.mp4

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <filesystem>
#include <vector>

static constexpr int MIN_TRACKS = 30; // need at least this many per frame, otherwise redetect
static constexpr int MAX_ORB = 500;
static constexpr bool SAVE_FRAMES = true;
static const std::string FRAME_DIR  = "frames";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video.mp4> [start_sec] [end_sec]\n";
        return 1;
    }

    // load in video
    std::string video_path = argv[1];
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Can't open " << video_path << "\n";
        return 1;
    }
    double fps = cap.get(cv::CAP_PROP_FPS);
    int total_frames = (int)cap.get(cv::CAP_PROP_FRAME_COUNT);
    double total_sec = total_frames / fps;

    double start_sec = (argc >= 3) ? std::stod(argv[2]) : 0.0;
    double end_sec = (argc >= 4) ? std::stod(argv[3]) : total_sec;
    int start_frame = (int)(start_sec * fps);
    int end_frame = (int)(end_sec * fps);
    cap.set(cv::CAP_PROP_POS_FRAMES, start_frame); // jump to start

    if (SAVE_FRAMES)
        std::filesystem::create_directories(FRAME_DIR); // build output dir

    // ORB and KLT setup
    auto orb = cv::ORB::create(MAX_ORB);
    std::vector<uchar> status;
    std::vector<float> err;
    std::vector<cv::KeyPoint> keypoints;
    // need diff keypoint colors, just stepping around color wheel
    const int N_COLORS = 20;
    std::vector<cv::Scalar> PALETTE;
    cv::Mat bgr;
    for (int i = 0; i < N_COLORS; i++) {
        cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(180 * i / N_COLORS, 220, 220));
        cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
        auto* p = bgr.ptr<uint8_t>(0);
        PALETTE.push_back(cv::Scalar(p[0], p[1], p[2]));
    }
    std::vector<int> track_ids;
    int next_id = 0;
    std::vector<int> good_ids;

    cv::Mat prev_gray, prev_color, gray, color;
    std::vector<cv::Point2f> prev_pts, good_prev_pts, curr_pts, good_pts; // where good points are the sucessfully tracked ones
    bool initialized = false;
    int frame_idx = start_frame;
    int saved_count = 0; // for ffmpeg output frame numbering
    int total_above = 0; // frames with >= MIN_TRACKS active tracks
    int total_counted = 0;

    while (true) {
        if (!cap.read(color) || color.empty()) break;
        if (frame_idx > end_frame) break;

        cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);
        double timestamp = frame_idx / fps;

        int num_tracks = 0;
        // Detect orb keypoints
        if (!initialized || (int)prev_pts.size() < MIN_TRACKS) {
            std::printf("(redetecting)\n");
            orb->detect(gray, keypoints);
            prev_pts.clear(); track_ids.clear();
            cv::KeyPoint::convert(keypoints, prev_pts);
            // assign IDs to each point found
            for (size_t i = 0; i < prev_pts.size(); i++)
                track_ids.push_back(next_id++);
            prev_gray = gray.clone();
            prev_color = color.clone();
            initialized = true;
            frame_idx++;
            continue; // need two frames to track
        }

        // KLT optical flow
        good_pts.clear(); good_prev_pts.clear(); good_ids.clear(); status.clear(); err.clear();
        cv::calcOpticalFlowPyrLK(prev_gray, gray, prev_pts,  curr_pts, status, err, cv::Size(21, 21), 3); // last two args are search window size and pyramid levels
        // filter to successfull tracks
        for (size_t i = 0; i < status.size(); i++) {
            if (status[i]){
                good_pts.push_back(curr_pts[i]);
                good_prev_pts.push_back(prev_pts[i]);
                good_ids.push_back(track_ids[i]);
            }
        }
        num_tracks = (int)good_pts.size();
        total_counted++;
        if (num_tracks >= MIN_TRACKS) total_above++;

        // Output image is the ith frame and i-1 frame side by side w/ lines connecting keypoints
        int W = color.cols, H = color.rows;
        cv::Mat canvas(H, W * 2, CV_8UC3);
        prev_color.copyTo(canvas(cv::Rect(0, 0, W, H))); // prev on left
        color.copyTo(canvas(cv::Rect(W, 0, W, H))); // curr on right
        double sum_dx = 0, sum_dy = 0; // tracking displacement for average motion arrow
 
        for (size_t i = 0; i < good_pts.size(); i++) {
            cv::Scalar col = PALETTE[good_ids[i] % PALETTE.size()];
            cv::Point2f left_pt = good_prev_pts[i];
            cv::Point2f right_pt = good_pts[i] + cv::Point2f((float)W, 0);
            cv::circle(canvas, left_pt, 5, col, -1);
            cv::circle(canvas, right_pt, 5, col, -1);
            cv::line(canvas, left_pt, right_pt, col, 1);
 
            // add displacement for this keypoint
            sum_dx += good_pts[i].x - good_prev_pts[i].x;
            sum_dy += good_pts[i].y - good_prev_pts[i].y;
        }
 
        // arrow for avg motion between frames
        if (num_tracks > 0) {
            double avg_dx = sum_dx / num_tracks;
            double avg_dy = sum_dy / num_tracks;
            cv::Point2f arrow_start((float)(W + W / 2), (float)(H / 2));
            float arrow_scale = -5.0f;
            cv::Point2f arrow_end(arrow_start.x + (float)(avg_dx * arrow_scale), arrow_start.y + (float)(avg_dy * arrow_scale));
            cv::arrowedLine(canvas, arrow_start, arrow_end, cv::Scalar(255, 255, 255), 3, cv::LINE_AA, 0, 0.3);
        }
 
        if (SAVE_FRAMES) {
            std::string filename = FRAME_DIR + "/frame_" + std::string(5 - std::to_string(saved_count).size(), '0') + std::to_string(saved_count) + ".jpg";
            cv::imwrite(filename, canvas);
            saved_count++;
        }

        // log
        if ((frame_idx - start_frame) % 30 == 0) {
            double survival = total_counted > 0 ? 100.0 * total_above / total_counted : 0.0;
            std::printf("frame %5d     t=%6.2fs      tracks=%3d     survival=%.1f%%\n", frame_idx, timestamp, num_tracks, survival);
        }

        // carry everything over
        prev_pts = good_pts;
        track_ids = good_ids;
        prev_gray = gray.clone();
        prev_color = color.clone();
        frame_idx++;
    }

    double survival = total_counted > 0 ? 100.0 * total_above / total_counted : 0.0;
    std::cout << "Done! Track survival rate: " << survival << "% " << "(" << total_above << "/" << total_counted << " frames)\n";
    return 0;
}