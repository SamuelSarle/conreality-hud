/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef LEAPFUNCS_H
#define LEAPFUNCS_H

class leap_event_listener : public Leap::Listener {
public:
  virtual void onConnect(const Leap::Controller&);
  virtual void onDisconnect(const Leap::Controller&);
  virtual void onFrame(const Leap::Controller&);
};

Leap::Vector convertCoords(cv::Mat image, Leap::Finger finger);
void drawLeapFingerTip(const Leap::Controller&, cv::Mat image);

#endif //LEAPFUNCS_H
