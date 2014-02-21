//
//  optical_flow.h
//  BlockMatching
//
//  Created by sawadanx on 2014/02/20.
//  Copyright (c) 2014年 kuno_lab. All rights reserved.
//

#ifndef __BlockMatching__optical_flow__
#define __BlockMatching__optical_flow__

#include <iostream>
#include <cv.h>
#include <highgui.h>

#define SEARCH_SIZE 5

#define BLOCK_SIZE 5

struct ncost
{
private:
    double *_datas;
    int _label_size;
    int _width, _height;
public:
    ncost(int width, int height, int label_size) {
        _label_size = label_size;
        _width = width;
        _height = height;
        _datas = (double *)malloc(sizeof(double) * width * height * label_size);
        init();
    }
    void init() {
        for (int i=0;i<_width*_height*_label_size;i++) {
            _datas[i] = 0;
        }
    }
    void setCost(int y, int x, int label, double data) {
        _datas[_label_size*(y*_width+x)+label] = data;
    }
    double getCost(int y, int x, int label) {
        return _datas[_label_size*(y*_width+x)+label];
    }
    void release() {
        free(_datas);
    }
};

void show_flow_label(const char *name, CvSize size, int search_size = SEARCH_SIZE);

void block_match(IplImage *prev, IplImage *cur, IplImage *dst, struct ncost& cost);

void convert2flow(IplImage *src, IplImage *label, IplImage *dst);

void convertflow(IplImage *src, IplImage *dst);

void alpha_extension(IplImage *src, IplImage *label, struct ncost &cost, IplImage *dst);

//void optical_flow(IplImage *prev, IplImage *curr, IplImage *dst_label, IplImage *dst_flow);

#endif /* defined(__BlockMatching__optical_flow__) */
