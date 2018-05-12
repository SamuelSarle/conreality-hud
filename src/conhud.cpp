/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "version.h"

#include "globals.h"
#include "rendering.h"
#include "input.h"

#ifndef DISABLE_DARKNET
#include "darknet.h"
#endif

#ifndef DISABLE_LEAPMOTION
#include "leapfuncs.h"
#endif

struct image_data {
#ifndef DISABLE_STEREO
  cv::Mat left_frame, right_frame;
#else
  cv::Mat frame;
#endif
};

Globals global;
#ifndef DISABLE_LEAPMOTION
Leap_data leap_data;
#endif

void edgeFilter(cv::Mat* image);
void applyFilters(cv::Mat* image);
void printHelp();

/********************
*                   *
*   MAIN FUNCTION   *
*                   *
********************/

int main(int argc, char* argv[]) {

  std::string filename = "0";
  std::string names_file = "darknet/data/coco.names";
  std::string cfg_file = "darknet/cfg/yolov3.cfg";
  std::string weights_file = "darknet/yolov3.weights";
//std::string names_file = "darknet/data/voc.names";
//std::string cfg_file = "darknet/cfg/tiny-yolo-voc.cfg";
//std::string weights_file = "darknet/tiny-yolo-voc.weights";
//std::string cfg_file = "darknet/cfg/yolo-voc.cfg";
//std::string weights_file = "darknet/yolo-voc.weights";
//std::string names_file = "darknet/data/custom.names";
//std::string cfg_file = "darknet/cfg/yolo-voc.2.0.cfg";
//std::string weights_file = "darknet/yolo-voc_custom.weights";

/*used for limiting clock speed*/
  constexpr int FPS = 30;
  constexpr int FRAME_DELAY = 1000 / FPS;
  auto next_frame = std::chrono::steady_clock::now();

/*set default screen size*/
  int window_h = 1080;
  int window_w = 1920;

  std::string out_videofile = "result.avi";

  int c;
  while ((c = getopt(argc, argv, "hrs:w")) != -1) {
    switch (c) {
      case 'h': //help
        printHelp();
        return EXIT_SUCCESS;
      case 'r': //record
        global.flags.save_output_videofile = true;
        break;
      case 's': //source
        filename = optarg;
        break;
      case 'w': //windowed
        global.flags.fullscreen = 0;
        window_h = 504; //16:9
        window_w = 896;
        break;
      case '?':
        if (optopt == 's') {
          std::printf("Option -%c requires an argument.\n", optopt);
        } else if (isprint(optopt)) {
          std::printf("Unknown option '-%c'\n", optopt);
        }
        return EXIT_FAILURE;
      default:
        break;
    }
  }

#ifndef DISABLE_LEAPMOTION
  Leap::Controller leap_controller;
  leap_controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
  leap_controller.setPolicy(Leap::Controller::POLICY_IMAGES);
  leap_controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
  leap_event_listener listener;
  leap_controller.addListener(listener);
#endif

#ifndef DISABLE_STEREO
  cv::VideoCapture capture_left, capture_right;
#else
  cv::VideoCapture capture;
#endif

  std::string file_ext = filename.substr(filename.find_last_of(".") + 1);
  if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mpjg" || file_ext == "mov") {
#ifndef DISABLE_STEREO
    capture_left.open(filename);
#else
     capture.open(filename);
#endif
  } else if (isdigit(filename[0])) {
#ifndef DISABLE_STEREO
    capture_left.open(stoi(filename));
    capture_right.open(stoi(filename)+1); //Our stereo camera simply has two separate channels, one after another
    if (!capture_left.isOpened()) { std::printf("Failed to open camera!\n"); return EXIT_FAILURE; }
    if (!capture_right.isOpened()) { std::printf("Failed to open camera!\n"); return EXIT_FAILURE; }

    capture_left.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture_left.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

    capture_right.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture_right.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
#else
    capture.open(stoi(filename));
    if (!capture.isOpened()) { std::printf("Failed to open camera!\n"); return EXIT_FAILURE; }
#endif
  }

  cv::Mat capt_frame;

#ifndef DISABLE_STEREO
  capture_left >> capt_frame;
#else
  capture >> capt_frame;
#endif
  if (capt_frame.empty()) { std::printf("Failed to capture a frame!\n"); return EXIT_FAILURE; }
  global.frame_size = capt_frame.size();

#ifndef DISABLE_DARKNET
  Detector detector(cfg_file, weights_file);
  auto object_names = objectNamesFromFile(names_file);
#endif

#ifndef DISABLE_OSVR
  osvr::clientkit::ClientContext ctx("com.osvr.conreality.hud");
  osvr::clientkit::DisplayConfig display(ctx);

  if (!display.valid()) {
    std::printf("Failed to get display config!\n");
    return EXIT_FAILURE;
  }

  while (!display.checkStartup()) {
    ctx.update();
  }
#endif

  GLuint texture;

  if (!glfwInit() ) {
    std::fprintf(stderr, "Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  GLFWwindow* window;
  window = glfwCreateWindow(window_w, window_h, "Window", NULL, NULL);
  if (window == NULL) {
    std::fprintf(stderr, "Failed to open GLFW window.\n");
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::fprintf(stderr, "Failed to initialize GLEW\n");
    return EXIT_FAILURE;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback(window, handleKey);
  glfwSetMouseButtonCallback(window, handleMouseButton);

  if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
    global.flags.joystick_connected = true;
    std::printf("Joystick detected\n");
  }

  glfwSwapInterval(1);

  glViewport(0, 0, window_w, window_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, window_w, window_h, 0.0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  std::string player_name = "player1";
  float text_scale = 0.8;
  int text_thickness = 2;
  int baseline = 0;
  cv::Size name_size = cv::getTextSize(player_name, cv::FONT_HERSHEY_COMPLEX_SMALL, text_scale, text_thickness, &baseline);

  image_data capt_image, pros_image;

  tbb::concurrent_bounded_queue<image_data> image_queue;
  image_queue.set_capacity(10);

  global.kb_control_queue.set_capacity(5);
  global.ms_control_queue.set_capacity(5);

  std::thread t_capture;
  t_capture = std::thread([&]() {

    while(global.flags.is_running) {
      if (image_queue.size() < 10) {
#ifndef DISABLE_STEREO
  cv::Mat left_image, right_image;
        capture_left >> left_image;
        capture_right >> right_image;
        capt_image.left_frame = left_image.clone();
        capt_image.right_frame = right_image.clone();
        image_queue.try_push(capt_image);
#else
        capture >> capt_frame;
        capt_image.frame = capt_frame.clone();
        image_queue.try_push(capt_image);
#endif
      }
    }
  });

  cv::VideoWriter output_video;
  if (global.flags.save_output_videofile) {
    output_video.open(out_videofile, CV_FOURCC('D','I','V','X'), std::max(35, 30), global.frame_size, true);
  }

  bool to_detect = true;
  std::vector<bbox_t> result_vec;

/**************
*  main loop  *
**************/

  while (global.flags.is_running) {

    next_frame += std::chrono::milliseconds(FRAME_DELAY);

    handleEvents();

    time(&global.rawtime);

///////////////////////////////////////
/* WORKFLOW FOR STEREO CAMERA */

#ifndef DISABLE_STEREO
    if (!image_queue.empty()) {

      image_queue.try_pop(pros_image);
      if (pros_image.left_frame.empty() || pros_image.right_frame.empty()) { std::printf("Video feed has ended\n"); global.flags.is_running = false; }

#ifndef DISABLE_OSVR
      ctx.update();
      if (global.flags.fullscreen) { render(display, pros_image.left_frame, pros_image.right_frame, texture, window_w, window_h); }
      else {
        drawToGLFW(pros_image.right_frame, texture, window_w, window_h);
      }
#else
      drawToGLFW(pros_image.right_frame, texture, window_w, window_h);
#endif
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

///////////////////////////////////////
/* WORKFLOW FOR MONO CAMERA */
#else

    if (!image_queue.empty()) {
      image_queue.try_pop(pros_image);

      if (pros_image.frame.empty()) { std::printf("Video feed has ended\n"); global.flags.is_running = false; }

      applyFilters(&pros_image.frame);

#ifndef DISABLE_DARKNET
/*detect objects and draw a box around them*/
      if (global.flags.enable_detection) {
        if (to_detect) {
          result_vec = detector.detect(pros_image.frame);
        }
        drawBoxes(pros_image.frame, result_vec, object_names);
      //  showConsoleResult(result_vec, object_names);    //uncomment this if you want console feedback
        to_detect = !to_detect;
      }
#endif

#ifndef DISABLE_LEAPMOTION
      if (global.flags.draw_fingers) { drawLeapFingerTip(leap_controller, pros_image.frame); }
#endif
      if (global.flags.display_name) { putText(pros_image.frame, player_name, cv::Point((global.frame_size.width/2)-name_size.width/2, 50), cv::FONT_HERSHEY_COMPLEX_SMALL, text_scale, cv::Scalar(0,255,0), text_thickness); }

/*update and render video feed*/
#ifndef DISABLE_OSVR
      ctx.update();
      if (global.flags.fullscreen) { render(display, pros_image.frame, texture, window_w, window_h); }
      else {
        drawToGLFW(pros_image.frame, texture, window_w, window_h);
      }
#else

    drawToGLFW(pros_image.frame, texture, window_w, window_h);

    drawCircle(window_w/2, window_h/2, 50);
#endif

#ifndef DISABLE_LEAPMOTION
      if (global.flags.draw_fingers) { drawLeapFingerTip(leap_controller, pros_image.frame); }
#endif
      glfwSwapBuffers(window);
      glfwPollEvents();

      if (output_video.isOpened() && global.flags.save_output_videofile) {
        output_video << pros_image.frame;
      }
    }
#endif
///////////////////////////////////////

/*slow down to 30FPS if running faster*/
    std::this_thread::sleep_until(next_frame);

  } //main loop

  if (t_capture.joinable()) { t_capture.join(); }

  glfwTerminate();

  cv::destroyAllWindows();
#ifndef DISABLE_LEAPMOTION
  leap_controller.removeListener(listener);
#endif

  std::printf("Program exited\n");

  return EXIT_SUCCESS;
}

/********************
*                   *
*     FUNCTIONS     *
*                   *
********************/

void edgeFilter(cv::Mat* image) {

  cv::Mat src_gray, canny_output;
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cvtColor(*image, src_gray, CV_BGR2GRAY);
  cv::blur(src_gray, src_gray, cv::Size(3,3));

  cv::Canny(src_gray, canny_output, 30, 120, 3);

  cv::findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

  if (global.flags.edge_filter_ext) { *image = cv::Scalar(0,0,0); }

  for (unsigned int i = 0; i < contours.size(); i++) {
    cv::Scalar color = cv::Scalar(0,255,0);
    cv::drawContours(*image, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
  }
}

void applyFilters(cv::Mat* image) {
  if (global.flags.flip_image) { cv::flip(*image, *image, 0); }

  if (global.flags.edge_filter) { edgeFilter(image); }
  if (global.flags.display_time) {
    struct tm* timeinfo;
    char timeText[10];
    timeinfo = localtime(&global.rawtime);
    strftime(timeText, 80, "%H:%M:%S", timeinfo);

    putText(*image, timeText, cv::Point(global.frame_size.width-230,global.frame_size.height-150), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,255,0), 2);
  }
}

#ifndef DISABLE_STEREO

void updateBuffer(image_data data) {

}

#else

#endif

//TODO
void printHelp() {
  std::printf("help\n");
}
