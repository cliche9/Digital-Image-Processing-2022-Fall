//
//  main.cpp
//  exp4
//
//  Created by abc_mac on 2022/10/12.
//

#include <opencv.hpp>
#include <imgproc.hpp>
#include <iostream>
#include <vector>

cv::Mat original, transformed;
std::vector<cv::Point2f> affine_points_in_src, affine_points_in_dst;
int selected = -1;
double circle_radius = 5;
cv::Scalar circle_color(255, 255, 255);

static int indexOfPointAccessed(int x, int y) {
    for (int i = 0; i < 4; ++i) {
        if (cv::pow(affine_points_in_dst[i].x - x, 2) + cv::pow(affine_points_in_dst[i].y - y, 2) < cv::pow(circle_radius, 2)) {
            return i;
        }
    }
    return -1;
}

static void onMouseMoveTransformation(int event, int x, int y, int flags, void *param) {
    switch (event) {
        case cv::EVENT_LBUTTONDOWN: {
           selected = indexOfPointAccessed(x, y);
           break;
        } case cv::EVENT_LBUTTONUP: {
           selected = -1;
           break;
        } case cv::EVENT_MOUSEMOVE: {
            // point has been selected
            if (selected >= 0) {
                affine_points_in_dst[selected] = cv::Point2f(x, y);
                /** get matrix of afffine transformation */
                cv::Mat trans_matrix = cv::getPerspectiveTransform(affine_points_in_src, affine_points_in_dst);
                cv::warpPerspective(original, transformed, trans_matrix, cv::Size(original.cols, original.rows));
                /** draw control points */
                for (int i = 0; i < 4; ++i) {
                    cv::circle(transformed, affine_points_in_dst[i], circle_radius, circle_color, cv::FILLED);
                    line(transformed, affine_points_in_dst[i], affine_points_in_dst[(i + 1) % 4], circle_color, circle_radius / 2, cv::LINE_AA);
                }
                cv::imshow("transformation", transformed);
                if (cv::waitKey() == 27)
                    exit(0);
            }
        } default:
            break;
    }
}

int main(int argc, const char * argv[]) {
    original = cv::imread("/Users/abc_mac/Desktop/img_for_test.jpg");
    int width = original.cols, height = original.rows;
    /** sample points in src image and dst image */
    affine_points_in_src = {
        cv::Point2f(0.0, 0.0),
        cv::Point2f(0.0, height),
        cv::Point2f(width, height),
        cv::Point2f(width, 0.0)
    };
    
    affine_points_in_dst = {
        cv::Point2f(width / 3.0, 20.0),
        cv::Point2f(20.0, height - 20.0),
        cv::Point2f(width - 20.0, height - 20.0),
        cv::Point2f(2 * width / 3.0, 20.0)
    };
    
    /** get matrix of afffine transformation */
    cv::Mat trans_matrix = cv::getPerspectiveTransform(affine_points_in_src, affine_points_in_dst);
    cv::warpPerspective(original, transformed, trans_matrix, cv::Size(original.cols, original.rows));
    /** draw control points */
    for (int i = 0; i < 4; ++i) {
        cv::circle(transformed, affine_points_in_dst[i], circle_radius, circle_color, cv::FILLED);
        line(transformed, affine_points_in_dst[i], affine_points_in_dst[(i + 1) % 4], circle_color, circle_radius / 2, cv::LINE_AA);
    }
    
    cv::namedWindow("transformation", cv::WINDOW_AUTOSIZE);
    cv::imshow("original", original);
    cv::imshow("transformation", transformed);
    cv::setMouseCallback("transformation", onMouseMoveTransformation);
    if (cv::waitKey() == 27)            // press ESC to exit
        exit(0);

    return 0;
}
