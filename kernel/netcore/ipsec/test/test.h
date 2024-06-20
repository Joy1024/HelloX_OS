/* All structural testing routines are defined in this file. */

#include "structural_test.h"

/* MD5 testing. */
void md5_test(test_result *global_results);

/**
 * Main test function for the DES/3DES CBC tests.
 * It does nothing but calling the subtests one after the other.
 */
void des_test(test_result *global_results);

/**
 * Main test function for the SHA1 tests.
 * It does nothing but calling the subtests one after the other.
 */
void sha1_test(test_result *global_results);

/**
 * Test function for all the log functions
 * (Note: some of these tests are commented out by default to make the log output more uniform)
 */
void util_debug_test(test_result *global_results);

/**
 * Main test function for the SA tests.
 * It does nothing but calling the subtests one after the other.
 */
void sa_test(test_result *global_results);

/**
 * Main test function for the ESP tests.
 * It does nothing but calling the subtests one after the other.
 */
void esp_test(test_result *global_results);

/**
 * Main test function for the AH tests.
 * It does nothing but calling the subtests one after the other.
 */
void ah_test(test_result *global_results);
