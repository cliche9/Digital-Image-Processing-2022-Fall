//
//  main.cpp
//  exp8
//
//  Created by abc_mac on 2022/11/14.
//

#include <iostream>
#include <opencv.hpp>

const std::string dir_path = "/Users/abc_mac/Desktop/";

void distanceTransfrom(cv::Mat &src, cv::Mat &dst) {
    cv::threshold(src, dst, 150, 255, cv::THRESH_BINARY);
    dst.convertTo(dst, CV_32F);
    // create forward & backward mask
    cv::Mat mask_forward(2, 2, CV_32F);
    /**
     1.414  |  1
     --------------
       1    |   0
     */
    mask_forward.at<float>(0, 0) = 1.414;
    mask_forward.at<float>(0, 1) = mask_forward.at<float>(1, 0) = 1;
    mask_forward.at<float>(1, 1) = 0;
    cv::Mat mask_backward(2, 2, CV_32F);
    /**
      0     |    1
     --------------
      1     | 1.414
     */
    mask_backward.at<float>(0, 0) = 0;
    mask_backward.at<float>(0, 1) = mask_backward.at<float>(1, 0) = 1;
    mask_backward.at<float>(1, 1) = 1.414;
    int mask_width = mask_forward.cols, mask_height = mask_forward.rows;

    // forward traversing
    for (int y = mask_height - 1; y < src.rows; ++y) {
        for (int x = mask_width - 1; x < src.cols; ++x) {
            for (int i = mask_height - 1; i >= 0; --i) {
                for (int j = mask_width - 1; j >= 0; --j) {
                    dst.at<float>(y, x) = std::min(dst.at<float>(y, x), dst.at<float>(y - i, x - j) + mask_forward.at<float>(mask_height - 1 - i, mask_width - 1 - j));
                }
            }
        }
    }
    // backward traversing
    for (int y = src.rows - mask_height; y >= 0; --y) {
        for (int x = src.cols - mask_width; x >= 0; --x) {
            for (int i = 0; i < mask_height; ++i) {
                for (int j = 0; j < mask_width; ++j) {
                    dst.at<float>(y, x) = std::min(dst.at<float>(y, x), dst.at<float>(y + i, x + j) + mask_backward.at<float>(i, j));
                }
            }
        }
    }
}

int main(int argc, const char * argv[]) {
    cv::Mat src = cv::imread(dir_path + "distance_transform.png", cv::IMREAD_GRAYSCALE);
    cv::Mat dst;
    distanceTransfrom(src, dst);
    dst.convertTo(dst, CV_8U);
    cv::imwrite(dir_path + "result.png", dst);
    return 0;
}
