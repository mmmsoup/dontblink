#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "global.h"
#include "util.h"

#include "xlistener.h"

static Display              *XDPY;
static unsigned int          XSCRN_NUM;
static Screen               *XSCRN;
static Window                ROOT_WIN;
static int                   BITDEPTH;
static Colormap              COLORMAP;
static Visual               *VISUAL;

void start_xlistener()
{
  XDPY = XOpenDisplay(NULL);
  XSCRN_NUM = DefaultScreen(XDPY);
  XSCRN = ScreenOfDisplay(XDPY, XSCRN_NUM);
  ROOT_WIN = RootWindow(XDPY, XSCRN_NUM);
  COLORMAP = DefaultColormap(XDPY, XSCRN_NUM);
  VISUAL = DefaultVisual(XDPY, XSCRN_NUM);
  BITDEPTH = DefaultDepth(XDPY, XSCRN_NUM);

  int screen_width = WidthOfScreen(XSCRN);
  int screen_height = HeightOfScreen(XSCRN);

  // check whether scene 0 has been started and if it has we can start listening for x events
  while (1)
  {
    usleep(1000000);
    if (*keep_looping)
    {
      break;
    }
  }

  XEvent event;
  XSelectInput(XDPY, ROOT_WIN, SubstructureNotifyMask);
  prog_log("-> started xevent listener\n");
  while (*cont_execution)
  {
    XNextEvent(XDPY, &event);
    switch (event.type)
    {
      case ConfigureNotify:
        if (event.xconfigure.width == screen_width && event.xconfigure.height == screen_height)
        {
          if (*keep_looping)
          {
            *keep_looping = 0;
          }
        }
        break;
      default:
        break;
    }
  }

  XCloseDisplay(XDPY);
  prog_log("-> terminated xevent listener\n");
  exit(0);
}
