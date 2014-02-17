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
#include <math.h>

using namespace std;

#define IMAGE1 "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/test3.bmp"
#define IMAGE2 "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/test4.bmp"

#define SEARCH_SIZE 5
#define BLOCK_SIZE 5
#define COLOR_VARIATION 12

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

int getLabel(IplImage *src, int y, int x)
{
    int xx = (char)src->imageData[y*src->widthStep+x*src->nChannels];
    int yy = (char)src->imageData[y*src->widthStep+x*src->nChannels + 1];
    
    int sh = (SEARCH_SIZE-1)/2;
    return (yy+sh)*SEARCH_SIZE+xx+sh;
}

int getDist(int L1, int L2)
{
    int x1 = L1%SEARCH_SIZE;
    int y1 = L1/SEARCH_SIZE;
    int x2 = L2%SEARCH_SIZE;
    int y2 = L2/SEARCH_SIZE;
    
    return max(abs(x1-x2), abs(y1-y2));
}

void setLabel(IplImage *src, int y, int x, int label)
{
    int yy = label/SEARCH_SIZE - (SEARCH_SIZE-1)/2;
    int xx = label%SEARCH_SIZE - (SEARCH_SIZE-1)/2;
    
    src->imageData[y*src->widthStep+x*src->nChannels] = (char)xx;
    src->imageData[y*src->widthStep+x*src->nChannels + 1] = (char)yy;
    
}

void block_match(IplImage *prev, IplImage *cur, IplImage *dst, struct ncost& cost)
{
    int height = prev->height;
    int width = prev->width;
    int bh = (SEARCH_SIZE-1)/2;
    int sh = (SEARCH_SIZE-1)/2;
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
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 0] = char(min_x - w);
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 1] = char(min_y - h);
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 2] = 0;
        }
        cout << "." << flush;
    }
}

void HSVtoRGB(int h, int s, int v, int &r, int &g, int &b){
    float f;
    int i, p, q, t;

    i = (int)(h / 60.0f) % 6;
    f = (float)(h / 60.0f) - (float)(h / 60.0f);
    p = (int)(v * (1.0f - (s / 255.0f)));
    q = (int)(v * (1.0f - (s / 255.0f) * f));
    t = (int)(v * (1.0f - (s / 255.0f) * (1.0f - f)));
    
    switch(i){
        case 0 : r = v; g = t; b = p; break;
        case 1 : r = q; g = v; b = p; break;
        case 2 : r = p; g = v; b = t; break;
        case 3 : r = p; g = q; b = v; break;
        case 4 : r = t; g = p; b = v; break;
        case 5 : r = v; g = p; b = q; break;
    }
}

CvScalar getColor(int x, int y)
{
    double dist = sqrt(x*x+y*y);
    double theta = acos((double)x/dist);

    if (y < 0) {
        theta = 2*M_PI - theta;
    }
    int H = 180*theta/M_PI;
    int r,g,b;
    HSVtoRGB(H, 255, 255, r, g, b);
    return cvScalar(b, g, r);
}

void convert2flow(IplImage *src, IplImage *label, IplImage *dst)
{
#define INTERVAL 8 //フロー間隔
#define EMP 2 //フロー強調
    int height = src->height;
    int width = src->width;

    cvCopy(src, dst);
    
    
    
    for (int h=0;h<height;h+=INTERVAL) {
        for (int w=0;w<width;w+=INTERVAL) {
            int x = (char)label->imageData[h*label->widthStep+w*label->nChannels + 0];
            int y = (char)label->imageData[h*label->widthStep+w*label->nChannels + 1];
            getColor(x, y);
            cvLine(dst, cvPoint(w, h), cvPoint(w+x*EMP, h+y*EMP), getColor(x, y));
            cvCircle(dst, cvPoint(w+x*EMP, h+y*EMP), 1, getColor(x, y));
        }
    }
}

void alpha_extension(IplImage *src, struct ncost &cost, IplImage *dst)
{
    int height = src->height;
    int width = src->width;

    int E_t;
    int E = INT_MAX;
    
    for (int alpha = 0;alpha<SEARCH_SIZE*SEARCH_SIZE;alpha++) {
        typedef Graph<int, int, int> GraphType;
        
        GraphType *g = new GraphType(height*width, height*width*4);
        
        for (int i=0;i<height*width;i++) {
            g->add_node();
            // データ項の付与
            int y = i/width;
            int x = i%width;
            
            int cur_label = getLabel(src, y, x);
            g->add_tweights(i, cost.getCost(y, x, cur_label), cost.getCost(y, x, alpha));
        }
        for (int i=0;i<width*(height-1);i++) {
            g->add_node();
            // 平滑化項の付与(縦)
            int y = i/width;
            int x = i%width;
            
            g->add_tweights(i+width*height, getDist(getLabel(src, y, x) ,getLabel(src, y+1, x)), 0);
            g->add_edge(i, i+width*height, getDist(getLabel(src, y, x), alpha), getDist(getLabel(src, y, x), alpha));
            g->add_edge(i+width, i+width*height, getDist(getLabel(src, y+1, x), alpha), getDist(getLabel(src, y+1, x), alpha));
        }
        for (int i=0;i<width*height;i++) {
            g->add_node();
            if (i%(width-1) == 0) continue;
            // 平滑化項の付与(横)
            int y = i/width;
            int x = i%width;
            
            g->add_tweights(i+width*height+width*(height-1), getDist(getLabel(src, y, x) ,getLabel(src, y, x+1)), 0);
            g->add_edge(i, i+width*height+width*(height-1), getDist(getLabel(src, y, x), alpha), getDist(getLabel(src, y, x), alpha));
            g->add_edge(i+1, i+width*height+width*(height-1), getDist(getLabel(src, y, x+1), alpha), getDist(getLabel(src, y, x+1), alpha));
        }
        
        int flow = g->maxflow();
        E_t = flow;
        
        if (E>E_t) {
            E = E_t;
            cout << "Update: Flow = " << E_t << endl;
            for (int i=0;i<width*height;i++) {
                if (g->what_segment(i) == GraphType::SOURCE) {
                    int y = i/width;
                    int x = i%width;
                    setLabel(src, y, x, alpha);
                }
            }
            cvShowImage("fdfdf", src);
            cvWaitKey();
            
        }
        delete g;
    }
}

int main(int argc, const char * argv[])
{
    IplImage *img1 = cvLoadImage(IMAGE1);
    IplImage *img2 = cvLoadImage(IMAGE2);
    IplImage *ret = cvCreateImage(cvGetSize(img1), img1->depth, 3);
    IplImage *result = cvCreateImage(cvGetSize(img1), img1->depth, 3);
    
    cvShowImage("image 1", img1);
    cvShowImage("image 2", img2);

    struct ncost cost = ncost(img1->height, img2->width, SEARCH_SIZE*SEARCH_SIZE);
    block_match(img1, img2, ret, cost);
    cvShowImage("ret", ret);
    
    convert2flow(img1, ret, result);
    
    cvShowImage("resu", result);
    
    cvWaitKey();
    
    alpha_extension(ret, cost, result);
    convert2flow(img1, ret, result);
    cvShowImage("result", result);

    cvWaitKey();
    return 0;
}

