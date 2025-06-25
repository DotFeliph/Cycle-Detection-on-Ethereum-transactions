      
/**
 * @file graph.c
 * @brief Implementation of graph data structures and algorithms.
 * @defgroup graph_logic Graph Logic
 * @{
 */
    

#include "graph.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- GLOBAL DEFINITION ---

/**
 * @brief Global pointer to the head of the vertex hash map.
 */
VertexMap *vertex_map = NULL;

// --- FORWARD DECLARATIONS FOR STATIC HELPERS ---

static void addToHash(const char *name, size_t *current_index);
static Graph hashToGraph(FILE *file, Graph G, size_t totalTransactions, parse_wei_ctx_t *ctx);
static void recursiveDFS(Graph G, int *visited, int *recStack, int *path, mpz_t *valuesPath, int *depth, vertex v, FILE *p, log_function_t log_func, size_t *cycle_count);

// --- HASHMAP FUNCTIONS ---

/**
 * @brief Adds a new vertex to the hash map if it doesn't already exist.
 * @param name The vertex name (wallet address).
 * @param current_index A pointer to the next available index, which is incremented if a new vertex is added.
 */
static void addToHash(const char *name, size_t *current_index) {
    VertexMap *v;
    HASH_FIND_STR(vertex_map, name, v);
    if (v == NULL) {
        v = malloc(sizeof(VertexMap));
        if (!v) {
            fprintf(stderr, "Fatal: malloc failed in addToHash.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(v->key, name);
        v->index = (*current_index)++;
        HASH_ADD_STR(vertex_map, key, v);
    }
}

/**
 * @brief Fills the hash map with all unique vertices from a file.
 * @param file Pointer to the input file containing transactions.
 * @return The total number of unique vertices found.
 */
size_t addAllVertexToHashmap(FILE *file) {
    char from_ad[43], to_ad[43], value_str[100];
    size_t index = 0;

    while (fscanf(file, "%42s %42s %99s", from_ad, to_ad, value_str) == 3) {
        addToHash(from_ad, &index);
        addToHash(to_ad, &index);
    }
    return index;
}

/**
 * @brief Retrieves the integer index for a given vertex name.
 * @param name The wallet address to look up.
 * @return The index of the vertex. Returns 0 if not found (assuming valid indices are > 0 or checks are done).
 * @note This function assumes the vertex exists. A robust implementation might handle misses.
 */
size_t getVertexIndex(const char *name) {
    VertexMap *v;
    HASH_FIND_STR(vertex_map, name, v);
    // This assumes v will not be NULL. The caller must ensure 'name' is in the map.
    return v->index;
}

/**
 * @brief Frees all memory allocated for the vertex hash map.
 */
void freeVertexMap() {
    VertexMap *current, *tmp;
    HASH_ITER(hh, vertex_map, current, tmp) {
        HASH_DEL(vertex_map, current);
        free(current);
    }
}

// --- GRAPH LOADER FUNCTIONS ---

/**
 * @brief Reads transaction data from a file and populates the graph with edges.
 * @param file The input file, rewound to the beginning.
 * @param G The graph to populate.
 * @param totalTransactions The number of lines/transactions to read.
 * @param ctx The context for the wei parser.
 * @return The populated graph G.
 */
static Graph hashToGraph(FILE *file, Graph G, size_t totalTransactions, parse_wei_ctx_t *ctx) {
    char from_ad[43], to_ad[43], value_str[100];
    size_t from_index, to_index;

    mpz_t parsed_value;
    mpz_init(parsed_value);

    for (size_t i = 0; i < totalTransactions; i++) {
        if (fscanf(file, "%42s %42s %99s", from_ad, to_ad, value_str) != 3) {
            fprintf(stderr, "Warning: Malformed line at transaction %zu. Skipping.\n", i);
            continue;
        }

        if (parse_wei_optimized(ctx, value_str, parsed_value) != 0) {
            fprintf(stderr, "Warning: Failure parsing the value '%s'. Skipping transaction.\n", value_str);
            continue;
        }

        from_index = getVertexIndex(from_ad);
        to_index = getVertexIndex(to_ad);

        insertEdge(G, from_index, to_index, parsed_value);
    }
    mpz_clear(parsed_value);
    return G;
}

/**
 * @brief Loads a graph from a file.
 * @param file A pointer to the opened input file.
 * @param logger The logging function to use for progress messages.
 * @return An initialized and populated graph.
 */
Graph loadGraph(FILE *file, log_function_t logger) {
    parse_wei_ctx_t *ctx = parse_wei_ctx_create();
    if (!ctx) {
        fprintf(stderr, "Fatal error: unable to create wei_parser context.\n");
        exit(EXIT_FAILURE);
    }

    logger("Processing vertices...\n");
    clock_t start = clock();
    size_t vertexCount = addAllVertexToHashmap(file);
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    logger("Runtime to fill hashtable: %lf seconds\n", time_taken);
    logger("Total unique wallets (vertices): %zu\n", vertexCount);

    rewind(file);
    Graph graph = initGraph(vertexCount);

    logger("Building graph...\n");
    start = clock();
    graph = hashToGraph(file, graph, graph->vertexAmount, ctx);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Runtime to create graph: %lf seconds\n", time_taken);

    parse_wei_ctx_free(ctx);
    return graph;
}

// --- GRAPH MANIPULATION FUNCTIONS ---

/**
 * @brief Initializes a graph with V vertices.
 * @param V The number of vertices.
 * @return A pointer to the newly allocated graph.
 */
Graph initGraph(size_t V) {
    Graph G = malloc(sizeof(GraphDS));
    if (!G) {
        fprintf(stderr, "Error: Could not allocate memory for the graph.\n");
        exit(EXIT_FAILURE);
    }
    G->vertexAmount = V;
    G->edgesAmount = 0;
    G->adjList = malloc(V * sizeof(Transaction *));

    if (!G->adjList) {
        fprintf(stderr, "Error: Could not allocate memory for the adjacency list.\n");
        free(G);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < V; i++) {
        G->adjList[i] = NULL;
    }
    return G;
}

/**
 * @brief Inserts a directed edge v->w into the graph.
 * @param G The graph.
 * @param v The source vertex.
 * @param w The destination vertex.
 * @param value The value of the transaction.
 * @return 1 on success, 0 on failure.
 */
int insertEdge(Graph G, vertex v, vertex w, const mpz_t value) {
    if (v < 0 || (size_t)v >= G->vertexAmount || w < 0 || (size_t)w >= G->vertexAmount) return 0;

    Transaction *newNode = malloc(sizeof(Transaction));
    if (!newNode) {
        fprintf(stderr, "Error: Could not allocate memory for new transaction node.\n");
        return 0;
    }

    newNode->destination = w;
    mpz_init_set(newNode->transactionValue, value);
    newNode->next = G->adjList[v];
    G->adjList[v] = newNode;
    G->edgesAmount++;
    return 1;
}

/**
 * @brief Frees all memory associated with the graph.
 * @param G The graph to be freed.
 */
void freeGraph(Graph G) {
    if (!G) return;
    for (size_t i = 0; i < G->vertexAmount; i++) {
        Transaction *curr = G->adjList[i];
        while (curr) {
            Transaction *temp = curr;
            curr = curr->next;
            mpz_clear(temp->transactionValue);
            free(temp);
        }
    }
    free(G->adjList);
    free(G);
}

/**
 * @brief Displays the graph's adjacency list representation to stdout.
 * @param G The graph to display.
 */
void showGraph(Graph G) {
    printf("\n/--- GRAPH ADJACENCY LIST ---/\n");
    for (size_t v = 0; v < G->vertexAmount; v++) {
        printf("%zu: ", v);
        Transaction *curr = G->adjList[v];
        while (curr) {
            gmp_printf("%d (Value: %Zd) -> ", curr->destination, curr->transactionValue);
            curr = curr->next;
        }
        printf("NULL\n");
    }
    printf("/--------------------------/\n");
}

// --- CYCLE DETECTION (DFS) FUNCTIONS ---

/**
 * @brief Recursive helper for DFS to find cycles.
 * @param G The graph.
 * @param visited Array to keep track of visited vertices.
 * @param recStack Array for the recursion stack to detect cycles.
 * @param path Array to store the current traversal path.
 * @param valuesPath Array to store the values of edges in the current path.
 * @param depth Current depth in the DFS traversal.
 * @param v The current vertex being visited.
 * @param p File pointer to write cycle information.
 * @param log_func The logging function to use.
 * @param cycle_count Pointer to a counter for found cycles.
 */
static void recursiveDFS(Graph G, int *visited, int *recStack, int *path, mpz_t *valuesPath, int *depth, vertex v, FILE *p, log_function_t log_func, size_t *cycle_count) {
    visited[v] = 1;
    recStack[v] = 1;
    path[*depth] = v;
    (*depth)++;

    Transaction *curr = G->adjList[v];
    while (curr) {
        vertex w = curr->destination;
        mpz_set(valuesPath[*depth - 1], curr->transactionValue);

        if (!visited[w]) {
            log_func("(%d -> %d)\n", v, w);
            recursiveDFS(G, visited, recStack, path, valuesPath, depth, w, p, log_func, cycle_count);
        } else if (recStack[w]) { // Cycle detected
            (*cycle_count)++;
            log_func("Cycle #%zu: ", *cycle_count);
            fprintf(p, "Cycle #%zu: ", *cycle_count);

            mpz_t cycle_max_value;
            mpz_init(cycle_max_value); // Initializes to 0

            size_t start_index = 0;
            for (start_index = 0; (size_t)start_index < (size_t)*depth; start_index++) {
                if (path[start_index] == w) break;
            }
            
            // Find max value within the cycle path
            for (size_t i = start_index; (size_t)i < (size_t)*depth - 1; i++) {
                fprintf(p, "%d -> ", path[i]);
                log_func("%d -> ", path[i]);
                if (mpz_cmp(cycle_max_value, valuesPath[i]) < 0) {
                    mpz_set(cycle_max_value, valuesPath[i]);
                }
            }
            
            fprintf(p, "%d -> ", path[*depth - 1]);
            log_func("%d -> ", path[*depth - 1]);
            if (mpz_cmp(cycle_max_value, curr->transactionValue) < 0) {
                mpz_set(cycle_max_value, curr->transactionValue);
            }

            fprintf(p, "%d\n", w);
            log_func("%d\n", w);

            gmp_fprintf(p, "Max Flow: %Zd WEI\n", cycle_max_value);
            log_func("Max Flow in Cycle: ");
            if (log_func == log_verbose) {
                gmp_printf("%Zd\n", cycle_max_value);
            }
            mpz_clear(cycle_max_value);
        }
        curr = curr->next;
    }
    recStack[v] = 0; // Backtrack
    (*depth)--;
}

/**
 * @brief Main function to perform Depth First Search and find cycles.
 * The algorithm visits each vertex and edge once. Therefore, its time
 * complexity is linear in the number of vertices and edges.
 *
 * The complexity can be expressed as:
 * \f[
 *   O(V + E)
 * \f]
 * where \f$ V \f$ is the number of vertices and \f$ E \f$ is the number of edges.
 * @param G The graph to search.
 * @param filename The name of the file to write cycle information to.
 * @param logger The logging function to use.
 * @param total_cycles_found A pointer to a size_t to store the count of found cycles.
 */
void depthFirstSearch(Graph G, const char *const filename, log_function_t logger, size_t *total_cycles_found) {
    if (G->vertexAmount == 0) return;

    int *visited = calloc(G->vertexAmount, sizeof(int));
    int *recStack = calloc(G->vertexAmount, sizeof(int));
    int *path = malloc(G->vertexAmount * sizeof(int));
    mpz_t *valuesPath = malloc(G->vertexAmount * sizeof(mpz_t));
    int depth = 0;

    FILE *p = fopen(filename, "w");
    if (p == NULL) {
        perror("ERROR: creating/opening output file");
        // Free allocated memory before returning
        free(visited);
        free(recStack);
        free(path);
        free(valuesPath);
        return;
    }

    if (!visited || !recStack || !path || !valuesPath) {
        fprintf(stderr, "ERROR: bad alloc for DFS arrays\n");
        free(visited);
        free(recStack);
        free(path);
        free(valuesPath);
        fclose(p);
        return;
    }

    for (size_t i = 0; i < G->vertexAmount; i++) {
        mpz_init(valuesPath[i]);
    }

    *total_cycles_found = 0;
    for (size_t v = 0; v < G->vertexAmount; v++) {
        if (!visited[v]) {
            recursiveDFS(G, visited, recStack, path, valuesPath, &depth, v, p, logger, total_cycles_found);
        }
    }

    // Cleanup
    free(visited);
    free(recStack);
    free(path);
    for (size_t i = 0; i < G->vertexAmount; i++) {
        mpz_clear(valuesPath[i]);
    }
    free(valuesPath);
    fclose(p);
}


// --- LOGGING FUNCTIONS ---

/**
 * @brief A logging function that does nothing (silent mode).
 * @param format The format string.
 * @param ... Variable arguments for the format string.
 */
void log_silent(const char *format, ...) {
    (void)format; // Suppress unused parameter warning
}

/**
 * @brief A logging function that prints messages to stdout.
 * @param format The format string.
 * @param ... Variable arguments for the format string.
 */
void log_verbose(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

 /** @} */ 