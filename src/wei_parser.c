      
/**
 * @file wei_parser.c
 * @brief Implementation of the optimized Wei value parser.
 * @defgroup wei_parser Wei Value Parser
 * @{
 */  

#include "wei_parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct parse_wei_ctx_t
 * @brief The internal structure of the Wei parser context.
 *
 * Holds reusable GMP variables and a cache to accelerate parsing.
 */
struct parse_wei_ctx_t {
    /** @var mantissa_int The integer part of the number before applying the exponent. */
    mpz_t mantissa_int;
    /** @var power_of_10 A temporary variable to hold powers of 10 for calculations. */
    mpz_t power_of_10;
    /** @var remainder Used to check if a division results in a non-integer value. */
    mpz_t remainder;
    /** @var pow10_cache An array caching pre-calculated powers of 10. */
    mpz_t pow10_cache[POW10_CACHE_SIZE];
};

/**
 * @brief Allocates and initializes the context required for the parser.
 * @return A pointer to the allocated context, or NULL on allocation failure.
 */
parse_wei_ctx_t* parse_wei_ctx_create() {
    parse_wei_ctx_t *ctx = malloc(sizeof(parse_wei_ctx_t));
    if (!ctx) {
        return NULL; // Allocation failed
    }

    mpz_init(ctx->mantissa_int);
    mpz_init(ctx->power_of_10);
    mpz_init(ctx->remainder);

    // Pre-compute powers of 10 and store them in the cache
    mpz_init_set_ui(ctx->pow10_cache[0], 1);
    for (int i = 1; i < POW10_CACHE_SIZE; ++i) {
        mpz_init(ctx->pow10_cache[i]);
        mpz_mul_ui(ctx->pow10_cache[i], ctx->pow10_cache[i - 1], 10);
    }
    return ctx;
}

/**
 * @brief Frees all memory associated with the parser context.
 * @param ctx The context to be freed.
 */
void parse_wei_ctx_free(parse_wei_ctx_t *ctx) {
    if (!ctx) return;

    mpz_clear(ctx->mantissa_int);
    mpz_clear(ctx->power_of_10);
    mpz_clear(ctx->remainder);
    for (int i = 0; i < POW10_CACHE_SIZE; ++i) {
        mpz_clear(ctx->pow10_cache[i]);
    }

    free(ctx);
}

/**
 * @brief Converts a numeric string to a Wei integer value.
 * @param ctx The parser context.
 * @param s The input string.
 * @param out A pointer to the mpz_t variable that will receive the result.
 * @return 0 on success, or a negative error code.
 */
int parse_wei_optimized(parse_wei_ctx_t *ctx, const char *s, mpz_t out) {
    // Reset mantissa to zero for the new operation
    mpz_set_ui(ctx->mantissa_int, 0);

    const char *e_ptr = strpbrk(s, "eE");
    const char *mantissa_end = e_ptr ? e_ptr : s + strlen(s);

    long decimal_places = 0;
    int dot_found = 0;
    int has_digits = 0;

    // Build the mantissa mathematically
    for (const char *p = s; p < mantissa_end; ++p) {
        if (isdigit((unsigned char)*p)) {
            has_digits = 1;
            mpz_mul_ui(ctx->mantissa_int, ctx->mantissa_int, 10);
            mpz_add_ui(ctx->mantissa_int, ctx->mantissa_int, *p - '0');
            if (dot_found) {
                decimal_places++;
            }
        } else if (*p == '.' && !dot_found) {
            dot_found = 1;
        } else if ((*p == '+' || *p == '-') && p == s) {
            // Sign at the beginning is allowed but ignored as mpz handles it.
        } else {
            return -1; // Invalid character
        }
    }

    if (!has_digits) return -1; // No digits found

    long exponent_val = 0;
    if (e_ptr) {
        exponent_val = strtol(e_ptr + 1, NULL, 10);
    }

    long final_power_of_10 = exponent_val - decimal_places;

    if (final_power_of_10 > 0) {
        // Multiply by 10^N
        if (final_power_of_10 < POW10_CACHE_SIZE) {
            mpz_set(ctx->power_of_10, ctx->pow10_cache[final_power_of_10]);
        } else {
            mpz_ui_pow_ui(ctx->power_of_10, 10, final_power_of_10);
        }
        mpz_mul(out, ctx->mantissa_int, ctx->power_of_10);

    } else if (final_power_of_10 < 0) {
        // Divide by 10^N
        unsigned long divisor_exp = -final_power_of_10;
        if (divisor_exp < POW10_CACHE_SIZE) {
            mpz_set(ctx->power_of_10, ctx->pow10_cache[divisor_exp]);
        } else {
            mpz_ui_pow_ui(ctx->power_of_10, 10, divisor_exp);
        }
        
        // Divide and check if there is a remainder
        mpz_tdiv_qr(out, ctx->remainder, ctx->mantissa_int, ctx->power_of_10);
        if (mpz_cmp_ui(ctx->remainder, 0) != 0) {
            return -2; // Result is not a whole number
        }
    } else {
        // Exponent is zero, just copy the mantissa
        mpz_set(out, ctx->mantissa_int);
    }

    return 0; // Success
}

 /** @} */ 