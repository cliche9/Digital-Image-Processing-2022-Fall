//
//  main.cpp
//  exp7
//
//  Created by abc_mac on 2022/11/13.
//

#include <iostream>
#include <fstream>
#include <opencv.hpp>

std::string dir_path = "/Users/abc_mac/Desktop/";

void output(const std::vector<int> &hist, std::ostream &out) {
    for (int e : hist)
        out << e << ' ';
}

void equalizedHist(cv::Mat &src, cv::Mat &dst) {
    // gray histogram
    std::ofstream original_hist(dir_path + "hist_origin.txt");
    std::ofstream equalized_hist(dir_path + "hist_equalized.txt");
    std::vector<int> hist(256, 0);
    std::vector<int> equalized(256, 0);
    std::vector<double> pf_hist(256, 0);
    int width = src.cols, height = src.rows;
    int sum = width * height;       // sum of pixel count
    // original gray histogram
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ++hist[src.at<uchar>(y, x)];
        }
    }
    output(hist, original_hist);
    // accumulative freq of gray value
    pf_hist[0] = hist[0] / (sum + 0.0f);
    for (int i = 1; i < 256; ++i) {
        pf_hist[i] = pf_hist[i - 1] + hist[i] / (sum + 0.0f);
    }
    // apply the convert function
    for (int i = 0; i < 256; ++i) {
        hist[i] = int(pf_hist[i] * 255 + 0.5);
    }
    // get the equalized image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            dst.at<uchar>(y, x) = hist[src.at<uchar>(y, x)];
            ++equalized[dst.at<uchar>(y, x)];
        }
    }
    output(equalized, equalized_hist);
}

int main(int argc, const char * argv[]) {
    // convert rgb to gray
    cv::Mat src = cv::imread(dir_path + "lena_std.tif");
    cv::Mat dst(src.size(), src.type());
    if (src.channels() == 1) {
        equalizedHist(src, dst);
    } else {
        cv::Mat ycrcb;
        cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        split(ycrcb, channels);
        equalizedHist(channels[0], channels[0]);
        merge(channels,ycrcb);
        cvtColor(ycrcb, dst, cv::COLOR_YCrCb2BGR);
    }
    cv::imwrite(dir_path + "equalized.png", dst);
    
    return 0;
}
