/**
 * @file main.c
 * @brief Main entry point for the graph cycle detection program.
 *
 * @mainpage Cycle Detector
 *
 * @section intro_sec Introduction
 *
 * This is a command-line tool designed to detect cycles in transaction graphs.
 * It reads a file where each line represents a transaction (from, to, value),
 * builds a directed graph in memory, and uses a Depth-First Search (DFS)
 * algorithm to find and report all cycles.
 *
 * @section compile_sec Compilation
 *
 * To compile the project, simply run `make` from the root directory:
 * @code
 * make
 * @endcode
 *
 * @section run_sec Running the Program
 *
 * The program requires an input file with transaction data.
 * @code
 * ./main path/to/transactions.txt
 * @endcode
 *
 * For more detailed output during execution, use the verbose flag `-v`:
 * @code
 * ./main -v path/to/transactions.txt
 * @endcode
 *
 * The output containing the found cycles will be saved to a uniquely named file
 * in the working directory.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cli_parser.h"
#include "graph.h"

static FILE *openFile(char const *const filename);

/**
 * @brief The main function and entry point of the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return 0 on successful execution, 1 on error.
 */
int main(int argc, char **argv) {
    FILE *file = NULL;
    Graph graph = NULL;
    CLIOptions options;

    parse_cli_args(argc, argv, &options);

    if (options.show_help) {
        print_usage(argv[0]);
        return 0;
    }
    if (options.short_help){
        print_short_help(argv[0]);
        return 0;
    }

    if (options.positional_count < 1) {
        fprintf(stderr, "Incorrect usage: an input file is required.\n\n");
        print_short_help(argv[0]);
        return 1;
    }

    log_function_t logger = options.verbose ? log_verbose : log_silent;
    const char *const outName = make_output_filename(&options);
    logger("Output will be saved to: %s\n", outName);

    file = openFile(options.positionals[0]);
    if (file == NULL) {
        return 1; 
    }

    // TODO: Integrate with a tool to extract data or provide test files.
    graph = loadGraph(file, logger);
    if (graph == NULL) {
        fprintf(stderr, "Error: Failed to load graph from file.\n");
        fclose(file);
        return 1;
    }

    size_t total_cycles_found = 0;
    logger("\nStarting cycle detection...\n");
    clock_t start = clock();
    depthFirstSearch(graph, outName, logger, &total_cycles_found);
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    logger("\n-----------------------------------\n");
    logger("Cycle detection completed.\n");
    logger("Runtime to detect cycles: %f seconds\n", time_taken);
    logger("Total cycles found: %zu\n", total_cycles_found);
    logger("-----------------------------------\n");

    fclose(file);
    freeGraph(graph);
    freeVertexMap();
    graph = NULL;

    return 0;
}

/**
 * @brief Opens a file in read mode and handles errors.
 *
 * @param filename The path to the file to open.
 * @return A FILE pointer on success, or NULL on failure.
 */
static FILE *openFile(char const *const filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return NULL;
    }
    // TODO: Could add further validation to check if file is empty or has valid format.
    return file;
}