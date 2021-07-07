/**
 * @file getopt.cpp
 * Minimal, thread safe version of getopt
 */

#include <cstdio>
#include "getopt.h"

int optind = 1;
int optopt;         // unknown option character or an option with a missing required argument,
const char *optarg; // char pointer points to arguments of related option.

/**
  * @brief  Check if p is a valid option and if the option takes an argument.
  * @param  p Option characters to be checked.
  * @param  options Option string that specifies the option characters that are valid for this program.
  * @param  takesarg If the matched option character takes an argument, takesarg is set to 1; 
  *                  if the argument is optional, takesarg is set to 2; else, 0.
  * @retval p if matches; '?' if mismatch.
  */
static char isvalidopt(char p, const char *options, int &takesarg)
{
    int idx = 0;
    takesarg = 0;

    while (options[idx] != 0 && p != options[idx])
    {
        ++idx;
    }

    if (options[idx] == 0)
    {
        return '?';
    }

    if (options[idx + 1] == ':')
    {
        takesarg = 1;
        if (options[idx + 2] == ':')
            takesarg = 2;
    }

    return options[idx];
}

/**
  * @brief  Reorder argv and put non-options at the end.
  * @param  argc Number of arguments in the array.
  * @param  argv Arguments string array.
  * @param  options Option string that specifies the option characters that are valid for this program.
  * @retval The last valid option index in argv.
  */       
static int reorder(int argc, const char *argv[], const char *options)
{
    const char *tmp_argv[argc];
    char c;
    int idx = 1;
    int optidx = 1;
    int nonoptidx = argc - 1;
    int takesarg;

    // move the options to the front, and add non-options to the end
    while (idx < argc && argv[idx] != 0)
    {
        if ((argv[idx][0] == '-') && ((c = isvalidopt(argv[idx][1], options, takesarg)) != '?'))
        {
            tmp_argv[optidx++] = argv[idx++];

            if ((takesarg) && (idx < argc) && (argv[idx][0] != '-'))
            {
                tmp_argv[optidx++] = argv[idx++];
            }
        }
        else
        {
            tmp_argv[nonoptidx--] = argv[idx++];
        }
    }

    // Reorder argv
    for (idx = 1; idx < argc; idx++)
    {
        argv[idx] = tmp_argv[idx];
    }

    //for (int i = 0; i < argc; i++)
    //    printf("argv[%d] = %s ", i, argv[i]);
    //printf("\n");

    return optidx - 1;
}

/**
  * @brief  Get valid option characters specified by options string.
  * @param  argc Number of arguments in the array.
  * @param  argv Arguments string array.
  * @param  options The options argument is a string that specifies the option characters that are valid for this program.
  *                 An option character in this string can be followed by a colon (‘:’) to indicate that it takes a required argument.
  *                 If an option character is followed by two colons (‘::’), its argument is optional; this is a GNU extension.
  * @param  optopt When getopt encounters an unknown option character or an option with a missing required argument,
  *                 it stores that option character in this global variable. You can use this for providing your own diagnostic messages.
  * @param  optind  This global variable is set by getopt to the index of the next element of the argv array to be processed.
  *                 Once getopt has found all of the option arguments, you can use this variable to determine where the remaining non-option arguments begin.
  *                 The initial value of this variable is 1.
  * @param  optarg This global variable is set by getopt to point at the value of the option argument, for those options that accept arguments.
  * @retval The valid option character is returned when option match occur; When no more option arguments are available, it returns -1.
  *         There may still be more non-option arguments; you must compare the external variable optind against the argc parameter to check this.
  *         If getopt finds an option character in argv that was not included in options, or a missing option argument,
  *         it returns ‘?’ and sets the external variable optopt to the actual option character..
  */
int getopt(int argc, const char *argv[], const char *options)
{
    const char *p;
    char c;
    int takesarg;
    static int optargc = 0; // Valid option index in reordered argv.

    if (optind == 1)
    {
        optargc = reorder(argc, argv, options);
    }

    if (optind > optargc)
    { //Error: option takes an argument, but there is no more argument
        return -1;
    }
    //printf("\noptargc = %d\n", optargc);
    //printf("optind: %d\n", optind);

    p = argv[optind++];

    optarg = NULL;

    if (p && options && optind && p[0] == '-')
    {
        c = isvalidopt(p[1], options, takesarg);
        optopt = p[1];

        if (c == '?')
        {
            return (int)'?';
        }

        if (takesarg)
        {
            if ((optind > optargc) || (argv[optind][0] == '-'))
            {
                if (takesarg==1)
                { //Error: option takes an argument, but there is no more argument
                    return (int)'?';
                }
                else// Argument is optional.
                    return (int)c;
            }

            optarg = argv[optind++];
        }

        return (int)c;
    }

    return -1;
}
