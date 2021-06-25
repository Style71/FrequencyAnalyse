/**
 * @file getopt.h
 * Thread safe version of getopt
 */
#ifndef GETOPT_H
#define GETOPT_H

extern int optind;
extern int optopt;
extern const char *optarg;

int getopt(int argc, const char *argv[], const char *options);

#endif //GETOPT_H
