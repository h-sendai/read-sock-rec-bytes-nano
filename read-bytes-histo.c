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

int debug = 0;
gsl_histogram *histo;
unsigned long histo_overflow = 0;
unsigned long total_bytes = 0;

int usage()
{
    char msg[] = "Usage: ./read-bytes-histo ip_address port";
    fprintf(stderr, "%s\n", msg);

    return 0;
}

void sig_int(int signo)
{
    fprintf(stderr, "total bytes: %ld bytes\n", total_bytes);

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

    while ( (c = getopt(argc, argv, "d")) != -1) {
        switch (c) {
            case 'd':
                debug = 1;
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
    histo = gsl_histogram_alloc(n_bin);
    gsl_histogram_set_ranges_uniform(histo, 0, 1460*n_bin);

    int sockfd = tcp_socket();
    if (sockfd < 0) {
        errx(1, "tcp_socket");
    }

    if (connect_tcp(sockfd, remote_host, port) < 0) {
        errx(1, "connect_tcp");
    }

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
