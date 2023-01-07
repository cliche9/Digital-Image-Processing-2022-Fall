//
//  main.cpp
//  exp12
//
//  Created by abc_mac on 2023/1/1.
//

#include <opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
using namespace cv::xfeatures2d;

const std::string dir_path = "/Users/abc_mac/Desktop/";

void alphaBlending(int xStart, int xEnd, const cv::Mat &warp, cv::Mat &dst) {
    float overlapWidth = xEnd - xStart;
    float alpha = 1.0;
    for (int y = 0; y < dst.rows; ++y) {
        for (int x = xStart; x < xEnd; ++x) {
            if (warp.at<cv::Vec3b>(y, x) == cv::Vec3b(0, 0, 0))
                continue;
            alpha = (overlapWidth - (x - xStart)) / overlapWidth;
            dst.at<cv::Vec3b>(y, x) = dst.at<cv::Vec3b>(y, x) * alpha + warp.at<cv::Vec3b>(y, x) * (1.0 - alpha);
        }
    }
}

void imageStitching(cv::Mat &leftImage, cv::Mat &rightImage, cv::Mat &dst, bool toDraw) {
    // 1. 创建SURF特征检测器
    int hessian = 800;
    cv::Ptr<SURF> detector = SURF::create(hessian);
    
    // 2. 利用SURF进行图像特征检测
    std::vector<cv::KeyPoint> leftKeypoints, rightKeypoints;
    cv::Mat leftDescriptor, rightDescriptor;
    detector->detectAndCompute(leftImage, cv::Mat(), leftKeypoints, leftDescriptor);
    detector->detectAndCompute(rightImage, cv::Mat(), rightKeypoints, rightDescriptor);
    
    // 3. 使用FLANN库进行特征匹配
    cv::FlannBasedMatcher matcher;
    std::vector<cv::DMatch> matches;
    matcher.match(leftDescriptor, rightDescriptor, matches);
    
    // 4. 过滤筛选匹配程度高的关键点
    float threshold = 0;
    float ratio = 0.15;
    for (auto &match : matches)
        threshold = std::max(threshold, match.distance);
    threshold *= ratio;
    std::vector<cv::DMatch> goodMatches;
    std::vector<cv::Point2f> leftGoodKeypoints, rightGoodKeypoints;
    for (auto &match : matches) {
        if (match.distance < threshold) {
            leftGoodKeypoints.push_back(leftKeypoints[match.queryIdx].pt);
            rightGoodKeypoints.push_back(rightKeypoints[match.trainIdx].pt);
            goodMatches.push_back(match);
        }
    }
    
    if (toDraw) {
        cv::Mat result;
        /* 绘制特征点 */
        cv::drawKeypoints(leftImage, leftKeypoints, result);
        cv::imwrite(dir_path + "left_keypoints.png", result);
        cv::drawKeypoints(rightImage, rightKeypoints, result);
        cv::imwrite(dir_path + "right_keypoints.png", result);
        /* 绘制特征匹配 */
        cv::drawMatches(leftImage, leftKeypoints, rightImage, rightKeypoints, goodMatches, result);
        cv::imwrite(dir_path + "good_matches.png", result);
    }
    
    // 5. 计算两平面的射影变换矩阵
    if (leftGoodKeypoints.size() < 4 || rightGoodKeypoints.size() < 4)
        exit(1);
    
    cv::Mat H = cv::findHomography(rightGoodKeypoints, leftGoodKeypoints, cv::RANSAC);
    cv::Mat warp;
    cv::warpPerspective(rightImage, warp, H, cv::Size(leftImage.cols + rightImage.cols, leftImage.rows));
    cv::imwrite(dir_path + "warp.png", warp);
    
    // 6. 图像拼贴得到最终的全景图
    dst = warp.clone();
    leftImage.copyTo(dst(cv::Rect(0, 0, leftImage.cols, leftImage.rows)));
    cv::imwrite(dir_path + "direct_pasted.png", dst);
    
    // 7. 计算混合区域的坐标范围
    cv::Mat pointFrom = (cv::Mat_<double>(3, 1) << 0.0, 0.0, 1.0);    // 左上角原始坐标
    cv::Mat pointTo = H * pointFrom;    // 左上角变换后坐标
    cv::Point2f leftTop;
    leftTop.x = pointTo.at<double>(0, 0) / pointTo.at<double>(2, 0);
    if (leftTop.x < 0)
        leftTop.x = 0;
    
    pointFrom = (cv::Mat_<double>(3, 1) << 0.0, leftImage.rows, 1.0);   // 左下角原始坐标
    pointTo = H * pointFrom;    // 左下角变换后坐标
    cv::Point2f leftBottom;
    leftBottom.x = pointTo.at<double>(0, 0) / pointTo.at<double>(2, 0);
    if (leftBottom.x < 0)
        leftBottom.x = 0;
    
    float xStart = std::min(leftTop.x, leftBottom.x);
    float xEnd = warp.cols;
    
    // 8 图像alpha混合消除接缝
    alphaBlending(xStart, xEnd, warp, dst);
    cv::imwrite(dir_path + "alpha_blending.png", dst);
}

void clipToFullscreen(cv::Mat &dst){
    // 全景图轮廓提取
    cv::Mat stitched, gray, thresh;
    std::vector<std::vector<cv::Point>> cnts;
    cv::copyMakeBorder(dst, stitched, 10, 10, 10, 10, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    cv::cvtColor(stitched, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY);
    cv::findContours(thresh, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // 轮廓最大外接正矩形
    cv::Mat mask = cv::Mat::zeros(thresh.size(), CV_8U);
    cv::Rect bound = cv::boundingRect(cnts[0]);
    cv::rectangle(mask, bound, 255, -1);
    // 腐蚀处理, 直到完全去除黑边
    cv::Mat maxRect = mask.clone();
    cv::Mat sub = mask.clone();
    int count = 0;
    while (cv::countNonZero(sub) > 0) {
        ++count;
        cv::erode(maxRect, maxRect, NULL);
        cv::subtract(maxRect, thresh, sub);
        std::cout << "loop " << count << std::endl;
    }
    // 提取maxRect轮廓并裁减
    cv::findContours(maxRect, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    bound = cv::boundingRect(cnts[0]);
    dst = stitched(bound);
    cv::imwrite(dir_path + "clipped.png", dst);
}


int main(int argc, const char * argv[]) {
    std::vector<std::string> filenames = { "1.png", "2.png", "3.png" };
    cv::Mat src = cv::imread(dir_path + filenames[0]);
    cv::Mat dst;
    for (int i = 1; i < filenames.size(); ++i) {
        cv::Mat append = cv::imread(dir_path + filenames[i]);
        imageStitching(src, append, dst, true);
        // clipToFullscreen(dst);
        src = dst.clone();
    }
    
    return 0;
}