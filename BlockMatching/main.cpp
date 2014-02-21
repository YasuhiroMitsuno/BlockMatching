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

#include "optical_flow.h"

using namespace std;

#define IMAGE1 "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/test3.bmp"
#define IMAGE2 "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/test4.bmp"

#define INPUT_FILE "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/mov/mov2.avi"

#define RESULT "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/result.bmp"
#define FLOW "/Users/kuno_lab/yasuhiro/Code/BlockMatching/BlockMatching/images/flow.bmp"

#define SEARCH_SIZE 5
#define BLOCK_SIZE 5
#define COLOR_VARIATION 12

#define INITIAL_BLOCK_MATCH 1

#define LAMBDA 30
#define BIAS 1



int main(int argc, const char * argv[])
{
    cout << "INPUT_FILE = " << INPUT_FILE << endl;
    
    CvCapture *capture = 0;
    capture = cvCreateFileCapture(INPUT_FILE);
    
    IplImage *frame = cvQueryFrame(capture);
    IplImage *prev = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
    IplImage *curr = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
    cvCopy(frame, prev);
    
    IplImage *label = cvCreateImage(cvGetSize(prev), prev->depth, prev->nChannels);
    IplImage *flow = cvCreateImage(cvGetSize(prev), prev->depth, prev->nChannels);
    
    show_flow_label("flow Label", cvGetSize(prev));
    
    for (int i=0;i<25;i++) {
        frame = cvQueryFrame(capture);
        cvCopy(frame, prev);
    }
    
    while (1) {
        frame = cvQueryFrame(capture);
        if (!frame) break;
        cvCopy(frame, curr);
        optical_flow(prev, curr, label, flow);
        cvShowImage("ret", label);
        cvShowImage("result", flow);
        optical_flow(curr, prev, label, flow);
        cvShowImage("r_ret", label);
        cvShowImage("r_result", flow);
        cvWaitKey(1);
        cvCopy(curr, prev);
        //cvCopy(curr, prev);
    }
    

    
    cvWaitKey();
}

