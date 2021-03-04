#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wait.h>

#include "args.h"        // parse command-line arguments
#include "global.h"      // variables that need to be accessed from multiple files
#include "setroot.h"     // functions relating to the desktop wallpaper
#include "util.h"        // useful things used by multiple files
#include "xlistener.h"   // functions for listening to xevents

int                  *keep_looping;
int                  *cont_execution;

int                   use_static_images = -1;
int                   quiet_on = -1;

int main(int argc, char *argv[])
{
  // add callbacks for signals
  signal(SIGINT, sig_callback);
  signal(SIGTERM, sig_callback);

  // NOTE: I HAVEN'T ACTUALLY IMPLEMENTED STATIC WALLPAPERS SO IT WILL USE ANIMATED ONES EITHER WAY
  // do things with optional arguments
  parse_args(argc, argv);
  if (use_static_images == 1)
  {
    prog_log("-> using static images for wallpaper\n");
  }
  else if (use_static_images == 0)
  {
    prog_log("-> using animated wallpaper\n");
  }
  else // (use_static_images == -1) so is unset
  {
    prog_log("-> wallpaper mode unset, using static images by default\n");
    use_static_images = 1;
  }

  // enable vars to be accessed and updated by both child and parent after fork()
  keep_looping = mmap(NULL, sizeof *keep_looping, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *keep_looping = 0;
  cont_execution = mmap(NULL, sizeof *cont_execution, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *cont_execution = 1;

  pid_t cpid, wpid;
  int status;
  cpid = fork();
  if (cpid < 0)
  {
    printf("ERROR: fork() failed\n");
    exit(1);
  }
  else if (cpid == 0)
  {
    // child process to listen for xevents
    start_xlistener();
  }
  else
  {
    // parent process to manage setting the wallpaper
    setroot_main();

    while ((wpid = wait(&status)) > 0);

    if (quiet_on == 1)
    {
      printf("stopped\n");
    }
  }

  return 0;
}
