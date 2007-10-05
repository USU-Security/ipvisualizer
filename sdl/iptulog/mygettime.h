/*
 * file:	mygettime.h
 * purpose:	i like my gettime function. in a .h for inlining
 * creator: 	rian shelley
 * created:	03-12-2007
 */

#ifndef MYGETTIME
#define MYGETTIME
/*
 * function:    gettime()
 * purpose: to return the time in milliseconds since the unix epoch
 * returns: didn't you read the purpose?
 */
inline unsigned long long gettime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}


#endif /*MYGETTIME_H*/
