/* usleep.c - use it if you have no usleep on your system */

#include <sys/time.h>
#include <signal.h>

void handle_sigalrm(nr)
int nr;
{
    /* Do nothing! */
}

usleep(useconds)
  unsigned useconds;
{
    struct itimerval timer_value, old_timer_value;
    void (*old_sigval)();

#ifdef DEMONLORD
    /* If we have trouble with an "alarm clock" crashing pmf: */
    signal(SIGALRM, SIG_IGN);
#endif
    
    getitimer(ITIMER_REAL, &timer_value);
    timerclear(&timer_value.it_value);
    timerclear(&timer_value.it_interval);
    timer_value.it_interval.tv_sec = useconds / 1000000;
    timer_value.it_interval.tv_usec = useconds % 1000000;
    timer_value.it_value.tv_sec = useconds / 1000000;
    timer_value.it_value.tv_usec = useconds % 1000000;
    old_sigval = signal(SIGALRM, handle_sigalrm);
    if (setitimer(ITIMER_REAL, &timer_value, &old_timer_value) == -1)
	fatal("The call to setitimer failed.");
    pause();
    setitimer(ITIMER_REAL, &old_timer_value, 0);
    signal(SIGALRM, old_sigval);
} /* usleep */
