#include "opencv4/opencv2/opencv.hpp"
#include <iostream>
using namespace std;
using namespace cv;

Mat source, transformed, background, mask;
int cValue, bValue, value;

void Sigmoid(int, void*) {
    namedWindow("Image", WINDOW_AUTOSIZE);

    for (int y = 0; y < source.rows; ++y) {
        for (int x = 0; x < source.cols; ++x) {
            for (int c = 0; c < 3; ++c) {
                double t = ((source.at<Vec3b>(y, x)[c] - 127) / 255.0) * cValue * 0.1;
                transformed.at<Vec3b>(y, x)[c] = 
                    saturate_cast<uchar>(
                        source.at<Vec3b>(y, x)[c] * ((1.0 / (1.0 + exp(-t))) + 0.3) + bValue - 100
                    );
            }
        }
    }

    imshow("Original Image", source);
    imshow("Contrast && Brightness", transformed);
}

void maskFunc(int, void*) {
    double sum = 0.0;
    for (int y = 0; y < source.rows; ++y) {
        for (int x = 0; x < source.cols; ++x) {
            sum = 0.0;
            for (int c = 0; c < 3; ++c) {
                /** brutal method
                mask.at<Vec3b>(y, x)[c] = max(0, source.at<Vec3b>(y, x)[c] - background.at<Vec3b>(y, x)[c]);
                */
                /** median filter */
                sum += pow((source.at<Vec3b>(y, x)[c] - background.at<Vec3b>(y, x)[c]), 2);
            }

            sum = sqrt(sum);
            if (sum >= value) {
                for (int c = 0; c < 3; ++c) {
                    mask.at<Vec3b>(y, x)[c] = 255;
                }
            } else {
                for (int c = 0; c < 3; ++c) {
                    mask.at<Vec3b>(y, x)[c] = 0;
                }
            }
        }
    }
    
    imshow("Slider", mask);
}

void contrastAndBrightnessTest() {
    source = imread("../res/01.jpg");
    transformed = Mat::zeros(source.size(), source.type());

    cValue = 20;
    bValue = 100;

    // slider window for contrast & brightness ajustment
    namedWindow("Slider", WINDOW_AUTOSIZE);
    createTrackbar("Constract", "Slider", &cValue, 200, Sigmoid);
    createTrackbar("Brightness", "Slider", &bValue, 200, Sigmoid);

    Sigmoid(cValue, 0);
    Sigmoid(bValue, 0);
    waitKey();
}

void maskTest() {
    // mask exp
    source = imread("../res/08.jpg");
    background = imread("../res/08_bg.jpg");
    namedWindow("Slider", WINDOW_AUTOSIZE);
    // mask image
    mask = Mat::zeros(source.size(), source.type());
    // slider for getting front objects
    createTrackbar("Mask", "Slider", &value, 200, maskFunc);
    maskFunc(value, 0);

    waitKey();
}

int main() {
    maskTest();
    return 0;
}