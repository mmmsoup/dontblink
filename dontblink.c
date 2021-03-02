#include <Imlib2.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define NUM_OF_SCENES 6

Display              *XDPY;
unsigned int          XSCRN_NUM;
Screen               *XSCRN;
Window                ROOT_WIN;
int                   BITDEPTH;
Colormap              COLORMAP;
Visual               *VISUAL;
Atom                  prop_root, prop_esetroot;
int                  *keep_looping;
int                  *cont_execution;
int                   MILLISECONDS_PER_FRAME = 50;
char                  ROOT_DIR[256];
char                 *FRAME_FILE_NAME_FORMAT = "%05d.gif";
int                   currently_obscured = 0;

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

struct scene
{
  Pixmap *pmap;
  Imlib_Image *img;
  int fnum;
};
struct scene scene_array[NUM_OF_SCENES];

int handle_args(int argc, char *argv[])
{
  return 0;
}

int load_scene_from_file(char *folder_path, char *format, int scene_index)
{
  // load images from disk
  int frame_number = 0;
	char current_file[256];
	char file_name[256];

	snprintf(file_name, 128, format, frame_number);
	if (file_name[0] == '\0')
  {
		return 1;
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

  return 0;
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

void animate_root_background(int scene_index)
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
  printf("-> cleaning up\n");
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

    printf("   -> scene %d freed\n", i);
  }

  XCloseDisplay(XDPY);
}

void sig_callback(int sig)
{
  *cont_execution = 0;
  *keep_looping = 0;
  return;
}

int main(int argc, char *argv[])
{
  // add callbacks for signals
  signal(SIGINT, sig_callback);
  signal(SIGTERM, sig_callback);

  // do things with optional arguments
  if (handle_args(argc, argv) != 0)
  {
    printf("ERROR: parsing arguments failed\n");
    exit(1);
  }

  // enable multithreading with Xlib
  if (!XInitThreads())
  {
    printf("ERROR: XInitThreads() failed\n");
    exit(1);
  }

  keep_looping = mmap(NULL, sizeof *keep_looping, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *keep_looping = 0;

  cont_execution = mmap(NULL, sizeof *cont_execution, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *cont_execution = 1;

  pid_t cpid;
  cpid = fork();
  if (cpid < 0)
  {
    printf("ERROR: fork() failed\n");
    exit(1);
  }
  if (cpid == 0) // child process to listen for xevents
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
    printf("-> started xevent listener\n");
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
    printf("-> terminated xevent listener\n");
    return 0;
  }
  else // parent process to manage setting the wallpaper
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
    printf("-> generating pixmaps from files\n");
    char *home = getenv("HOME");
    strcpy(ROOT_DIR, home);
    strcat(ROOT_DIR, "/.dontblink/anim/");
    char frames_dir[256];
    for (int i = 0; i < NUM_OF_SCENES; i++ )
    {
      snprintf(frames_dir, 256, "%s%d/", ROOT_DIR, i);
      printf("   -> %s loaded\n", frames_dir);
      load_scene_from_file(frames_dir, FRAME_FILE_NAME_FORMAT, i);
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
      animate_root_background(current_scene);
    }

    printf("-> terminating...\n");
    clean_up();
  }

  return 0;
}
