//
//  main.cpp
//  BlockMatching
//
//  Created by kuno_lab on 2014/02/17.
//  Copyright (c) 2014年 kuno_lab. All rights reserved.
//

#include <iostream>
#include <cv.h>
#include <highgui.h>
#include "block.h"
#include "graph.h"

using namespace std;

#define IMAGE1 "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/test3.bmp"
#define IMAGE2 "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/test4.bmp"

#define SEARCH_SIZE 5

struct ncost
{
    double *_datas;
    int _label_size;
    int _width, _height;
    ncost(int height, int width, int label_size) {
        _label_size = label_size;
        _width = width;
        _height = height;
        _datas = (double *)malloc(sizeof(double) * width * height * label_size);
    }
    void setCost(int y, int x, int label, double data) {
        _datas[_label_size*(y*_width+x)+label] = data;
    }
    double getCost(int y, int x, int label) {
        return _datas[_label_size*(y*_width+x)+label];
    }
};

int getLabel(int y, int x, int search_size = 5)
{
    int sh = (search_size-1)/2;
    return (y+sh)*search_size+x+sh;
}

void block_match(IplImage *prev, IplImage *cur, IplImage *dist, struct ncost& cost, int block_size = 5, int search_size = 5)
{
    int height = prev->height;
    int width = prev->width;
    int bh = (search_size-1)/2;
    int sh = (search_size-1)/2;
    for (int h=0;h<height;h++) {
        for (int w=0;w<width;w++) {
//            prev->imageData[h*prev->widthStep+w*prev->nChannels]; // 探索元ピクセル
            double min_diff = INT_MAX;
            int min_x, min_y;
            for (int cy=h-sh;cy<=h+sh;cy++) {
                for (int cx=w-sh;cx<=w+sh;cx++) {
                    // 画像範囲外は無視
                    if (cy < 0 || cy >= height || cx < 0 || cx >= width) continue;
                    double diff_sum = 0;
                    int count = 0;
                    for (int y=cy-bh;y<=cy+bh;y++) {
                        for (int x=cx-bh;x<=cx+bh;x++) {
                            if (y < 0 || y >= height || x < 0 || x >= width) continue;
                            int diff = 0;
                            for (int c=0;c<3;c++) {
                                diff += abs(prev->imageData[h*prev->widthStep+w*prev->nChannels+c]
                                        - cur->imageData[y*cur->widthStep+x*cur->nChannels+c]);
                            }
                            diff_sum += diff;
                            count++;
                        }
                    }
                    if (diff_sum/count < min_diff) {
                        min_diff = diff_sum/count;
                        min_x = cx;
                        min_y = cy;
                    }
                }
            }
            cost.setCost(h, w, 0, min_diff);
            dist->imageData[h*dist->widthStep+w*dist->nChannels + 0] = char(min_x - w);
            dist->imageData[h*dist->widthStep+w*dist->nChannels + 1] = char(min_y - h);
            dist->imageData[h*dist->widthStep+w*dist->nChannels + 2] = 0;
        }
        cout << "." << flush;
    }
}

void alpha_extension(IplImage *src, struct ncost &cost, IplImage *dist, int search_size = 5)
{
    int height = src->height;
    int width = src->width;
    typedef Graph<int, int, int> GraphType;
    
    GraphType *g = new GraphType(height*width, height*width*4);
    
    g->add_node(height*width);
    
    for (int alpha = 0;alpha<search_size*search_size;alpha++) {
        for (int i=0;i<height*width;i++) {
            // データ項の付与
            int y = i/width;
            int x = i%width;
            int xx = src->imageData[y*src->widthStep+x*src->nChannels];
            int yy = src->imageData[y*src->widthStep+x*src->nChannels + 1];
            int cur_label = getLabel(yy, xx);
            g->add_tweights(i, cost.getCost(y, x, cur_label), cost.getCost(y, x, alpha));
        }
        for (int i=0;i<(height-1)*width;i++) {
            // 平滑化項の付与(横)
            int y = i/width;
            int x = i%width;
            int xx = src->imageData[y*src->widthStep+x*src->nChannels];
            int yy = src->imageData[y*src->widthStep+x*src->nChannels + 1];

//            g->add_tweights(i, cost.getCost(y, x, cur_label), cost.getCost(y, x, alpha));
//        }
        }
    }
}

int main(int argc, const char * argv[])
{
    IplImage *img1 = cvLoadImage(IMAGE1);
    IplImage *img2 = cvLoadImage(IMAGE2);
    IplImage *ret = cvCreateImage(cvGetSize(img1), img1->depth, 3);
    cvShowImage("image 1", img1);
    cvShowImage("image 2", img2);

    struct ncost nc = ncost(img1->height, img2->width, SEARCH_SIZE*SEARCH_SIZE);
    block_match(img1, img2, ret, nc);
    cvShowImage("ret", ret);
    
    cvWaitKey();
    
    return 0;
}

