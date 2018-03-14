/**
 * @file    viewfinder.cpp
 *
 * @brief   implementation for Viewfinder class
 *
 * @date    2018/02/25
 * @author  LeRoy Gary <lgary@andrew.cmu.edu>
 */

#include "camera_state.h"
#include "gui.h"
#include "viewfinder.h"
#include <cairomm/context.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <giomm/resource.h>
#include <glibmm/fileutils.h>
#include <iostream>

Viewfinder::Viewfinder() : cv_opened(false) {
    cv_cap.open(0);
   
    if (cv_cap.isOpened() == true) {
        cv_opened = true;
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Viewfinder::on_timeout), 50);
    }
}

Viewfinder::~Viewfinder() {}

bool Viewfinder::on_timeout()
{
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win)
    {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
    return true;
}

bool Viewfinder::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {

	if (!cv_opened) return false;
 
	cv::Mat cv_frame, cv_frame1;

	cv_cap.read(cv_frame);
 
	if (cv_frame.empty()) return false;
 
	cv::cvtColor (cv_frame, cv_frame1, CV_BGR2RGB);
		 
	Gdk::Cairo::set_source_pixbuf (cr, Gdk::Pixbuf::create_from_data(cv_frame1.data, Gdk::COLORSPACE_RGB, false, 8, cv_frame1.cols, cv_frame1.rows, cv_frame1.step));
 
	cr->paint();
		 
	return true;

	/*
  if (!image)
    return false;

  Gtk::Allocation allocation = get_allocation();
  const int frame_width = allocation.get_width();
  const int frame_height = allocation.get_height();
  const int img_width = image->get_width();
  const int img_height = image->get_height();

  // Draw the image in the top right corner, fixed aspect ratio to fit the window
  double scale = std::min(frame_width /(double) img_width,
                          frame_height /(double) img_height);
  int scaled_width = img_width*scale;
  int scaled_height = img_height*scale;
  Glib::RefPtr<Gdk::Pixbuf> image_scaled = 
        image->scale_simple(scaled_width, 
                            scaled_height, 
                            Gdk::INTERP_BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, image_scaled, 0, 0);
  cr->paint();

  // TODO un-magic-number this, also move to function?
  cr->set_line_width(4);
  cr->set_source_rgb(1.0, 1.0, 1.0);

  cr->save();
  cr->move_to(50,10);
  cr->line_to(10,10);
  cr->line_to(10,50);
  cr->stroke();

  cr->move_to(scaled_width - 50,10);
  cr->line_to(scaled_width - 10,10);
  cr->line_to(scaled_width - 10,50);
  cr->stroke();

  cr->move_to(50,scaled_height - 10);
  cr->line_to(10,scaled_height - 10);
  cr->line_to(10,scaled_height - 50);
  cr->stroke();

  cr->move_to(scaled_width - 50,scaled_height - 10);
  cr->line_to(scaled_width - 10,scaled_height - 10);
  cr->line_to(scaled_width - 10,scaled_height - 50);
  cr->stroke();
  cr->restore();

  //cr->save();

  Glib::RefPtr<Pango::Layout> layout;
  if (current_state == HDR) {
    layout = create_pango_layout("HDR");
  } else if (current_state == AF) {
    layout = create_pango_layout("AF");
  } else {
    layout = create_pango_layout("...");
	}

  int text_width;
  int text_height;

  //get the text dimensions (it updates the variables -- by reference)
  layout->get_pixel_size(text_width, text_height);

  // Position the text in the top-left
  cr->move_to(20,20);

  layout->show_in_cairo_context(cr);

  //cr->restore();
  return true;
*/
}

void Viewfinder::setCameraState(CameraState state) {
  current_state = state;
}
