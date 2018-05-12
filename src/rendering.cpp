/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "rendering.h"

extern Globals global;
#ifndef DISABLE_LEAPMOTION
extern Leap_data leap_data;
#endif

#ifndef DISABLE_OSVR
/*Mono camera rendering*/
void render(osvr::clientkit::DisplayConfig &display, cv::Mat frame, GLuint texture, int window_w, int window_h) {
  glClearColor(0,0,0,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  display.forEachEye([&](osvr::clientkit::Eye eye) {
    eye.forEachSurface([&](osvr::clientkit::Surface surface) {
      uint8_t eye_number = eye.getEyeID();
      auto viewport = surface.getRelativeViewport();
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, viewport.width, viewport.height, 0.0, 0.0, 100.0);
      glMatrixMode(GL_MODELVIEW);
      glViewport(static_cast<GLint>(viewport.left)+120, //temporary fix
                 static_cast<GLint>(viewport.bottom),
                 static_cast<GLsizei>(viewport.width),
                 static_cast<GLsizei>(viewport.height));
      drawToGLFW(frame, texture, viewport.width, viewport.height);


      if (global.flags.show_items) {
        if (eye_number == 0) { drawSquare(viewport.width/2.0f, viewport.height/2.0f, 150, 150); }
        if (eye_number == 1) { drawCircle(viewport.width/2.0f, viewport.height/2.0f, 100.0f); }
      }
    });
  });
}

/*Stereo camera rendering*/
void render(osvr::clientkit::DisplayConfig &display, cv::Mat left_frame, cv::Mat right_frame, GLuint texture, int window_w, int window_h) {
  glClearColor(0,0,0,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  display.forEachEye([&](osvr::clientkit::Eye eye) {
    eye.forEachSurface([&](osvr::clientkit::Surface surface) {
      uint8_t eye_number = eye.getEyeID();
      auto viewport = surface.getRelativeViewport();
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, viewport.width, viewport.height, 0.0, 0.0, 100.0);
      glMatrixMode(GL_MODELVIEW);
      glViewport(static_cast<GLint>(viewport.left)+120, //temporary fix
                 static_cast<GLint>(viewport.bottom),
                 static_cast<GLsizei>(viewport.width),
                 static_cast<GLsizei>(viewport.height));

        if (eye_number == 0) {
          drawToGLFW(left_frame, texture, viewport.width, viewport.height);
          if (global.flags.show_items) { drawSquare(viewport.width/2.0f, viewport.height/2.0f, 150, 150); }
        }
        else if (eye_number == 1) {
          drawToGLFW(right_frame, texture, viewport.width, viewport.height);
          if (global.flags.show_items) { drawCircle(viewport.width/2.0f, viewport.height/2.0f, 100.0f); }
        }
    });
  });
}
#endif

void drawToGLFW(cv::Mat img, GLuint texture, int window_w, int window_h) {

  glLoadIdentity();
  glGenTextures(1, &texture);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.size().width, img.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

  glEnable(GL_TEXTURE_2D);

  glColor4f(1.0f, 1.0f, 1.0f, 0.0f);

  glBegin(GL_QUADS);
    glTexCoord2i(0, 1); glVertex2i(0, window_h);
    glTexCoord2i(0, 0); glVertex2i(0, 0);
    glTexCoord2i(1, 0); glVertex2i(window_w, 0);
    glTexCoord2i(1, 1); glVertex2i(window_w, window_h);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &texture);
}

void drawSquare(float x, float y, int w, int h) {
  x -= h/2; //to center the square
  y -= w/2;
  glBegin(GL_POLYGON);
  glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
  glVertex3f(x,y,0);
  glVertex3f(x+w,y,0);
  glVertex3f(x+w,y+h,0);
  glVertex3f(x,y+h,0);
  glEnd();
}

void drawCircle(float x, float y, float radius) {

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(x, y, 0.0f);
  static const int circle_points = 100;
  static const float angle = 2.0f * 3.1416f / circle_points;

  glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
  glBegin(GL_POLYGON);
  double angle1 = 0.0;
  glVertex2d(radius * cos(0.0) , radius * sin(0.0));
  for (int i=0; i < circle_points; i++) {
    glVertex2d(radius * cos(angle1), radius *sin(angle1));
    angle1 += angle;
  }
  glEnd();
  glPopMatrix();
}
