//
//  main.cpp
//  exp11
//
//  Created by abc_mac on 2022/12/5.
//

#include <iostream>
#include <opencv.hpp>

const std::string dir_path = "/Users/abc_mac/Desktop/";

class pyramidFusion {
public:
    // @constructor
    pyramidFusion(cv::Mat &aImage, cv::Mat &bImage, int level) {
        aSource = aImage.clone();
        bSource = bImage.clone();
        this->level = level;
    }
    
    void initAllPyramids() {
        initGaussianPyramid(aSource, aGaussian);
        initLaplacianPyramid(aGaussian, aLaplacian);
        initGaussianPyramid(bSource, bGaussian);
        initLaplacianPyramid(bGaussian, bLaplacian);
    }
    
    void blend() {
        for (int i = 0; i < level; ++i) {
            int width = aLaplacian[i].cols, height = aLaplacian[i].rows;
            cv::Mat bRegion = bLaplacian[i](cv::Rect(width / 2, 0, width / 2, height));
            cv::Mat lpls = aLaplacian[i].clone();
            cv::Mat rightRegion = lpls(cv::Rect(width / 2, 0, width / 2, height));
            bRegion.copyTo(rightRegion);
            mixedLaplacian.push_back(lpls);
        }
    }
    
    cv::Mat reconstruct() {
        cv::Mat mixed = mixedLaplacian[0];
        for (int i = 1; i < level; ++i) {
            // L(i) = G(i) + PyrUp(G(i + 1));
            cv::pyrUp(mixed, mixed, mixedLaplacian[i].size());
            cv::add(mixed, mixedLaplacian[i], mixed);
        }
        return mixed;
    }
    
    
private:
    cv::Mat aSource, bSource;
    int level;
    std::vector<cv::Mat> aGaussian, bGaussian;
    std::vector<cv::Mat> aLaplacian, bLaplacian, mixedLaplacian;
    
    // create gaussian pyramid
    void initGaussianPyramid(cv::Mat &src, std::vector<cv::Mat> &gaussian) {
        // down sampling map (dsm)
        cv::Mat dsm = src.clone();
        dsm.convertTo(dsm, CV_32F);
        gaussian.push_back(dsm);
        for (int i = 1; i < level; ++i) {
            cv::pyrDown(dsm, dsm);      // gaussian blur & downsampling
            gaussian.push_back(dsm);
        }
    }
    
    // create lapalian pyramid
    void initLaplacianPyramid(std::vector<cv::Mat> &gaussian, std::vector<cv::Mat> &laplacian) {
        // up sampling map (usm)
        // laplacian (lpls)
        cv::Mat usm, lpls;
        laplacian.push_back(gaussian[level - 1]);
        for (int i = level - 1; i > 0; --i) {
            cv::pyrUp(gaussian[i], usm, gaussian[i - 1].size());
            cv::subtract(gaussian[i - 1], usm, lpls);
            laplacian.push_back(lpls);
        }
    }
};

int main(int argc, const char * argv[]) {
    cv::Mat aSrc = cv::imread(dir_path + "red.png");
    cv::Mat bSrc = cv::imread(dir_path + "blue.png");
    int width = aSrc.cols, height = aSrc.rows;
    cv::Mat bRegion = bSrc(cv::Rect(width / 2, 0, width / 2, height));
    cv::Mat brutalFusion = aSrc.clone();
    cv::Mat rightRegion = brutalFusion(cv::Rect(width / 2, 0, width / 2, height));
    bRegion.copyTo(rightRegion);
    cv::imwrite(dir_path + "brutal_result.png", brutalFusion);
    
    pyramidFusion fusion(aSrc, bSrc, 6);
    fusion.initAllPyramids();
    fusion.blend();
    cv::Mat pyramidFusion = fusion.reconstruct();
    cv::imwrite(dir_path + "pyramid_result.png", pyramidFusion);
    
    return 0;
}
