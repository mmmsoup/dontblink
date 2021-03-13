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

Window obscuring_windows_arr[64];
int obscuring_windows_num = 0;

/* get index of window in array if exists, else return -1 */
int win_index(Window win)
{
  if (obscuring_windows_num == 0)
  {
    return -1;
  }

  for (int i = 0; i < obscuring_windows_num; i++)
  {
    if (obscuring_windows_arr[i] == win)
    {
      return i;
    }
  }
  return -1;
}

void pop_window(int index)
{
  for (int i = index; i < obscuring_windows_num; i++)
  {
    obscuring_windows_arr[i] = obscuring_windows_arr[i+1];
  }
  return;
}

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

  Atom netwmstate = XInternAtom(XDPY, "_NET_WM_STATE", False);
  Atom netwmfullscreen = XInternAtom(XDPY, "_NET_WM_STATE_FULLSCREEN", False);
  Atom netwmmaxhorz = XInternAtom(XDPY, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  Atom netwmmaxvert = XInternAtom(XDPY, "_NET_WM_STATE_MAXIMIZED_VERT", False);
  Atom type;
  Window win_actual, win_identifier;
  XEvent event;
  int format, status, revert_to, win_arr_index;
  int currently_obscured = 0;
  unsigned long length, after;
  unsigned char *val;

  XSelectInput(XDPY, ROOT_WIN, SubstructureNotifyMask);
  prog_log("-> started xevent listener\n");

  while (*cont_execution)
  {
    XNextEvent(XDPY, &event);
    switch (event.type)
    {
      case ConfigureNotify:
        XGetInputFocus(XDPY, &win_actual, &revert_to);
        status = XGetWindowProperty(XDPY, win_actual, netwmstate, 0, 65536, False, AnyPropertyType, &type, &format, &length, &after, &val);

        if (length == 0) /* not obscuring desktop */
        {
          win_arr_index = win_index(win_actual);
          if (win_arr_index == -1)
          {
            break;
          }
          else
          {
            pop_window(win_arr_index);
            obscuring_windows_num -= 1;
          }
          break;
        }
        else
        {
          if (length == 1 && ((Atom*)val)[0] == netwmfullscreen)
          {
            if (win_index(win_actual) == -1)
            {
              obscuring_windows_arr[obscuring_windows_num] = win_actual;
              obscuring_windows_num += 1;
            }
            break;
          }
          else
          {
            int max_in_one_direction;
            Atom p;
            for (int i = 0; i < length; i++)
            {
              p = ((Atom*)val)[i];
              if (p == netwmfullscreen)
              {
                if (win_index(win_actual) == -1)
                {
                  obscuring_windows_arr[obscuring_windows_num] = win_actual;
                  obscuring_windows_num += 1;
                }
                break;
              }
              else if (p == netwmmaxhorz || p == netwmmaxvert)
              {
                if (max_in_one_direction == 1)
                {
                  if (win_index(win_actual) == -1)
                  {
                    obscuring_windows_arr[obscuring_windows_num] = win_actual;
                    obscuring_windows_num += 1;
                  }
                  break;
                }
                else
                {
                  max_in_one_direction = 1;
                }
              }
            }
          }
        }
        break;
      default:
        break;
    }

    if (obscuring_windows_num == 0)
    {
      currently_obscured = 0;
    }
    else
    {
      if (currently_obscured == 0)
      {
        // change background
        *keep_looping = 0;
        currently_obscured = 1;
      }
    }
  }

  XCloseDisplay(XDPY);
  prog_log("-> terminated xevent listener\n");
  return;
}
