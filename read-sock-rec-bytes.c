#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "my_signal.h"
#include "my_socket.h"
#include "set_timer.h"
#include "get_num.h"

int debug = 0;
unsigned long total_bytes = 0;
unsigned long read_count  = 0;
struct timeval start, stop;
int bufsize = 2*1024*1024;
struct read_bytes_data {
    struct timeval tv;
    int read_bytes;
};
struct read_bytes_data *read_bytes_data = NULL;

int usage()
{
    char msg[] = "Usage: ./read-bytes-rec-bytes [-b bufsize] [-N max_read_count] [-t TIMEOUT] ip_address:port\n"
                 "print read bytes to stdout when timeout or read() max_read_count times\n"
                 "example of ip_address:port\n"
                 "remote_host:1234\n"
                 "192.168.10.16:24\n"
                 "default port: 1234\n"
                 " Options:\n"
                 "    -b bufsize: suffix k for kilo (1024), m for mega(1024*1024) (default 2MB)\n"
                 "    -t TIMEOUT: seconds.  (default: 10 seconds)\n"
                 "    -N max_read_count: default 10^6.\n"
                 "exit if read() done max_read_count times or timeout (default 10 sec)";
    fprintf(stderr, "%s\n", msg);

    return 0;
}

void sig_int(int signo)
{
    struct timeval elapse;
    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &elapse);
    fprintf(stderr, "bufsize: %.3f kB\n", bufsize/1024.0);
    fprintf(stderr, "total bytes: %ld bytes\n", total_bytes);
    fprintf(stderr, "read count : %ld\n", read_count);
    fprintf(stderr, "running %ld.%06ld sec\n", elapse.tv_sec, elapse.tv_usec);
    double elapsed_time        = elapse.tv_sec + 0.000001*elapse.tv_usec;
    double transfer_rate_MB_s  = (double)total_bytes / elapsed_time / 1024.0 / 1024.0;
    double read_bytes_per_read = (double)total_bytes / (double)read_count / 1024.0;
    fprintf(stderr, "transfer_rate: %.3f MB/s\n", transfer_rate_MB_s);
    fprintf(stderr, "read_bytes_per_read: %.3f kB/read\n", read_bytes_per_read);
    
    for (unsigned long i = 0; i < read_count; ++i) {
        timersub(&read_bytes_data[i].tv, &start, &elapse);
        printf("%ld.%06ld %d\n",
            elapse.tv_sec, elapse.tv_usec, read_bytes_data[i].read_bytes);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    int period = 10; /* default run time (10 seconds) */
    long max_read_count = 1000000;

    while ( (c = getopt(argc, argv, "b:dht:N:")) != -1) {
        switch (c) {
            case 'b':
                bufsize = get_num(optarg);
                break;
            case 'h':
                usage();
                exit(0);
                break; /* NOTREACHED */
            case 'd':
                debug = 1;
                break;
            case 't':
                period = strtol(optarg, NULL, 0);
                break;
            case 'N':
                max_read_count = strtol(optarg, NULL, 0);
                break;
            default:
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
        exit(1);
    }

    int port = 1234;
    char *remote_host_info = argv[0];
    char *tmp = strdup(remote_host_info);
    char *remote_host = strsep(&tmp, ":");
    if (tmp != NULL) {
        port = strtol(tmp, NULL, 0);
    }

    read_bytes_data = malloc(sizeof(struct read_bytes_data)*max_read_count);
    if (read_bytes_data == NULL) {
        err(1, "malloc for read_bytes_data");
    }
    for (unsigned long i = 0; i < max_read_count; ++i) {
        read_bytes_data[i].tv.tv_sec  = 0;
        read_bytes_data[i].tv.tv_usec = 0;
        read_bytes_data[i].read_bytes = 0;
    }

    my_signal(SIGINT,  sig_int);
    my_signal(SIGTERM, sig_int);
    my_signal(SIGALRM, sig_int);

    int sockfd = tcp_socket();
    if (sockfd < 0) {
        errx(1, "tcp_socket");
    }

    if (connect_tcp(sockfd, remote_host, port) < 0) {
        errx(1, "connect_tcp");
    }

    set_timer(period, 0, period, 0);
    gettimeofday(&start, NULL);

    char *buf = malloc(bufsize);
    if (buf == NULL) {
        err(1, "malloc for buf");
    }

    for ( ; ; ) {
        int n;
        n = read(sockfd, buf, bufsize);
        /* not mandatory in this program */
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                err(1, "read");
            }
        }
        gettimeofday(&read_bytes_data[read_count].tv, NULL);
        total_bytes += n;
        read_bytes_data[read_count].read_bytes = n;
        read_count ++;
        if (read_count == max_read_count) {
            sig_int(0);
            exit(0);
        }
    }
}
