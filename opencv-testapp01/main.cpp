#include <cstdio>
#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;
using namespace std;

int main()
{
    Mat img = imread("cat_224x224.jpg", IMREAD_COLOR);
    if (img.empty()) {
        printf("Image not found !");
        return -1;
    }


    imshow("Display cat", img);
    int k = waitKey(0);
    if (k == 's')
        imwrite("cat_224x224_1.jpg", img);

    printf("Вас приветствует OpenCV %s!\n", CV_VERSION);
    return 0;
}