/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DARKNET_H
#define DARKNET_H

#include "yolo_v2_class.hpp"

void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names, int current_det_fps = -1, int current_cap_fps = -1);
cv::Scalar objIdToColor(int obj_id);
void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names);
std::vector<std::string> objectNamesFromFile(std::string const filename);

#endif //DARKNET_H
