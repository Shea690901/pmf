/* usleep3.c: when both usleep.c and usleep2.c fails! */

usleep(useconds)
  unsigned useconds;
{
    sleep(1);
} /* usleep */
