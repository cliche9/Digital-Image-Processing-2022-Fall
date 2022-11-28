//
//  main.cpp
//  exp6
//
//  Created by abc_mac on 2022/10/28.
//

#include <opencv.hpp>
#include <iostream>

/**
 pixel color perspective, create a range weight table
 */
void createRangeWeightMask(std::vector<double> &range_mask, double sigma_r) {
    for (int i = 0; i < 256; ++i) {
        double range_diff = exp( - (i * i) / (2 * sigma_r * sigma_r) );
        range_mask.push_back(range_diff);
    }
}

/**
 spatial perspective, create a spatial weight table
 */
void createSpatialWeightMask(cv::Mat &spatial_mask, cv::Size ksize, double sigma_s) {
    spatial_mask.create(ksize, CV_64F);
    cv::Size gaussian_center( (ksize.width - 1) / 2, (ksize.height - 1) / 2 );
    double x_square, y_square;
    
    for (int y = 0; y < ksize.height; ++y) {
        y_square = pow(y - gaussian_center.height, 2);
        for (int x = 0; x < ksize.width; ++x) {
            x_square = pow(x - gaussian_center.width, 2);
            spatial_mask.at<double>(y, x) = exp( - (x_square + y_square) / (2 * sigma_s * sigma_s) );
        }
    }
}

void bilateralFilter(cv::Mat &src, cv::Mat &dst, cv::Size ksize, double sigma_r, double sigma_s) {
    dst.create(src.size(), src.type());
    cv::Mat spatial_mask;
    std::vector<double> range_mask;
    createSpatialWeightMask(spatial_mask, ksize, sigma_s);
    createRangeWeightMask(range_mask, sigma_r);
    // padding
    int vertical = (ksize.height - 1) / 2;
    int horizontal = (ksize.width - 1) / 2;
    cv::Mat padding_src;
    cv::copyMakeBorder(src, padding_src, vertical, vertical, horizontal, horizontal, cv::BORDER_REFLECT_101);

    int channels = src.channels();
    bool isGrayGraph = channels == 1;
    cv::Mat mask[3] = { cv::Mat::zeros(ksize, CV_64F), cv::Mat::zeros(ksize, CV_64F), cv::Mat::zeros(ksize, CV_64F) };
    std::vector<int> color_diff(channels, 0);          // color difference between pixel (y, x) and the center pixel
    std::vector<double> color(channels, 0.0f);         // final color for point(y, x)
    std::vector<double> kernel_sum(channels, 0.0f);    // sum of kernel range values, just for normalization
    // loop for traversing pixels
    for (int y = vertical; y < src.rows + vertical; ++y) {
        for (int x = horizontal; x < src.cols + horizontal; ++x) {
            std::fill(color_diff.begin(), color_diff.end(), 0);
            std::fill(color.begin(), color.end(), 0);
            std::fill(kernel_sum.begin(), kernel_sum.end(), 0);
            // loop for the filter kernel
            for (int i = -vertical; i <= vertical; ++i) {
                for (int j = -horizontal; j <= horizontal; ++j) {
                    // get kernel color difference
                    if (isGrayGraph) {
                        int kernel_center = padding_src.at<uchar>(y, x);
                        int pixel_now = padding_src.at<uchar>(y + i, x + j);
                        color_diff[0] = abs(pixel_now - kernel_center);
                    } else {
                        cv::Vec3b kernel_center = padding_src.at<cv::Vec3b>(y, x);
                        cv::Vec3b pixel_now = padding_src.at<cv::Vec3b>(y + i, x + j);
                        for (int idx = 0; idx < 3; ++idx)
                            color_diff[i] = abs(pixel_now[i] - kernel_center[i]);
                    }
                    // compute kernel mask value
                    for (int idx = 0; idx < channels; ++idx) {
                        mask[idx].at<double>(i + vertical, j + horizontal) = range_mask[color_diff[idx]] * spatial_mask.at<double>(i + vertical, j + horizontal);
                        kernel_sum[idx] += mask[idx].at<double>(i + vertical, j + horizontal);
                        // std::cout << mask[idx].at<double>(i + vertical, j + horizontal) << std::endl;
                    }
                }
            }
            // normalization
            for (int idx = 0; idx < channels; ++idx)
                mask[idx] /= kernel_sum[idx];
            // implement the bilateral filter equation
            for (int i = -vertical; i <= vertical; ++i) {
                for (int j = -horizontal; j <= horizontal; ++j) {
                    if (isGrayGraph)
                        color[0] += padding_src.at<uchar>(y + i, x + j) * mask[0].at<double>(i + vertical, j + horizontal);
                    else {
                        cv::Vec3b pixel_now = padding_src.at<cv::Vec3b>(y + i, x + j);
                        for (int idx = 0; idx < 3; ++idx)
                            color[idx] += pixel_now[idx] * mask[idx].at<double>(i + vertical, j + horizontal);
                    }
                }
            }
            // clamp to [0, 255]
            for (int idx = 0; idx < channels; ++idx) {
                if (color[idx] < 0)
                    color[idx] = 0;
                else if (color[idx] > 255)
                    color[idx] = 255;
            }
            // update image pixel color
            if (isGrayGraph)
                dst.at<uchar>(y - vertical, x - horizontal) = static_cast<uchar>(color[0]);
            else {
                cv::Vec3b pixel_color;
                for (int idx = 0; idx < 3; ++idx)
                    pixel_color[idx] = static_cast<uchar>(color[idx]);
                dst.at<cv::Vec3b>(y - vertical, x - horizontal) = pixel_color;
            }
        }
    }
}

int main(int argc, const char * argv[]) {
    std::string src_path = "/Users/abc_mac/Desktop/Angry_Cat";
    cv::Mat src = cv::imread(src_path + ".png");
    if (src.empty())
        return -1;
    
    cv::Mat dst;
    cv::Size ksize(23, 23);
    double sigma_r = 35, sigma_s = 10;
    // gaussian filter
    cv::GaussianBlur(src, dst, ksize, sigma_s);
    cv::imwrite(src_path + "_gaussian_filter.png", dst);
    // custom bilateral filter
    double tick1 = (double)cv::getTickCount();
    bilateralFilter(src, dst, ksize, sigma_r, sigma_s);
    double tick2 = (double)cv::getTickCount();
    std::cout << "Custom bilateral filter: " << tick2 - tick1 << " ms.\n";
    cv::imwrite(src_path + "_custom_bilateral_filter.png", dst);
    // cv::bilateralFilter
    tick1 = (double)cv::getTickCount();
    cv::bilateralFilter(src, dst, ksize.width, sigma_r, sigma_s);
    tick2 = (double)cv::getTickCount();
    std::cout << "OpenCV bilateral filter: " << tick2 - tick1 << " ms.\n";
    cv::imwrite(src_path + "_opencv_bilateral_filter.png", dst);
    
    return 0;
}
