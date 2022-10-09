#include "opencv4/opencv2/opencv.hpp"
#include <iostream>
using namespace std;
using namespace cv;

Vec3b resampling(const Mat &src, Point2d point) {
    int x0 = floor(point.x), y0 = floor(point.y);
    int x1 = x0 + 1, y1 = y0 + 1;
    
    /** bilinear interpolation */
    return
        (src.at<Vec3b>(y0, x0) * (x1 - point.x) + src.at<Vec3b>(y0, x1) * (point.x - x0)) * (y1 - point.y)
      + (src.at<Vec3b>(y1, x0) * (x1 - point.x) + src.at<Vec3b>(y1, x1) * (point.x - x0)) * (point.y - y0);
}

void convexLens(const Mat &src, Mat &dst) {
    int width = src.cols, height = src.rows;
    /** define a circle area for transformation */
    Point2d center(width / 2, height / 2);                              // center of transformation area
    double radius = sqrt(width * width + height * height) / 2;          // radius of this area

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double dist = norm(Point2d(x, y) - center);
            /** check if (x, y) is in the convex lens */
            if (dist <= radius) {
                /** compute the transformed position */
                Point2d new_pos = center + (dist / radius) * ((Point2d(x, y) - center));
                /** bilinear resampling */
                dst.at<Vec3b>(y, x) = resampling(src, new_pos);
            }
        }
    }
}

int main() {
    VideoCapture cap(0);
    Mat source, transformed;
    /** get frame size and fps of the camera */
    Size frame_size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);
    // cout << frame_size << fps;
    VideoWriter writer("/Users/abc_mac/Desktop/convexLens.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), fps / 3, frame_size);
    
    while (true) {
        cap >> source;
        // imshow("camera0", source);
        source.copyTo(transformed);
        convexLens(source, transformed);
        writer.write(transformed);
        imshow("凸透镜", transformed);
        if (waitKey(1) == 27)
            break;
    }
    destroyWindow("凸透镜");
    writer.release();

    return 0;
}
