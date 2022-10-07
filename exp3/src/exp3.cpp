#include "opencv4/opencv2/opencv.hpp"
#include <iostream>
using namespace std;
using namespace cv;

void resampling(const Mat &input) {

}

void convexLens(Mat src, Mat dst) {
    int width = src.cols, height = src.rows;
    /** define a circle area for transformation */
    Point2d center(width / 2, height / 2);                          // center of transformation area
    int radius = sqrt(width * width + height * height) * 0.15;      // radius of this area

    for (int y = 0; y < height; ++y) {
        uchar *row_data = dst.ptr<uchar>(y);
        for (int x = 0; x < width; ++x) {
            int dist = norm(Point2d(x, y) - center);
            Point2d new_p = center + dist / radius * (Point2d(x, y) - center) / 2;
            row_data[3 * x] = src.at<uchar>(new_p.y, 3 * new_p.x);
            row_data[3 * x + 1] = src.at<uchar>(new_p.y, 3 * new_p.x + 1);
            row_data[3 * x + 2] = src.at<uchar>(new_p.y, 3 * new_p.x + 2);
        }
    }
}

int main() {
    VideoCapture cap;
    cap.open(0);
    Mat source, transformed;
    
    while (true) {
        cap >> source;
        source.copyTo(transformed);
        convexLens(source, transformed);
        imshow("凸透镜", transformed);
        waitKey(15);
    }

    return 0;
}