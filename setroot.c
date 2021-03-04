#include <Imlib2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "global.h"
#include "util.h"

#include "setroot.h"

static Display              *XDPY;
static unsigned int          XSCRN_NUM;
static Screen               *XSCRN;
static Window                ROOT_WIN;
static int                   BITDEPTH;
static Colormap              COLORMAP;
static Visual               *VISUAL;

int                   MILLISECONDS_PER_FRAME = 50;
char                  ROOT_DIR[256];
char                 *ANIM_FRAME_FILE_NAME_FORMAT = "%05d.gif";
char                 *STATIC_FRAME_FILE_NAME_FORMAT = "%05d.png";

struct scene scene_array[NUM_OF_SCENES];

Atom prop_root, prop_esetroot;

struct timespec timespec_get_difference(struct timespec start, struct timespec end)
{
    struct timespec temp;

    if ((end.tv_nsec-start.tv_nsec)<0)
    {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
};

void load_scene_from_file(char *folder_path, char *format, int scene_index)
{
  // load images from disk
  int frame_number = 0;
	char current_file[256];
	char file_name[256];

	snprintf(file_name, 128, format, frame_number);
	if (file_name[0] == '\0')
  {
    printf("ERROR: unable to load frames from file\n");
		exit(1);
	}

	memset(current_file, 0, sizeof(current_file));
	strcat(current_file, folder_path);
	strcat(current_file, file_name);
  Imlib_Image *images = NULL;

	while (access(current_file, R_OK) != -1)
  {
		Imlib_Image temp_image;
		temp_image = imlib_load_image(current_file);
		frame_number++;
		images = realloc(images, frame_number*sizeof(char*));
		*(images+frame_number-1) = temp_image;
		memset(current_file, 0, sizeof(current_file));
		snprintf(file_name, 128, format, frame_number);
		strcat(current_file, folder_path);
		strcat(current_file, file_name);
	}

  // generate pixmaps from images
  int width = WidthOfScreen(XSCRN);
	int height = HeightOfScreen(XSCRN);
	Pixmap *temp = malloc(sizeof(Pixmap)*frame_number);

	for (int i = 0; i < frame_number; i++)
  {
		*(temp+i) = XCreatePixmap(XDPY, ROOT_WIN, width, height, BITDEPTH);
		imlib_context_set_drawable(*(temp+i));
		imlib_context_set_image(*(images+i));
		imlib_render_image_on_drawable(0, 0);
	}

  scene_array[scene_index].img = images;
  scene_array[scene_index].fnum = frame_number;
  scene_array[scene_index].pmap = temp;

  // free images
  for (int i = 0; i < frame_number; i++)
  {
		imlib_context_set_image(images[i]);
		imlib_free_image();
	}

  return;
}

void set_pixmap_property(Pixmap p)
{
//  Atom prop_root, prop_esetroot;
	Atom type;
  int format;
  unsigned long length, after;
  unsigned char *data_root, *data_esetroot;

  if ((prop_root != None) && (prop_esetroot != None))
  {
    XGetWindowProperty(XDPY, ROOT_WIN, prop_root, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_root);
    if (type == XA_PIXMAP)
    {
      XGetWindowProperty(XDPY, ROOT_WIN, prop_esetroot, 0L, 1L, False, AnyPropertyType, &type, &format, &length, &after, &data_esetroot);
      if (data_esetroot) { free(data_esetroot); }
    }
    if (data_root) { free(data_root); }
  }

//  prop_root = XInternAtom(XDPY, "_XROOTPMAP_ID", False);
//  prop_esetroot = XInternAtom(XDPY, "ESETROOTPMAP_ID", False);

  XChangeProperty(XDPY, ROOT_WIN, prop_root, XA_PIXMAP, 32,
                    PropModeReplace, (unsigned char *) &p, 1);
//    XChangeProperty(XDPY, ROOT_WIN, prop_esetroot, XA_PIXMAP, 32,
//                    PropModeReplace, (unsigned char *) &p, 1);

//    XSetCloseDownMode(XDPY, RetainPermanent);
  XFlush(XDPY);
}

void set_root_background_static(int scene_index)
{
  Pixmap current_pixmap = *(scene_array[scene_index].pmap);
  set_pixmap_property(current_pixmap);
  XSetWindowBackgroundPixmap(XDPY, ROOT_WIN, current_pixmap);
  XClearWindow(XDPY, ROOT_WIN);

  // check periodically whether it needs to stop so the root background can be changed
  while (*keep_looping)
  {
    usleep(MILLISECONDS_PER_FRAME*1000);
  }

  return;
}

void set_root_background_anim(int scene_index)
{
  struct scene s = scene_array[scene_index];

  struct timespec start={0,0}, end={0,0};
	int nsec_per_frame = MILLISECONDS_PER_FRAME*1000; // *1000 to turn msec into nsec
  int current = 0;
	int difference;
	clock_gettime(CLOCK_REALTIME, &start);
	Pixmap current_pixmap;

	while (*keep_looping)
  {
		current_pixmap = *(s.pmap+current);
		set_pixmap_property(current_pixmap);

		XSetWindowBackgroundPixmap(XDPY, ROOT_WIN, current_pixmap);
		XClearWindow(XDPY, ROOT_WIN);

		current++;
		if (current >= s.fnum)
    {
			current = 0;
		}

		clock_gettime(CLOCK_REALTIME, &end);
		difference = timespec_get_difference(start, end).tv_nsec/1000;
		if (difference > nsec_per_frame)
    {
			difference = nsec_per_frame;
		}
		usleep(nsec_per_frame-difference);
		clock_gettime(CLOCK_REALTIME, &start);
	}

  return;
}

void clean_up()
{
  Pixmap *pix;
  Imlib_Image *img;
  prog_log("-> cleaning up\n");
  for (int i = 0; i < NUM_OF_SCENES; i++)
  {
    pix = scene_array[i].pmap;
    img = scene_array[i].img;
    for (int j = 0; j < scene_array[i].fnum; j++)
    {
      XFreePixmap(XDPY, *(pix+j));
    }

    free(pix);
    free(img);

    snprintf(log_str_buf, 256, "   -> scene %d freed\n", i);
    prog_log(log_str_buf);
  }

  XCloseDisplay(XDPY);
}

void setroot_main()
{
  // set variables
  XDPY = XOpenDisplay(NULL);
  XSCRN_NUM = DefaultScreen(XDPY);
  XSCRN = ScreenOfDisplay(XDPY, XSCRN_NUM);
  ROOT_WIN = RootWindow(XDPY, XSCRN_NUM);
  COLORMAP = DefaultColormap(XDPY, XSCRN_NUM);
  VISUAL = DefaultVisual(XDPY, XSCRN_NUM);
  BITDEPTH = DefaultDepth(XDPY, XSCRN_NUM);

  imlib_context_set_display(XDPY);
  imlib_context_set_visual(VISUAL);
  imlib_context_set_colormap(COLORMAP);

  // load files and generate pixmaps
  prog_log("-> generating pixmaps from files\n");
  char *home = getenv("HOME");
  strcpy(ROOT_DIR, home);
  strcat(ROOT_DIR, "/.dontblink");
  char frames_dir[256];
  char *path_end;
  char *format;
  if (use_static_images)
  {
    path_end = "%s/%d/";
    format = STATIC_FRAME_FILE_NAME_FORMAT;
  }
  else
  {
    path_end = "%s/%d/anim/";
    format = ANIM_FRAME_FILE_NAME_FORMAT;
  }
  for (int i = 0; i < NUM_OF_SCENES; i++ )
  {
    snprintf(frames_dir, 256, path_end, ROOT_DIR, i);
    load_scene_from_file(frames_dir, format, i);

    if (use_static_images)
    {
      snprintf(log_str_buf, 256, "   -> %s00000.png loaded\n", frames_dir);
    }
    else
    {
      snprintf(log_str_buf, 256, "   -> %s loaded\n", frames_dir);
    }
    prog_log(log_str_buf);
  }

  // set atoms
  prop_root = XInternAtom(XDPY, "_XROOTPMAP_ID", False);
  prop_esetroot = XInternAtom(XDPY, "ESETROOTPMAP_ID", False);
  if (prop_root == None || prop_esetroot == None)
  {
    printf("ERROR: failed to get _XROOTPMAP_ID or ESETROOTPMAP_ID atom\n");
    clean_up();
    exit(1);
  }

  // display scenes
  int current_scene = -1;
  while(*cont_execution)
  {
    current_scene += 1;
    if (current_scene == NUM_OF_SCENES)
    {
      current_scene = 0;
    }
    *keep_looping = 1;
    if (use_static_images)
    {
      set_root_background_static(current_scene);
    }
    else
    {
      set_root_background_anim(current_scene);
    }
  }

  prog_log("-> terminating...\n");
  clean_up();
}
