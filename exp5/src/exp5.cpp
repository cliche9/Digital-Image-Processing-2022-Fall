//
//  main.cpp
//  exp5
//
//  Created by abc_mac on 2022/10/24.
//

#include <opencv.hpp>
#include <iostream>

static void meanFilter(const cv::Mat &input, cv::Mat &output, cv::Size wsize) {
    // check window size
    if (wsize.width % 2 == 0 || wsize.height % 2 == 0) {
        std::cerr << "window has to be odd size!\n";
        exit(-1);
    }
    // padding
    int vertical = (wsize.height - 1) / 2;
    int horizontal = (wsize.width - 1) / 2;
    cv::Mat padding_input;
    cv::copyMakeBorder(input, padding_input, vertical, vertical, horizontal, horizontal, cv::BORDER_REFLECT_101);
    output = cv::Mat::zeros(input.rows, input.cols, input.type());
    /** mean filter */
    double sum = 0, mean = 0;
    // traverse (x, y) in the image
    for (int y = vertical; y < input.rows + vertical; ++y) {
        for (int x = horizontal; x < input.cols + horizontal; ++x) {
            // traverse (i, j) in the filter map
            for (int i = y - vertical; i <= y + vertical; ++i) {
                for (int j = x - horizontal; j <= x + horizontal; ++j) {
                    sum += padding_input.at<uchar>(i, j);
                }
            }
            mean = sum / wsize.area();
            output.at<uchar>(y - vertical, x - horizontal) = mean;
            sum = 0;
            mean = 0;
        }
    }
}

static void fastIntegral(const cv::Mat &input, cv::Mat &integral) {
    integral = cv::Mat::zeros(input.rows + 1, input.cols + 1, CV_64F);
    for (int y = 1; y < integral.rows; ++y) {
        for (int x = 1, sum = 0; x < integral.cols; ++x) {
            // integral by rows: integral(i, j) = integral(i - 1, j) + sum(row(i));
            sum += input.at<uchar>(y - 1, x - 1);
            integral.at<double>(y, x) = integral.at<double>(y - 1, x) + sum;
            // std::cout << (int)integral.at<double>(y, x) << std::endl;
        }
    }
}

static void meanFilterIntegral(const cv::Mat &input, cv::Mat &output, cv::Size wsize) {
    // check window size
    if (wsize.width % 2 == 0 || wsize.height % 2 == 0) {
        std::cerr << "window has to be odd size!\n";
        exit(-1);
    }
    // padding
    int vertical = (wsize.height - 1) / 2;
    int horizontal = (wsize.width - 1) / 2;
    cv::Mat padding_input, integral;
    cv::copyMakeBorder(input, padding_input, vertical, vertical, horizontal, horizontal, cv::BORDER_REFLECT_101);
    output = cv::Mat::zeros(input.rows, input.cols, input.type());
    // calculate the integral map
    fastIntegral(padding_input, integral);
    // mean filter
    for (int y = vertical; y < input.rows + vertical; ++y) {
        for (int x = horizontal; x < input.cols + horizontal; ++x) {
            double left_top = integral.at<double>(y - vertical, x - horizontal);
            double left_bottom = integral.at<double>(y + vertical + 1, x - horizontal);
            double right_bottom = integral.at<double>(y + vertical + 1, x + horizontal + 1);
            double right_top = integral.at<double>(y - vertical, x + horizontal + 1);
            double mean = (right_bottom - left_bottom - right_top + left_top) / wsize.area();
            
            // clamp mean to [0, 255];
            if (mean < 0)
                mean = 0;
            else if (mean > 255)
                mean = 255;
            
            output.at<uchar>(y - vertical, x - horizontal) = mean;
        }
    }
}

int main(int argc, const char * argv[]) {
    cv::Mat src = cv::imread("/Users/abc_mac/Desktop/Pillars_of_Creation.png");
    if (src.empty())
        return -1;
    
    if (src.channels() > 1)
        cv::cvtColor(src, src, cv::COLOR_RGB2GRAY);
    
    cv::Mat dst;
    cv::Size wsize(7, 7);
    // normal mean filter
    double tick1 = (double)cv::getTickCount();
    meanFilter(src, dst, wsize);
    double tick2 = (double)cv::getTickCount();
    std::cout << "Normal mean filter: " << tick2 - tick1 << " ms.\n";
    // integral mean filter
    tick1 = (double)cv::getTickCount();
    meanFilterIntegral(src, dst, wsize);
    tick2 = (double)cv::getTickCount();
    std::cout << "Integral mean filter: " << tick2 - tick1 << " ms.\n";
    // boxFilter
    tick1 = (double)cv::getTickCount();
    cv::boxFilter(src, dst, -1, wsize);
    tick2 = (double)cv::getTickCount();
    std::cout << "OpenCV boxFilter: " << tick2 - tick1 << " ms.\n";
    
    cv::namedWindow("original");
    cv::imshow("original", src);
    cv::namedWindow("mean filter");
    cv::imshow("mean filter", dst);
    cv::imwrite("/Users/abc_mac/Desktop/Pillars_of_Creation_mean_filter.png", dst);
    if (cv::waitKey() == 27)
        return 0;
    
    return 0;
}
