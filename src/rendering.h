/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef RENDERING_H
#define RENDERING_H

#ifndef DISABLE_OSVR
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame, GLuint texture, int window_w, int window_h);
void render(osvr::clientkit::DisplayConfig &display, cv::Mat left_frame, cv::Mat right_frame, GLuint texture, int window_w, int window_h);
#endif
void drawToGLFW(cv::Mat img, GLuint texture, int window_w, int window_h);
#ifndef DISABLE_LEAPMOTION
//void drawLeapFingerTip(const Leap::Controller& controller, cv::Mat image);
#endif
void drawSquare(float x, float y, int w, int h);
void drawCircle(float cx, float cy, float r);

#endif //RENDERING_H
