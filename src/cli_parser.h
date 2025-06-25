/**
 * @file cli_parser.h
 * @brief Defines the structures and functions for parsing command-line arguments.
 */

#ifndef B0EF9175_7456_442B_BD53_7B1A58C47678
#define B0EF9175_7456_442B_BD53_7B1A58C47678

#include <stdbool.h>

/**
 * @struct CLIOptions
 * @brief Holds the configuration options specified by the command-line arguments.
 */
typedef struct {
    /** @var output_file Path to the output file specified with --output or -o. */
    const char *output_file;

    /** @var verbose Flag for verbose mode, enabled with --verbose or -v. */
    bool verbose;

    /** @var show_help Flag to display the detailed help message, enabled with --help or -h. */
    bool show_help;

    /** @var short_help Flag to display the short usage message, enabled with --usage or -u. */
    bool short_help;

    /** @var user_specified_output True if the user provided an output file, false otherwise. */
    bool user_specified_output;

    /** @var positional_count The number of positional arguments (e.g., input files). */
    int positional_count;

    /** @var positionals An array of strings containing the positional arguments. */
    char **positionals;
} CLIOptions;

/**
 * @brief Parses the command-line arguments and populates the CLIOptions struct.
 * @param argc The argument count from main().
 * @param argv The argument vector from main().
 * @param opts A pointer to the CLIOptions struct to be filled.
 */
void parse_cli_args(int argc, char **argv, CLIOptions *opts);

/**
 * @brief Prints the detailed help/usage message to stdout.
 * @param progname The name of the program (argv[0]).
 */
void print_usage(const char *progname);

/**
 * @brief Prints a short usage message to stdout.
 * @param progname The name of the program (argv[0]).
 */
void print_short_help(const char *progname);

/**
 * @brief Determines the output filename.
 *
 * If the user specified an output file, it returns that name. Otherwise, it
 * generates a unique filename based on the current timestamp.
 *
 * @param opts A pointer to the populated CLIOptions struct.
 * @return A constant character pointer to the determined output filename.
 */
const char *make_output_filename(const CLIOptions *opts);

#endif /* B0EF9175_7456_442B_BD53_7B1A58C47678 */