//
//  main.cpp
//  exp9
//
//  Created by abc_mac on 2022/11/20.
//

#include <iostream>
#include <opencv.hpp>

const std::string dir_path = "/Users/abc_mac/Desktop/";

void edgeResponse(cv::Mat &src, cv::Mat &mag, cv::Mat &grad_x, cv::Mat &grad_y, cv::Mat &angles) {
    // 1. compute grad_x & grad_y
    cv::Sobel(src, grad_x, CV_32F, 1, 0);
    cv::Sobel(src, grad_y, CV_32F, 0, 1);
    // 2. compute magnitude & angles
    cv::cartToPolar(grad_x, grad_y, mag, angles, true);
    cv::convertScaleAbs(grad_x, grad_x);
    cv::convertScaleAbs(grad_y, grad_y);
    cv::convertScaleAbs(mag, mag);
    // 3. paint grads and angle
    cv::imwrite(dir_path + "edge_response_x.png", grad_x);
    cv::imwrite(dir_path + "edge_response_y.png", grad_y);
    cv::imwrite(dir_path + "edge_response.png", mag);
    cv::imwrite(dir_path + "edge_response_angle.png", angles);
}

void nmsMethod(cv::Mat &mag, cv::Mat &angles, cv::Mat &dst) {
    int width = mag.cols, height = mag.rows;
    dst = mag.clone();
    
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            // scale angle to [0, 180]
            float angle = angles.at<float>(y, x) > 180.0f ? angles.at<float>(y, x) : angles.at<float>(y, x) - 180.0f;
            /* sample 3x3 area for interplotation
             00 | 01 | 02
             10 | 11 | 12
             20 | 21 | 22
             */
            int sample[3][3] = {
                mag.at<uchar>(y - 1, x - 1),
                mag.at<uchar>(y - 1, x),
                mag.at<uchar>(y - 1, x + 1),
                mag.at<uchar>(y, x - 1),
                mag.at<uchar>(y, x),
                mag.at<uchar>(y, x + 1),
                mag.at<uchar>(y + 1, x - 1),
                mag.at<uchar>(y + 1, x),
                mag.at<uchar>(y + 1, x + 1)
            };
            // nms filter
            if ((angle >= 0 && angle < 45)) {
                if ((sample[1][1] <= (sample[1][2] + (sample[0][2] - sample[1][2]) * tan(angle))) ||
                    (sample[1][1] <= (sample[1][0] + (sample[2][0] - sample[1][0]) * tan(angle)))) {
                    dst.at<uchar>(y, x) = 0;
                }
            } else if ((angle >= 45 && angle < 90)) {
                if ((sample[1][1] <= (sample[0][1] + (sample[0][2] - sample[0][1]) / tan(angle))) ||
                    (sample[1][1] <= (sample[2][1] + (sample[2][0] - sample[2][1]) / tan(angle)))) {
                    dst.at<uchar>(y, x) = 0;
                }
            } else if ((angle >= 90 && angle < 135)) {
                if ((sample[1][1] <= (sample[0][1] + (sample[0][0] - sample[0][1]) / tan(180.0f - angle))) ||
                    (sample[1][1] <= (sample[2][1] + (sample[2][2] - sample[2][1]) / tan(180.0f - angle)))) {
                    dst.at<uchar>(y, x) = 0;
                }
            } else {
                if ((sample[1][1] <= (sample[1][0] + (sample[0][0] - sample[1][0]) * tan(180.0f - angle))) ||
                    (sample[1][1] <= (sample[1][2] + (sample[2][2] - sample[1][2]) * tan(180.0f - angle)))) {
                    dst.at<uchar>(y, x) = 0;
                }
            }
        }
    }
    cv::imwrite(dir_path + "nms_result.png", dst);
}

// double threshold filter
void doubleThreshold(cv::Mat &src, double low, double high) {
    for (int y = 0; y < src.rows; ++y) {
        for (int x = 0; x < src.cols; ++x) {
            if (src.at<uchar>(y, x) >= high)
                src.at<uchar>(y, x) = 255;
            else if (src.at<uchar>(y, x) < low)
                src.at<uchar>(y, x) = 0;
        }
    }
}

// double threshold filter and link
void doubleThresholdLink(cv::Mat &src, double low, double high) {
    int growth = 0;
    do {
        growth = 0;
        for (int y = 1; y < src.rows - 1; ++y) {
            for (int x = 1; x < src.cols - 1; ++x) {
                if (src.at<uchar>(y, x) >= low && src.at<uchar>(y, x) < high) {
                    if (src.at<uchar>(y - 1, x - 1) == 255 ||
                        src.at<uchar>(y - 1, x) == 255 ||
                        src.at<uchar>(y - 1, x + 1) == 255 ||
                        src.at<uchar>(y, x - 1) == 255 ||
                        src.at<uchar>(y, x + 1) == 255 ||
                        src.at<uchar>(y + 1, x - 1) == 255 ||
                        src.at<uchar>(y + 1, x) == 255 ||
                        src.at<uchar>(y + 1, x + 1) == 255
                        ) {
                        src.at<uchar>(y, x) = 255;
                        ++growth;
                    } else {
                        src.at<uchar>(y, x) = 0;
                    }
                }
            }
        }
    } while (growth);
}

int main(int argc, const char * argv[]) {
    cv::Mat src = cv::imread(dir_path + "4bottles.png");
    // 1. rgb 2 gray
    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    // 2. gaussian blur
    cv::GaussianBlur(src, src, cv::Size(3, 3), 0, 0);
    // 3. edge response
    cv::Mat mag, grad_x, grad_y, angles;
    edgeResponse(src, mag, grad_x, grad_y, angles);
    // 4. edge mask by nms method
    cv::Mat dst;
    nmsMethod(mag, angles, dst);
    // 5. double threshold and link
    doubleThreshold(dst, 50, 100);
    doubleThresholdLink(dst, 50, 100);
    cv::imwrite(dir_path + "threshold_result.png", dst);
    cv::Canny(src, dst, 50, 100);
    cv::imwrite(dir_path + "canny_result.png", dst);
    
    return 0;
}
