#ifndef __REQUEST_H__


struct Statistics{
    struct timeval arrival_time;
    struct timeval dispatch_time;
    int thread_id;
    int thread_count;
    int thread_static_count;
    int thread_dynamic_count;
};

void requestHandle(int fd, struct Statistics* stats);

#endif
