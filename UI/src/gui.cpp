// thanks https://git.gnome.org//browse/gstreamermm/tree/examples/media_player_gtkmm/player_window.cc

#include "camera_mode.h"
#include "gui.h"
#include <iostream>
#include <gdk/gdkx.h>
#include <opencv2/opencv.hpp>
#include <python2.7/Python.h>
#include "boost/filesystem.hpp"

Gui::Gui()
: l1_box(                   Gtk::ORIENTATION_VERTICAL,  4),
  l2_box_top(               Gtk::ORIENTATION_HORIZONTAL,4),
  l4_options_CONTINUOUS(    Gtk::ORIENTATION_VERTICAL,  4),
  l4_options_SINGLE_CAPTURE(Gtk::ORIENTATION_VERTICAL,  4),
  l4_options_HDR(           Gtk::ORIENTATION_VERTICAL,  4),
  save_SC("Save"),
  save_HDR("Save"),
  next_G("Next Image"),
  adjustment_exposure(Gtk::Adjustment::create(1.0, 1.0, 100.0, 1.0, 10.0, 0.0)),
  adjustment_iso(Gtk::Adjustment::create(1.0, 1.0, 100.0, 1.0, 10.0, 0.0)),
  exposure(adjustment_exposure),
  iso(adjustment_iso),
  exposure_label("Exposure [1-100]", Gtk::ALIGN_START),
  iso_label("ISO [1-100]", Gtk::ALIGN_START)
{
  // set title of new window.
  set_title("18-500 Team D7 GUI");
  // set border width of the window.
  set_border_width(10);
  set_default_size(800,600);

  // add highest level container (vertical box)
  add(l1_box);

  // add second level containers (two horizontal boxes)
  l1_box.pack_start(l2_box_top, true, true);
  l1_box.pack_start(l2_toolbar, false, false);
  Gui::populate_toolbar();

  // add left panel to top l2 box
  l2_box_top.pack_start(l3_stack, false, false);

  // add viewfinder to top l2 box
  l2_box_top.pack_start(l3_viewfinder, true, true);

  // add control options for each mode
  l3_stack.add(l4_options_CONTINUOUS, "continuous options");

  l3_stack.add(l4_options_SINGLE_CAPTURE, "single capture options");
  l4_options_SINGLE_CAPTURE.pack_start(save_SC);
  l4_options_SINGLE_CAPTURE.pack_start(exposure_label);
  l4_options_SINGLE_CAPTURE.pack_start(exposure);
  l4_options_SINGLE_CAPTURE.pack_start(iso_label);
  l4_options_SINGLE_CAPTURE.pack_start(iso);
  save_SC.signal_clicked().connect(sigc::mem_fun(*this, &Gui::on_save));
  exposure.signal_changed().connect(sigc::mem_fun(*this, &Gui::on_exposure_change));
  iso.signal_changed().connect(sigc::mem_fun(*this, &Gui::on_iso_change));

  l3_stack.add(l4_options_HDR, "HDR options");
  l4_options_HDR.pack_start(save_HDR);
  save_HDR.signal_clicked().connect(sigc::mem_fun(*this, &Gui::on_save));

  l3_stack.add(l4_options_GALLERY, "Gallery options");
  l4_options_GALLERY.pack_start(next_G);
  next_G.signal_clicked().connect(sigc::mem_fun(*this, &Gui::on_next_gallery));

  l3_stack.set_visible_child(l4_options_CONTINUOUS);

  // Initializing Python environment
  print("Initializing Python environment");
  Py_Initialize();
  PyRun_SimpleString("import os");
  PyRun_SimpleString("import sys");
  // append script folder to PATH so imports work properly
  PyRun_SimpleString("sys.path.append('/home/pi/workspace/18500-D7/hdr/')");

  show_all_children();
  print("Setup finished.");
}

Gui::~Gui() {
  // destroy Python environment
  Py_Finalize();
}

void Gui::on_mode_change(CameraMode mode) {
  print("Changing Camera mode");

  Gui::set_current_mode(mode);
  // viewfinder must be refreshed
  l3_viewfinder.queue_draw();

  switch (mode) {
    case CameraMode::CONTINUOUS:
      l3_stack.set_visible_child(l4_options_CONTINUOUS);
      break;
    case CameraMode::SINGLE_CAPTURE:
      l3_stack.set_visible_child(l4_options_SINGLE_CAPTURE);
      l3_viewfinder.get_frame(true);
      l3_viewfinder.queue_draw();
      break;
    case CameraMode::HDR:
      l3_stack.set_visible_child(l4_options_HDR);
      hdr();
      break;
    case CameraMode::GALLERY:
      l3_stack.set_visible_child(l4_options_GALLERY);
      gallery();
      break;
    default:
      error("unknown CameraMode", "GUI entered unknown camera mode in on_mode_change()");
  }
}

void Gui::on_exposure_change() {
  print("Changing Exposure...");
  l3_viewfinder.set_property(
      CV_CAP_PROP_AUTO_EXPOSURE,0);
  l3_viewfinder.set_property(
      CV_CAP_PROP_EXPOSURE, exposure.get_value_as_int());
}

void Gui::on_iso_change() {
  print("Changing ISO...");
  l3_viewfinder.set_property(
      CV_CAP_PROP_GAIN, iso.get_value_as_int());
}

void Gui::on_save() {
  std::stringstream ss;
  ss << IMG_SAVE_PATH;
  ss << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  ss << ".jpg";
  cv::Mat frame = l3_viewfinder.get_frame();
  cv::imwrite(ss.str().c_str(), frame);
  Gui::on_mode_change(CameraMode::CONTINUOUS);
}

void Gui::on_off() {
  Gtk::Main::quit();
}

void Gui::on_next_gallery() {
  const char* top_file = *saved_files.begin();
  saved_files.push_back(top_file);

  cv::Mat image = cv::imread(top_file);
  l3_viewfinder.set_frame(image);
}

void Gui::populate_toolbar() {
  // create the photo capture button
  auto pc_image = new Gtk::Image(IMG_RESOURCE_PATH + "shoot.ico");
  auto pc_button = new Gtk::ToolButton(*pc_image, "shoot");
  pc_button->set_tooltip_text("Take photo");
  // link photo capture button to function
  pc_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::SINGLE_CAPTURE));
  l2_toolbar.append(*pc_button);

  // create the photo capture button
  auto live_image = new Gtk::Image(IMG_RESOURCE_PATH + "live.ico");
  auto live_button = new Gtk::ToolButton(*live_image, "live");
  live_button->set_tooltip_text("Continuous Shooting");
  // link photo capture button to change camera mode 
  live_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::CONTINUOUS));
  l2_toolbar.append(*live_button);

  // create the HDR mode button
  auto hdr_image = new Gtk::Image(IMG_RESOURCE_PATH + "hdr.ico");
  auto hdr_button = new Gtk::ToolButton(*hdr_image, "hdr");
  hdr_button->set_tooltip_text("Toggle HDR mode");
  hdr_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::HDR));
  l2_toolbar.append(*hdr_button);

  // create the Panorama mode button
  auto panorama_image = new Gtk::Image(IMG_RESOURCE_PATH + "panorama.ico");
  auto panorama_button = new Gtk::ToolButton(*panorama_image, "panorama");
  panorama_button->set_tooltip_text("Toggle Panorama mode");
  panorama_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::PANORAMA));
  l2_toolbar.append(*panorama_button);

  // create the Stabilize mode button
  auto stabilize_image = new Gtk::Image(IMG_RESOURCE_PATH + "stabilize.ico");
  auto stabilize_button = new Gtk::ToolButton(*stabilize_image, "stabilize");
  stabilize_button->set_tooltip_text("Toggle Image Stabilization mode");
  stabilize_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::STABILIZE));
  l2_toolbar.append(*stabilize_button);

  // create the gallery button
  auto gallery_image = new Gtk::Image(IMG_RESOURCE_PATH + "gallery.ico");
  auto gallery_button = new Gtk::ToolButton(*gallery_image, "gallery");
  gallery_button->set_tooltip_text("Toggle Gallery mode");
  gallery_button->signal_clicked().connect(sigc::bind<CameraMode>(
      sigc::mem_fun(*this, &Gui::on_mode_change),
      CameraMode::GALLERY));
  l2_toolbar.append(*gallery_button);

  // create the OFF button
  auto off_image = new Gtk::Image(IMG_RESOURCE_PATH + "off.ico");
  auto off_button = new Gtk::ToolButton(*off_image, "off");
  off_button->set_tooltip_text("Turn camera OFF");
  off_button->signal_clicked().connect(sigc::mem_fun(*this, &Gui::on_off));
  l2_toolbar.append(*off_button);
}

void Gui::set_current_mode(CameraMode mode) {
  current_mode = mode;
  l3_viewfinder.set_camera_mode(mode);
}

void Gui::hdr() {
  // let go of the camera to allow python script to take control
  l3_viewfinder.uninitialize_camera();

  // run the script
  FILE* file = fopen("/home/pi/workspace/18500-D7/hdr/runhdrpi.py", "r");
  // change working directory so calls and file operations work properly
  PyRun_SimpleString("os.chdir('/home/pi/workspace/18500-D7/hdr/')");
  PyRun_SimpleFile(file, "/home/pi/workspace/18500-D7/hdr/runhdrpi.py");
  fclose(file);

  // get the result back and dsiplay
  cv::Mat image = cv::imread("/home/pi/workspace/18500-D7/hdr/output.jpg");
  l3_viewfinder.set_frame(image);

  // re-take camera
  l3_viewfinder.initialize_camera();
}

void Gui::gallery() {
  boost::filesystem::recursive_directory_iterator it(IMG_SAVE_PATH);
  boost::filesystem::recursive_directory_iterator endit;

  const std::string ext("png");

  saved_files.clear();

  std::string s;
	while(it != endit) {
    if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ext) {
      s = it->path().string();
      const char* filename = s.c_str();
      saved_files.push_back(filename);
    }
    ++it;
  }
}

