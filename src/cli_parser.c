      
/**
 * @file cli_parser.c
 * @brief Implementation of the command-line argument parser.
 * @defgroup cli_parser CLI Parser
 * @{
 */    

#include "cli_parser.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Forward declaration for static helper function
static const char* make_unique_filename(const char* prefix, const char* filetype);

/**
 * @brief Parses the command-line arguments and populates the CLIOptions struct.
 * @param argc The argument count from main().
 * @param argv The argument vector from main().
 * @param opts A pointer to the CLIOptions struct to be filled.
 */
void parse_cli_args(int argc, char **argv, CLIOptions *opts) {
    // Default values
    opts->output_file = NULL;
    opts->verbose = false;
    opts->show_help = false;
    opts->short_help = false;
    opts->user_specified_output = false;
    opts->positional_count = 0;
    opts->positionals = NULL;

    static struct option long_options[] = {
        {"help",    no_argument,       NULL, 'h'},
        {"output",  required_argument, NULL, 'o'},
        {"verbose", no_argument,       NULL, 'v'},
        {"usage",   no_argument,       NULL, 'u'},
        {0, 0, 0, 0}
    };
    const char *optstring = "uho:v";

    int opt;
    while ((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
        switch (opt) {
            case 'u':
                opts->short_help = true;
                break;
            case 'h':
                opts->show_help = true;
                break;
            case 'o':
                opts->output_file = optarg;
                opts->user_specified_output = true;
                break;
            case 'v':
                opts->verbose = true;
                break;
            case '?': // getopt_long already printed an error message.
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Store positional arguments
    opts->positional_count = argc - optind;
    if (opts->positional_count > 0) {
        opts->positionals = &argv[optind];
    }
}

/**
 * @brief Prints a short usage message to stdout.
 * @param progname The name of the program (argv[0]).
 */
void print_short_help(const char *progname){
    printf("Usage: %s [OPTIONS] [FILES...]\n", progname);
}

/**
 * @brief Prints the detailed help/usage message to stdout.
 * @param progname The name of the program (argv[0]).
 */
void print_usage(const char *progname) {
    printf("Usage: %s [OPTIONS] [FILES...]\n", progname);
    puts("Options:");
    puts("  -u, --usage          Display short usage message and exit");
    puts("  -h, --help           Display this help and exit");
    puts("  -o, --output <file>  Defines output file");
    puts("  -v, --verbose        Enables verbose mode");
    // TODO: explain in detailed form how to use the program
}

/**
 * @brief Creates a unique filename with a timestamp.
 * @param prefix The prefix for the filename.
 * @param filetype The file extension (e.g., "txt").
 * @return A pointer to a static buffer containing the unique filename.
 * @note The returned string is stored in a static buffer and will be
 *       overwritten by subsequent calls.
 */
static const char* make_unique_filename(const char* prefix, const char* filetype) {
    static char filename[256];
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info); // Using thread-safe localtime_r

    snprintf(filename, sizeof(filename),
             "%s--%04d-%02d-%02d_%02d-%02d-%02d.%s", prefix,
             tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, filetype);
    return filename;
}

/**
 * @brief Determines the output filename.
 *
 * If the user specified an output file, it returns that name. Otherwise, it
 * generates a unique filename based on the current timestamp.
 *
 * @param opts A pointer to the populated CLIOptions struct.
 * @return A constant character pointer to the determined output filename.
 */
const char *make_output_filename(const CLIOptions *opts) {
    if (opts->user_specified_output) {
        return opts->output_file;
    }
    return make_unique_filename("output", "txt");
}

 /** @} */ 