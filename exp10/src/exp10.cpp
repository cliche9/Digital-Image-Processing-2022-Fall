//
//  main.cpp
//  exp10
//
//  Created by abc_mac on 2022/11/28.
//

#include <iostream>
#include <opencv.hpp>

const std::string dir_path = "/Users/abc_mac/Desktop/";

void replace(cv::Mat &frame, cv::Mat &bg, cv::Mat &mask, cv::Mat &dst) {
    dst = frame.clone();
    for (int y = 0; y < frame.rows; ++y) {
        for (int x = 0; x < frame.cols; ++x) {
            if (mask.at<uchar>(y, x) == 255)
                dst.at<cv::Vec3b>(y, x) = bg.at<cv::Vec3b>(y, x);
        }
    }
}

int main(int argc, const char * argv[]) {
    cv::Mat background = cv::imread(dir_path + "bg.png");
    cv::VideoCapture cap(dir_path + "video.mp4");
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    // 1. resize bg image to the same size of video frame
    cv::resize(background, background, cv::Size(width, height), 1, 1, cv::INTER_CUBIC);
    // green color range in hsv
    int hmin = 35, smin = 43, vmin = 46;
    int hmax = 77, smax = 255, vmax = 255;
    
    cv::Mat frame, hsv, mask, dst;
    int frame_counter = 0;
    
    while (true) {
        cap >> frame;
        ++frame_counter;
        if (frame_counter == int(cap.get(cv::CAP_PROP_FRAME_COUNT))) {
            frame_counter = 0;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        }
        // 2. convert to hsv
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        // 3. get front mask of every frame
        cv::inRange(hsv, cv::Scalar(hmin, smin, vmin), cv::Scalar(hmax, smax, vmax), mask);
        // 4. replace the background of original frame
        replace(frame, background, mask, dst);
        
        cv::imshow("original", frame);
        cv::imshow("mask", mask);
        cv::imshow("replaced", dst);
        
        if (cv::waitKey(10) == 27)
            break;
    }
    
    cv::destroyAllWindows();
    cap.release();
    
    return 0;
}
