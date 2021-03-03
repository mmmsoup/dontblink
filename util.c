#include <stdarg.h>
#include <stdio.h>

#include "global.h"

#include "util.h"

/*
for using printf formatting with prog_log() -
   -> snprintf(log_str_buf, 256, ... );
   -> prog_log(log_str_buf);
*/
char log_str_buf[256];

void prog_log(char *output)
{
  if (!quiet_on)
  {
    printf(output);
  }
}

void sig_callback(int sig)
{
  *cont_execution = 0;
  *keep_looping = 0;
  return;
}
