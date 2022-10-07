#include "opencv4/opencv2/opencv.hpp"
#include <iostream>
using namespace std;
using namespace cv;

/** load a image and show it
 * title: Image Name
 * img_url: Image Url
 */ 
void showImage(const string &title, const cv::Mat &input) {
    cv::imshow(title, input);
    waitKey(0);
}

/** get one channel from the input image */
void getChannel(const uchar *input, int width, int height, int inStep, int inChannels, uchar *output, int outStep, int channelToGet) {
    for (int y = 0; y < height; ++y, output += outStep) {
        for (int x = 0; x < width; ++x, input += inChannels) {
            output[x] = input[channelToGet];
        }
    }
}

/** alpha mixture using background image */ 
void alphaMixture(cv::Mat &src, const cv::Mat &background, cv::Mat &out) {
    for (int x = 0; x < src.rows; ++x) {
        for (int y = 0; y < src.cols; ++y) {
            double alpha = 0.5;             // can be randomly set
            out.at<Vec4b>(x, y)[0] = 
                (1 - alpha) * background.at<Vec4b>(x, y)[0] +
                alpha * src.at<Vec4b>(x, y)[0];
            out.at<Vec4b>(x, y)[1] = 
                (1 - alpha) * background.at<Vec4b>(x, y)[1] +
                alpha * src.at<Vec4b>(x, y)[1];
            out.at<Vec4b>(x, y)[2] = 
                (1 - alpha) * background.at<Vec4b>(x, y)[2] +
                alpha * src.at<Vec4b>(x, y)[2];
            out.at<Vec4b>(x, y)[3] = 
                (1 - alpha) * background.at<Vec4b>(x, y)[3] +
                alpha * src.at<Vec4b>(x, y)[3];    
        }
    }
}

int main() {
    // test of exp1.1
    cv::Mat input = cv::imread("res/img1.png", IMREAD_UNCHANGED);
    showImage("Input Image", input);
    // test of exp1.2
    int width = input.cols;
    int height = input.rows;
    int inChannels = input.channels();
    int step = input.step;

    cv::Mat output(input.size(), CV_8UC1);
    // get alpha channel
    getChannel(input.data, width, height, step, inChannels, output.data, output.step, 3);
    showImage("Alpha Channel", output);
    // test of exp1.3
    cv::Mat background = imread("res/background.jpg", IMREAD_UNCHANGED);
    cv::Mat mixed(height, width, CV_8UC4);
    alphaMixture(input, background, mixed);
    showImage("Mixed Image", mixed);
    
    return 0;
}
