#ifndef GLOBAL_H
  #define GLOBAL_H

  #include <Imlib2.h>

  #define NUM_OF_SCENES 6

  struct scene
  {
    Pixmap *pmap;
    Imlib_Image *img;
    int fnum;
  };
  extern struct scene scene_array[];

  extern int use_static_images;
  extern int quiet_on;

  extern int *cont_execution;
  extern int *keep_looping;

#endif
