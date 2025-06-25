/**
 * @file graph.h
 * @brief Defines graph data structures and related function prototypes.
 *
 * This file contains definitions for the graph, transactions, vertex map (hash table),
 * and the function signatures for graph manipulation, cycle detection, and data loading.
 */

#ifndef CF34E36C_803A_4D30_AC71_0842482F2317
#define CF34E36C_803A_4D30_AC71_0842482F2317

#include <gmp.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "uthash.h"
#include "wei_parser.h" // Assumed to exist for parse_wei_ctx_t

// --- TYPE DEFINITIONS ---

/** @typedef vertex
 *  @brief A type alias for a graph vertex, represented by an integer.
 */
typedef int vertex;

/**
 * @struct VertexMap
 * @brief A hash map entry that maps a string key (wallet address) to an integer index.
 * Uses uthash for the hash table implementation.
 */
typedef struct vertex_map {
    char key[43];      /**< The string key (wallet address). */
    size_t index;      /**< The integer index corresponding to the key. */
    UT_hash_handle hh; /**< Handle for uthash. */
} VertexMap;

/**
 * @struct Transaction
 * @brief A node in the adjacency list, representing a directed edge in the graph.
 */
typedef struct transaction {
    vertex destination;          /**< The destination vertex of the transaction. */
    mpz_t transactionValue;      /**< The value of the transaction (using GMP for large numbers). */
    struct transaction *next;    /**< Pointer to the next transaction in the list. */
} Transaction;

/**
 * @struct GraphDS
 * @brief The main graph data structure using an adjacency list.
 */
typedef struct {
    size_t vertexAmount;         /**< The number of vertices in the graph. */
    size_t edgesAmount;          /**< The number of edges in the graph. */
    Transaction **adjList;       /**< An array of pointers to Transaction lists (the adjacency list). */
} GraphDS;

/** @typedef Graph
 *  @brief A pointer to the GraphDS struct.
 */
typedef GraphDS *Graph;


/** @typedef log_function_t
 *  @brief A function pointer type for logging messages.
 */
typedef void (*log_function_t)(const char *, ...);

/**
 * @struct LogInfo_t
 * @brief A struct to hold logging and performance metrics.
 */
typedef struct {
    size_t walletsAmount;
    size_t transactionAmount;
    size_t cyclesFound;
    double runtimeFillHashmap;
    double runtimeAlgorithm;
    double runtimeCreateGraph;
    char algorithmUsed[64];
    const char *outputFileName;
} LogInfo_t;


// --- GLOBAL VARIABLES ---

/** @var vertex_map
 *  @brief Global pointer to the head of the vertex hash map.
 */
extern VertexMap *vertex_map;

// --- HASHMAP FUNCTIONS ---

/**
 * @brief Adds all unique vertices from a file to the hash map.
 * @param file Pointer to the input file containing transactions.
 * @return The total number of unique vertices (wallets) found.
 */
size_t addAllVertexToHashmap(FILE *file);

/**
 * @brief Retrieves the integer index for a given vertex name (wallet address).
 * @param name The string key (wallet address) to look up.
 * @return The index of the vertex.
 */
size_t getVertexIndex(const char *name);

/**
 * @brief Frees all memory allocated for the vertex hash map.
 */
void freeVertexMap();

// --- GRAPH LOADER FUNCTIONS ---

/**
 * @brief Loads a graph from a file.
 *
 * This function first populates a hash map with all vertices, then initializes
 * a graph of the appropriate size, and finally populates the graph with edges.
 * It uses the provided logger to report progress.
 *
 * @param file A pointer to the opened input file.
 * @param logger The logging function to use (log_verbose or log_silent).
 * @return An initialized and populated graph.
 */
Graph loadGraph(FILE *file, log_function_t logger);

// --- GRAPH MANIPULATION FUNCTIONS ---

/**
 * @brief Initializes a graph with V vertices and empty adjacency lists.
 * @param V The number of vertices for the graph.
 * @return A pointer to the newly allocated and initialized graph.
 */
Graph initGraph(size_t V);

/**
 * @brief Inserts a directed edge from vertex v to vertex w with a given value.
 * @param G The graph.
 * @param v The source vertex.
 * @param w The destination vertex.
 * @param value The value of the transaction (edge weight).
 * @return 1 on success, 0 on failure.
 */
int insertEdge(Graph G, vertex v, vertex w, const mpz_t value);

/**
 * @brief Frees all memory associated with the graph.
 * @param G The graph to be freed.
 */
void freeGraph(Graph G);

/**
 * @brief Displays the graph's adjacency list representation.
 * @param G The graph to display.
 */
void showGraph(Graph G);

// --- CYCLE DETECTION (DFS) FUNCTIONS ---

/**
 * @brief The main function to perform Depth First Search and find cycles.
 * @param G The graph to search.
 * @param filename The name of the file to write cycle information to.
 * @param logger The logging function to use (log_verbose or log_silent).
 * @param total_cycles_found A pointer to a size_t to store the count of found cycles.
 */
void depthFirstSearch(Graph G, const char *const filename, log_function_t logger, size_t *total_cycles_found);

// --- LOGGING FUNCTIONS ---

/**
 * @brief A logging function that prints messages to stdout.
 * @param format The format string.
 * @param ... Variable arguments for the format string.
 */
void log_verbose(const char *format, ...);

/**
 * @brief A logging function that does nothing (silent mode).
 * @param format The format string.
 * @param ... Variable arguments for the format string.
 */
void log_silent(const char *format, ...);


#endif /* CF34E36C_803A_4D30_AC71_0842482F2317 */