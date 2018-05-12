/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "leapfuncs.h"

extern Globals global;
extern Leap_data leap_data;
extern void drawCircle(float x, float y, float radius);

void leap_event_listener::onConnect(const Leap::Controller& controller) {
  std::cout << "Leap Motion connected" << std::endl;
}

void leap_event_listener::onDisconnect(const Leap::Controller& controller) {
  std::cout << "Leap Motion disconnected" << std::endl;
}

void leap_event_listener::onImages(const Leap::Controller & controller) {
  leap_data.images = controller.images();
}

void leap_event_listener::onFrame(const Leap::Controller& controller) {
  Leap::Frame frame = controller.frame();
  leap_data.hands = frame.hands();
/*
  if (leap_data.hands.count() == 2) {
    if ((leap_data.hands[0].palmNormal().roll() > 0.8f) && (leap_data.hands[1].palmNormal().roll() > 0.8f) &&
        (leap_data.hands[0].palmNormal().yaw() < 0.4f) && (leap_data.hands[1].palmNormal().yaw() < 0.4f) &&
        (leap_data.hands[0].palmVelocity().z > 100.0f) && (leap_data.hands[1].palmVelocity().z > 100.0f) &&
        (leap_data.hands[0].palmVelocity().z < 300.0f) && (leap_data.hands[1].palmVelocity().z < 300.0f)) {

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
  for (int i = 0; i < leap_data.hands.count(); i++) {
    Leap::FingerList finger_list = leap_data.hands[i].fingers();
    Leap::Finger thumb = finger_list[0];
    if (leap_data.hands[i].isRight()) {
      std::cout << "right hand ";
      if (leap_data.hands[i].grabStrength() < 0.1f && leap_data.hands[i].pinchStrength() < 0.1f) {
        std::cout << "palm ";
        if (leap_data.hands[i].palmNormal().roll() > 0.6f) {
          std::cout << "up ";
          if (leap_data.hands[i].palmVelocity().z < -200) { std::cout << "upwards."; }
        }
        if (leap_data.hands[i].palmNormal().roll() < -0.6f) {
          std::cout << "down ";
          if (leap_data.hands[i].palmVelocity().z > 200) { std::cout << "downwards."; }
        }
      }
      else if (leap_data.hands[i].grabStrength() == 1.0f && thumb.isExtended()) {
        std::cout << "thumbs ";
        if (leap_data.hands[i].palmNormal().roll() < -0.6f) {
          std::cout << "down.";
        }
        if (leap_data.hands[i].palmNormal().roll() > 0.6f) {
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
  int app_width = 896;
  int app_height = 504;
//  int app_width = image.cols;
//  int app_height = image.rows;
  float x_offset, y_offset, z_offset;
  x_offset = 0.13;
  y_offset = -0.05;
  z_offset = 40;
  Leap::InteractionBox i_box = controller.frame().interactionBox();
  Leap::Vector normalizedPoint = i_box.normalizePoint(finger.tipPosition(), false);
  app_coords.x = (1 - normalizedPoint.x/* + x_offset*/) * app_width;
  app_coords.y = (normalizedPoint.z/* + y_offset*/) * app_height; //in HMDs Z is up/down
  app_coords.z = (1 - normalizedPoint.y) * z_offset; //in HMDs Y is near/far

  return app_coords;
}

void drawLeapFingerTip(const Leap::Controller& controller, cv::Mat image) {
  const float camera_offset = 20;
  int window_h = 504;
  int window_w = 896;
//  for (int k = 0; k < 2; k++) {
//std::cout << "ping ";
    Leap::Image leap_image = leap_data.images[0];
//std::cout << "pong" << std::endl;
    for (int i = 0; i < leap_data.hands.count(); i++) {
      for (int j = 0; j < leap_data.hands[i].fingers().count(); j++) {
 //       Leap::Vector app_coords = convertCoords(controller, image, leap_data.hands[i].fingers()[j]);

        Leap::Vector tip = leap_data.hands[i].fingers()[j].tipPosition();
//std::cout << "tip: " << tip << std::endl;
        float h_slope = -(tip.x/* + camera_offset * (2 * 0 - 1)*/)/tip.y;
        float v_slope = tip.z/tip.y;

//std::cout << "h_slope: " << h_slope << " v_slope: " << v_slope << std::endl;

        //Leap::Vector ray = Leap::Vector(h_slope * leap_image.rayScaleX() + leap_image.rayOffsetX(), v_slope * leap_image.rayScaleY() + leap_image.rayOffsetY(), 0);

//std::cout << "rayscales: " << leap_image.rayScaleX() << leap_image.rayScaleY() << std::endl;
//std::cout << "ray: " << ray << std::endl;

        Leap::Vector pixel = leap_image.warp(Leap::Vector(h_slope, v_slope, 0));
 //       Leap::Vector pixel = Leap::Vector(ray.x * window_w/* * 12 + 300*/, ray.y * window_h/* * 12 + 200*/, 0);

//std::cout << "pixel: " << pixel << std::endl;

        float confidence = leap_data.hands[i].confidence();
        cv::Scalar color = cv::Scalar(0, confidence * 255, (1 - confidence) * 255);
        //if (app_coords.z > 0) {
 //         cv::circle(image, cv::Point(app_coords.x, app_coords.y), app_coords.z, color, -1);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
      glOrtho(0.0, window_w, window_h, 0.0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
          drawCircle(pixel.x, pixel.y, 30);
      //    cv::circle(image, cv::Point(pixel.x, pixel.y), 20, color, -1);
        //}
      }
    }
//  }
}
