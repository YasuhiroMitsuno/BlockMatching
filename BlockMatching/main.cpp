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

#define IMAGE1 "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/test3.bmp"
#define IMAGE2 "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/test4.bmp"

#define INPUT_FILE "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/mov/mov2.avi"

#define RESULT "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/result.bmp"
#define FLOW "/Users/yasu/Labo/Code/BlockMatching/BlockMatching/images/flow.bmp"

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
    IplImage *tmp = cvCreateImage(cvGetSize(prev), prev->depth, prev->nChannels);
    
    show_flow_label("flow Label", cvGetSize(prev));
    
    for (int i=0;i<50;i++) {
        frame = cvQueryFrame(capture);
        cvCopy(frame, prev);
    }
    
    while (1) {
        frame = cvQueryFrame(capture);
        if (!frame) break;
        ncost cost = ncost(prev->width, prev->height, SEARCH_SIZE*SEARCH_SIZE);
        cvCopy(frame, curr);
        block_match(prev, curr, label, cost);

        convertflow(label, tmp);
        cvShowImage("block match", tmp);
        convert2flow(curr, label, tmp);
        cvShowImage("flow", tmp);
        
        alpha_extension(curr, label, cost, label);
        convertflow(label, tmp);
        cvShowImage("RESULT", tmp);
        convert2flow(curr, label, tmp);
        cvShowImage("RESULT FLOW", tmp);
        
        /*
        optical_flow(prev, curr, label, flow);
        cvShowImage("ret", label);
        cvShowImage("result", flow);
        optical_flow(curr, prev, label, flow);
        cvShowImage("r_ret", label);
        cvShowImage("r_result", flow);
 */
        cvWaitKey(1);
        cvCopy(curr, prev);
    }
    

    
    cvWaitKey();
}

