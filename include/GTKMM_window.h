#ifndef GTKMM_WINDOW_H
#define GTKMM_WINDOW_H

#include <gtkmm.h>
//#include <gtkmm/window.h>
//#include <gtkmm/frame.h>

class GTKMMWindow : public Gtk::Window
{
public:
  GTKMMWindow();
  virtual ~GTKMMWindow();

protected:

  //Child widgets:
  Gtk::Frame m_Frame;
};

#endif //GTKMM_WINDOW_H
