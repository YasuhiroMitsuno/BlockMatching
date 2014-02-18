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

#define INITIAL_BLOCK_MATCH 1

#define LAMBDA 120
#define BIAS 1

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

int getLabel(int x, int y)
{
    int sh = SEARCH_SIZE/2;
    return (y+sh)*SEARCH_SIZE+x+sh;
}

int getLabel(IplImage *src, int y, int x)
{
    int xx = (char)src->imageData[y*src->widthStep+x*src->nChannels];
    int yy = (char)src->imageData[y*src->widthStep+x*src->nChannels + 1];
    
    return getLabel(xx, yy);
}

/*
    ret: 0 ~ 360
 */
int getRadian(int x, int y)
{
    double dist = sqrt(x*x+y*y);
    double theta = acos((double)x/dist);
    
    if (y < 0) {
        theta = 2*M_PI - theta;
    }
    return int(180*theta/M_PI);
}

double getDist(int L1, int L2)
{
    int x1 = L1%SEARCH_SIZE;
    int y1 = L1/SEARCH_SIZE;
    int x2 = L2%SEARCH_SIZE;
    int y2 = L2/SEARCH_SIZE;
    
    int r1 = getRadian(x1, y1);
    int r2 = getRadian(x2, y2);
    
    int theta = r1-r2;
    if (theta < 0) theta += 180;
    if (theta > 180) theta -= 180;
    
    if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
        return  BIAS * max(abs(x1-x2), abs(y1-y2));
    }

    return  max(abs(x1-x2), abs(y1-y2));
}

void setLabel(IplImage *src, int y, int x, int label)
{
    int yy = label/SEARCH_SIZE - SEARCH_SIZE/2;
    int xx = label%SEARCH_SIZE - SEARCH_SIZE/2;
    
    src->imageData[y*src->widthStep+x*src->nChannels] = (char)xx;
    src->imageData[y*src->widthStep+x*src->nChannels + 1] = (char)yy;
    
}

void block_match(IplImage *prev, IplImage *cur, IplImage *dst, struct ncost& cost)
{
    int height = prev->height;
    int width = prev->width;
    int bh = SEARCH_SIZE/2;
    int sh = SEARCH_SIZE/2;
    
    IplImage *img = cvCreateImage(cvGetSize(prev), prev->depth, 1);
    cvZero(img);
    for (int h=0;h<height;h++) {
        for (int w=0;w<width;w++) {
            double diff_min = INT_MAX;
            double diff_max = 0;
            double diff_sum = 0;
            int avg_count = 0;
            int min_x, min_y;
            
            for (int cy=h-sh;cy<=h+sh;cy++) {
                for (int cx=w-sh;cx<=w+sh;cx++) {
                    // 画像範囲外は無視
                    if (cy < 0 || cy >= height || cx < 0 || cx >= width) continue;
                    avg_count++;
                    double diff = 0;
                    int count = 0;
                    for (int y=cy-bh;y<=cy+bh;y++) {
                        for (int x=cx-bh;x<=cx+bh;x++) {
                            if (y < 0 || y >= height || x < 0 || x >= width) continue;
                            int tmp_diff = 0;
                            for (int c=0;c<3;c++) {
                                tmp_diff += abs(prev->imageData[h*prev->widthStep+w*prev->nChannels+c]
                                        - cur->imageData[y*cur->widthStep+x*cur->nChannels+c]);
                            }
                            diff += tmp_diff;
                            count++;
                        }
                    }
                    
                    diff = diff/count;
                    
                    if (diff < diff_min) {
                        diff_min = diff;
                        min_x = cx;
                        min_y = cy;
                    }
                    
                    if (diff > diff_max) {
                        diff_max = diff;
                    }
                    
                    diff_sum += diff;
                    
                    cost.setCost(h, w, getLabel(cx-w, cy-h), diff);
                }
            }

//            cout << "平均: " << diff_sum/avg_count << "最小 :" << diff_min << "最大: " << diff_max << endl;
            if (diff_sum/avg_count < 5) {
                for (int cy=-sh;cy<=sh;cy++) {
                    for (int cx=-sh;cx<=sh;cx++) {
                        cost.setCost(h, w, getLabel(cx, cy), diff_max * 5);
                    }
                }
                img->imageData[img->widthStep*h+w] = 255;
                cost.setCost(h, w, getLabel(0, 0), 0);
                min_x = w; min_y = h;
            }
            
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 0] = char(min_x - w);
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 1] = char(min_y - h);
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 2] = 0;
        }
        cout << "." << flush;
    }
    cvShowImage("foff", img);
    cout << "Finished block Match" << endl;
}

void HSVtoRGB(int h, int s, int v, int &r, int &g, int &b){
    float f;
    int i, p, q, t;

    i = (int)(h / 60.0f) % 6;
    f = (float)(h / 60.0f) - i;
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
    if (x == 0 && y == 0) return cvScalar(0,0,0,0);
    y = -y;
    double dist = sqrt(x*x+y*y);
    double theta = acos((double)x/dist);

    if (y < 0) {
        theta = 2*M_PI - theta;
    }
    int H = 180*theta/M_PI ;

    double s = dist*2/(SEARCH_SIZE);
    
    if (s > 1) s = 1;
    
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
            cvLine(dst, cvPoint(w, h), cvPoint(w+x*EMP, h+y*EMP), getColor(x, y));
            cvCircle(dst, cvPoint(w+x*EMP, h+y*EMP), 1, getColor(x, y));
        }
    }
}

void convertflow(IplImage *src, IplImage *dst)
{
    int height = src->height;
    int width = src->width;
    
    for (int h=0;h<height;h++) {
        for (int w=0;w<width;w++) {
            int x = (char)src->imageData[h*src->widthStep+w*src->nChannels + 0];
            int y = (char)src->imageData[h*src->widthStep+w*src->nChannels + 1];
            CvScalar color = getColor(x, y);
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 0] = color.val[0];
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 1] = color.val[1];
            dst->imageData[h*dst->widthStep+w*dst->nChannels + 2] = color.val[2];
        }
    }
}

int diff(IplImage *hsv, CvPoint pt1, CvPoint pt2)
{
    return pow(hsv->imageData[pt1.y*hsv->widthStep+pt1.x*3+2]
               - hsv->imageData[pt2.y*hsv->widthStep+pt2.x*3+2],2)+1;
}

void alpha_extension(IplImage *src, IplImage *label, struct ncost &cost, IplImage *dst)
{
    int height = src->height;
    int width = src->width;

    int E_t;
    int E = INT_MAX;
    int success;
    
    IplImage *hsv = cvCreateImage(cvGetSize(src), src->depth, 3);
    cvCvtColor(src, hsv, CV_BGR2HSV);
    IplImage *tmp_img = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
    

    int center = SEARCH_SIZE*SEARCH_SIZE/2;
    
    for (int loop=0;loop<E;loop++) {
        cout << "LOOP: " << loop << endl;
        success = 0;

        for (int alpha = 0;alpha<SEARCH_SIZE*SEARCH_SIZE;alpha++) {
            cout << "alpha: " << alpha << endl;
            typedef Graph<int, int, int> GraphType;
            
            GraphType *g = new GraphType(height*width, height*width*4);
            
            for (int i=0;i<height*width;i++) {
                g->add_node();
                // データ項の付与
                int y = i/width;
                int x = i%width;
                
                int cur_label = getLabel(label, y, x);
                g->add_tweights(i, cost.getCost(y, x, cur_label), cost.getCost(y, x, alpha));
            }
            for (int i=0;i<width*(height-1);i++) {
                g->add_node();
                // 平滑化項の付与(縦)
                int y = i/width;
                int x = i%width;
                
                double lambda1 = LAMBDA;
                
                g->add_tweights(i+width*height, lambda1*getDist(getLabel(label, y, x) , getLabel(label, y+1, x)), 0);
                g->add_edge(i, i+width*height, lambda1*getDist(getLabel(label, y, x), alpha), lambda1*getDist(getLabel(label, y, x), alpha));
                g->add_edge(i+width, i+width*height, lambda1*getDist(getLabel(label, y+1, x), alpha), lambda1*getDist(getLabel(label, y+1, x), alpha));
            }
            for (int i=0;i<width*height;i++) {
                g->add_node();
                if (i%(width-1) == 0) continue;
                // 平滑化項の付与(横)
                int y = i/width;
                int x = i%width;
                
                double lambda2 = LAMBDA;
                
                g->add_tweights(i+width*height+width*(height-1), lambda2*getDist(getLabel(label, y, x) ,getLabel(label, y, x+1)), 0);
                g->add_edge(i, i+width*height+width*(height-1), lambda2*getDist(getLabel(label, y, x), alpha), lambda2*getDist(getLabel(label, y, x), alpha));
                g->add_edge(i+1, i+width*height+width*(height-1), lambda2*getDist(getLabel(label, y, x+1), alpha), lambda2*getDist(getLabel(label, y, x+1), alpha));
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
                        setLabel(label, y, x, alpha);
                    }
                }
                convertflow(label, tmp_img);
                cvShowImage("LABEL", tmp_img);
                convert2flow(src, label, tmp_img);
                cvShowImage("FLOW", tmp_img);
                cvWaitKey(1);
                success = 1;
            }
            delete g;
            
        }
        
        if (success == 0) break;
    }
    cout << "Finished alpha extension" << endl;
}

int main(int argc, const char * argv[])
{
    IplImage *img1 = cvLoadImage(IMAGE1);
    IplImage *img2 = cvLoadImage(IMAGE2);
    IplImage *ret = cvCreateImage(cvGetSize(img1), img1->depth, 3);
    IplImage *result = cvCreateImage(cvGetSize(img1), img1->depth, 3);

    int a = 0;
    while (a < 3) {
        cvShowImage("img", img1);
        cvWaitKey(1000);
        cvShowImage("img", img2);
        cvWaitKey(1000);
        a++;
    }
    
    cvZero(ret);
    int hh = ret->height/SEARCH_SIZE;
    int ww = ret->width/SEARCH_SIZE;
    for (int h=0;h<=ret->height;h++) {
        for (int w=0;w<=ret->width;w++) {
            int x =((int)(w/1.01)/ww - SEARCH_SIZE/2);
            int y = ((int)(h/1.01)/hh - SEARCH_SIZE/2);
            ret->imageData[h*ret->widthStep+w*ret->nChannels] = (char)x;
            ret->imageData[h*ret->widthStep+w*ret->nChannels+1] = (char)y;
        }
    }
    
    convertflow(ret, result);
    cvShowImage("COLOR FOR FLOW", result);
    
    cvShowImage("image 1", img1);
    cvShowImage("image 2", img2);

    struct ncost cost = ncost(img1->height, img2->width, SEARCH_SIZE*SEARCH_SIZE);

    block_match(img1, img2, ret, cost);
    convertflow(ret, result);
    cvShowImage("Block Match Label", result);
    cvShowImage("LABEL", result);
    convert2flow(img1, ret, result);
    cvShowImage("Block Match Flow", result);
    cvShowImage("FLOW", result);

    if (!INITIAL_BLOCK_MATCH) {
        cvZero(ret);
    }

    cvWaitKey();
    
    alpha_extension(img1, ret, cost, result);
    convert2flow(img1, ret, result);
    cvShowImage("result", result);

    cvWaitKey();
    return 0;
}

