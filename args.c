#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global.h" // access global vars

#include "args.h"

void show_help()
{
  printf("usage: 'dontblink [option]'\n");
  printf("options:\n");
  printf("   -a - force use of animated wallpaper (heavier on resources but looks cooler)\n");
  printf("   -s - force use of static images (easier on resources and loads quicker)\n");
  printf("   -q - run in quiet mode (silences most output)\n");
  printf("   -h - display this help dialogue\n");
  return;
}

void parse_args(int argc, char *argv[])
{
  int opt;

  while ((opt = getopt(argc, (char**)argv, "asqh")) != -1)
  {
    switch (opt)
    {
      case 'a':
        if (use_static_images != -1)
        {
          printf("'-a': you've already specified a wallpaper mode'\n");
          exit(1);
        }
        else
        {
          use_static_images = 0;
        }
        break;
      case 's':
        if (use_static_images != -1)
        {
          printf("'-s': you've already specified a wallpaper mode'\n");
          exit(1);
        }
        else
        {
          use_static_images = 1;
        }
        break;
      case 'q':
        if (quiet_on != -1)
        {
          printf("'-q': quiet mode already enabled\n");
          exit(1);
        }
        else
        {
          quiet_on = 1;
        }
        break;
      case 'h':
        if (strcmp(argv[1], "-h") == 0)
        {
          show_help();
          exit(0);
        }
        else
        {
          printf("'-h': flag invalid in this context - run 'dontblink -h' for help\n");
          exit(1);
        }
      case '?':
        show_help();
        exit(0);
    }
  }

  if (quiet_on == -1)
  {
    quiet_on = 0;
  }

  return;
}
