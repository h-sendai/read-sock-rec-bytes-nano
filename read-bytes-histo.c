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

#include <gsl/gsl_histogram.h>
#include <gsl/gsl_errno.h>

#include "my_signal.h"
#include "my_socket.h"
#include "set_timer.h"

int debug = 0;
gsl_histogram *histo;
unsigned long histo_overflow = 0;
unsigned long total_bytes = 0;
struct timeval start, stop;

int usage()
{
    char msg[] = "Usage: ./read-bytes-histo [-t TIMEOUT] ip_address port\n"
                 " Options:\n"
                 "    -t TIMEOUT: seconds.  (default: 10 seconds)\n";
    fprintf(stderr, "%s\n", msg);

    return 0;
}

void sig_int(int signo)
{
    struct timeval elapse;
    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &elapse);
    fprintf(stderr, "total bytes: %ld bytes\n", total_bytes);
    fprintf(stderr, "running %ld.%06ld sec\n", elapse.tv_sec, elapse.tv_usec);

    gsl_histogram_fprintf(stdout, histo, "%g", "%g");
    gsl_histogram_free(histo);
    if (histo_overflow > 0) {
        printf("overflow: %ld\n", histo_overflow);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    int n_bin = 30;
    int period = 10; /* default run time (10 seconds) */

    while ( (c = getopt(argc, argv, "dht:")) != -1) {
        switch (c) {
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
            default:
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 2) {
        usage();
        exit(1);
    }

    char *remote_host = argv[0];
    int port = strtol(argv[1], NULL, 0);
                
    my_signal(SIGINT,  sig_int);
    my_signal(SIGTERM, sig_int);
    my_signal(SIGALRM, sig_int);

    histo = gsl_histogram_alloc(n_bin);
    gsl_histogram_set_ranges_uniform(histo, 0, 1460*n_bin);

    int sockfd = tcp_socket();
    if (sockfd < 0) {
        errx(1, "tcp_socket");
    }

    if (connect_tcp(sockfd, remote_host, port) < 0) {
        errx(1, "connect_tcp");
    }

    set_timer(period, 0, period, 0);
    gettimeofday(&start, NULL);

    for ( ; ; ) {
        int n, m;
        char buf[2*1024*1024];
        n = read(sockfd, buf, sizeof(buf));
        total_bytes += n;
        m = gsl_histogram_increment(histo, n);
        if (m == GSL_EDOM) {
            histo_overflow += 1;
        }
    }
}
