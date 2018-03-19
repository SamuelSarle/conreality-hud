/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "leapfuncs.h"

extern Globals global;
extern Leap_hands_data leap_hands_data;

void leap_event_listener::onConnect(const Leap::Controller& controller) {
  std::cout << "Leap Motion connected" << std::endl;
}

void leap_event_listener::onDisconnect(const Leap::Controller& controller) {
  std::cout << "Leap Motion disconnected" << std::endl;
}

void leap_event_listener::onFrame(const Leap::Controller& controller) {
  Leap::Frame frame = controller.frame();
  leap_hands_data.hands = frame.hands();

/*
  if (leap_hands_data.hands.count() == 2) {
    if ((leap_hands_data.hands[0].palmNormal().roll() > 0.8f) && (leap_hands_data.hands[1].palmNormal().roll() > 0.8f) &&
        (leap_hands_data.hands[0].palmNormal().yaw() < 0.4f) && (leap_hands_data.hands[1].palmNormal().yaw() < 0.4f) &&
        (leap_hands_data.hands[0].palmVelocity().z > 100.0f) && (leap_hands_data.hands[1].palmVelocity().z > 100.0f) &&
        (leap_hands_data.hands[0].palmVelocity().z < 300.0f) && (leap_hands_data.hands[1].palmVelocity().z < 300.0f)) {

      //std::cout << "lift up" << std::endl;
    }
  }
*/
/*
  for (Leap::GestureList::const_iterator i = gestures.begin(); i != gestures.end(); i ++) {
    std::cout << (*i).type() << std::endl;
    if ((*i).type() == Leap::Gesture::TYPE_SWIPE) { std::cout << "swipe" << std::endl; }
  }
*/
/*
  for (int i = 0; i < leap_hands_data.hands.count(); i++) {
    Leap::FingerList finger_list = leap_hands_data.hands[i].fingers();
    Leap::Finger thumb = finger_list[0];
    if (leap_hands_data.hands[i].isRight()) {
      std::cout << "right hand ";
      if (leap_hands_data.hands[i].grabStrength() < 0.1f && leap_hands_data.hands[i].pinchStrength() < 0.1f) {
        std::cout << "palm ";
        if (leap_hands_data.hands[i].palmNormal().roll() > 0.6f) {
          std::cout << "up ";
          if (leap_hands_data.hands[i].palmVelocity().z < -200) { std::cout << "upwards."; }
        }
        if (leap_hands_data.hands[i].palmNormal().roll() < -0.6f) {
          std::cout << "down ";
          if (leap_hands_data.hands[i].palmVelocity().z > 200) { std::cout << "downwards."; }
        }
      }
      else if (leap_hands_data.hands[i].grabStrength() == 1.0f && thumb.isExtended()) {
        std::cout << "thumbs ";
        if (leap_hands_data.hands[i].palmNormal().roll() < -0.6f) {
          std::cout << "down.";
        }
        if (leap_hands_data.hands[i].palmNormal().roll() > 0.6f) {
          std::cout << "up.";
        }
      }
      std::cout << std::endl;
    }
  }
*/
}

Leap::Vector convertCoords(const Leap::Controller& controller, cv::Mat image, Leap::Finger finger) {
  Leap::Vector app_coords;
  int app_width = image.cols;
  int app_height = image.rows;
  float x_offset, y_offset, z_offset;
  x_offset = 0.13;
  y_offset = -0.05;
  z_offset = 40;
  Leap::InteractionBox i_box = controller.frame().interactionBox();
  Leap::Vector normalizedPoint = i_box.normalizePoint(finger.tipPosition(), false);
  app_coords.x = (1 - normalizedPoint.x + x_offset) * app_width;
  app_coords.y = (normalizedPoint.z + y_offset) * app_height; //in HMDs Z is up/down
  app_coords.z = (1 - normalizedPoint.y) * z_offset; //in HMDs Y is near/far

  return app_coords;
}

void drawLeapFingerTip(const Leap::Controller& controller, cv::Mat image) {
  for (int i = 0; i < leap_hands_data.hands.count(); i++) {
    for (int j = 0; j < leap_hands_data.hands[i].fingers().count(); j++) {
      Leap::Vector app_coords = convertCoords(controller, image, leap_hands_data.hands[i].fingers()[j]);
      float confidence = leap_hands_data.hands[i].confidence();
      cv::Scalar color;
      if (confidence > 0.5) { color = cv::Scalar(0,255,0); }
      else if (confidence < 0.5 && confidence > 0.25) { color = cv::Scalar(0,255,255); }
      else if (confidence < 0.25) { color = cv::Scalar(0,0,255); }
      if (app_coords.z > 0) {
        cv::circle(image, cv::Point(app_coords.x, app_coords.y), app_coords.z, color, -1);
      }
    }
  }
}
