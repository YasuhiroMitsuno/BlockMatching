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

void show_flow_label(const char *name, CvSize size, int search_size = SEARCH_SIZE);
void optical_flow(IplImage *prev, IplImage *curr, IplImage *dst_label, IplImage *dst_flow);

#endif /* defined(__BlockMatching__optical_flow__) */
