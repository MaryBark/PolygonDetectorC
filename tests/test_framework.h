#ifndef TEST_FRAMEWORK_H_
#define TEST_FRAMEWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "  FAILED: %s (line %d)\n", msg, __LINE__); \
            return 1; \
        } \
    } while(0)

#define TEST_ASSERT_FLOAT_EQUAL(a, b, epsilon, msg) \
    do { \
        if (fabs((a) - (b)) > (epsilon)) { \
            fprintf(stderr, "  FAILED: %s (line %d): %f != %f\n", msg, __LINE__, (double)(a), (double)(b)); \
            return 1; \
        } \
    } while(0)

#define TEST_ASSERT_STRING_EQUAL(a, b, msg) \
    do { \
        if (strcmp((a), (b)) != 0) { \
            fprintf(stderr, "  FAILED: %s (line %d): '%s' != '%s'\n", msg, __LINE__, (a), (b)); \
            return 1; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s...\n", #test_func); \
        if (test_func() == 0) { \
            printf("  ✓ PASSED\n\n"); \
            passed++; \
        } else { \
            printf("  ✗ FAILED\n\n"); \
            failed++; \
        } \
    } while(0)

#define TEST_SUMMARY() \
    printf("\n========================================\n"); \
    printf("Test Summary: %d passed, %d failed\n", passed, failed); \
    printf("========================================\n"); \
    return (failed > 0) ? 1 : 0

#endif  // TEST_FRAMEWORK_H_
