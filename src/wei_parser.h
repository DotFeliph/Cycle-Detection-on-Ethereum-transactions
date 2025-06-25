/**
 * @file wei_parser.h
 * @brief Defines the interface for an optimized Wei value parser.
 *
 * This parser is designed to efficiently convert string representations of numbers
 * (including decimal and scientific notation) into large integer values (mpz_t),
 * which represent the smallest unit of Ether (Wei). It uses a pre-allocated
 * context to improve performance by reusing GMP variables.
 */

#ifndef E22E35BD_50BC_4CBE_9F63_17EA72FB8D1B
#define E22E35BD_50BC_4CBE_9F63_17EA72FB8D1B

#include <gmp.h>
#include <stdio.h>

/**
 * @def POW10_CACHE_SIZE
 * @brief The size of the pre-calculated powers-of-10 cache.
 *
 * This cache stores frequently used powers of 10 to avoid recalculating them,
 * speeding up conversions.
 */
#define POW10_CACHE_SIZE 60

/**
 * @struct parse_wei_ctx_t
 * @brief An opaque type for the Wei parser context.
 *
 * This context holds pre-allocated GMP variables and caches to make parsing
 * operations more efficient when called multiple times.
 */
typedef struct parse_wei_ctx_t parse_wei_ctx_t;

/**
 * @brief Allocates and initializes the context required for the parser.
 *
 * Creates the context, initializes GMP variables, and pre-computes a cache
 * of powers of 10.
 *
 * @return A pointer to the allocated context, or NULL on allocation failure.
 */
parse_wei_ctx_t* parse_wei_ctx_create();

/**
 * @brief Frees all memory associated with the parser context.
 *
 * @param ctx The context to be freed.
 */
void parse_wei_ctx_free(parse_wei_ctx_t *ctx);

/**
 * @brief Converts a numeric string to a Wei integer value using an optimized method.
 *
 * Handles integers, decimals, and scientific notation (e/E).
 * The result is an integer; any fractional part will result in an error.
 *
 * @param ctx The parser context.
 * @param s The input string (e.g., "1.23", "54e18").
 * @param out A pointer to the mpz_t variable that will receive the result.
 * @return 0 on success.
 * @return -1 for an invalid character in the string.
 * @return -2 if the conversion results in a non-integer value (e.g., "1.23e17").
 */
int parse_wei_optimized(parse_wei_ctx_t *ctx, const char *s, mpz_t out);

#endif /* E22E35BD_50BC_4CBE_9F63_17EA72FB8D1B */