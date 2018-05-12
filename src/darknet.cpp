/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "darknet.h"

void drawBoxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names,
  int current_det_fps, int current_cap_fps)
{
  int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };

  for (auto &i : result_vec) {
    cv::Scalar color = objIdToColor(i.obj_id);
    cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 2);
    if (obj_names.size() > i.obj_id) {
      std::string obj_name = obj_names[i.obj_id];
      if (obj_name == "head") { cv::circle(mat_img, cv::Point(i.x+i.w/2, i.y+i.h/2), i.h/2, color, 2); }
      else {
        if (i.track_id > 0) { obj_name += " - " + std::to_string(i.track_id); }
        cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
        int const max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
        cv::rectangle(mat_img, cv::Point2f(std::max((int)i.x - 1, 0), std::max((int)i.y - 30, 0)),
          cv::Point2f(std::min((int)i.x + max_width, mat_img.cols-1), std::min((int)i.y, mat_img.rows-1)),
          color, CV_FILLED, 8, 0);
        putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(0, 0, 0), 2);
      }
    }
  }
  if (current_det_fps >= 0 && current_cap_fps >= 0) {
    std::string fps_str = "FPS detection: " + std::to_string(current_det_fps) + "   FPS capture: " + std::to_string(current_cap_fps);
    putText(mat_img, fps_str, cv::Point2f(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(50, 255, 0), 2);
  }
}

cv::Scalar objIdToColor(int obj_id) {
	int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };
	int const offset = obj_id * 123457 % 6;
	int const color_scale = 150 + (obj_id * 123457) % 100;
	cv::Scalar color(colors[offset][0], colors[offset][1], colors[offset][2]);
	color *= color_scale;
	return color;
}

void showConsoleResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names) {
  for (auto &i : result_vec) {
    if (obj_names.size() > i.obj_id) std::cout << obj_names[i.obj_id] << " - ";
    std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y 
      << ", w = " << i.w << ", h = " << i.h
      << std::setprecision(3) << ", prob = " << i.prob << std::endl;
  }
}

std::vector<std::string> objectNamesFromFile(std::string const filename) {
  std::ifstream file(filename);
  std::vector<std::string> file_lines;
  if (!file.is_open()) { return file_lines; }
  for (std::string line; getline(file, line);) { file_lines.push_back(line); }
  std::printf("Object names loaded\n");
  return file_lines;
}
