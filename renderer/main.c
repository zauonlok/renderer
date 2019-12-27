#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core/api.h"
#include "shaders/cache_helper.h"
#include "tests/test_blinn.h"
#include "tests/test_pbr.h"

typedef void testfunc_t(int argc, char *argv[]);
typedef struct {const char *testname; testfunc_t *testfunc;} testcase_t;

static testcase_t g_testcases[] = {
    {"blinn", test_blinn},
    {"pbr", test_pbr},
};

int main(int argc, char *argv[]) {
    int num_testcases = ARRAY_SIZE(g_testcases);
    const char *testname = NULL;
    testfunc_t *testfunc = NULL;
    int i;

    srand((unsigned int)time(NULL));
    platform_initialize();

    if (argc > 1) {
        testname = argv[1];
        for (i = 0; i < num_testcases; i++) {
            if (strcmp(g_testcases[i].testname, testname) == 0) {
                testfunc = g_testcases[i].testfunc;
                break;
            }
        }
    } else {
        i = rand() % num_testcases;
        testname = g_testcases[i].testname;
        testfunc = g_testcases[i].testfunc;
    }

    if (testfunc) {
        printf("test: %s\n", testname);
        testfunc(argc, argv);
    } else {
        printf("test not found: %s\n", testname);
        printf("available tests: ");
        for (i = 0; i < num_testcases; i++) {
            if (i != num_testcases - 1) {
                printf("%s, ", g_testcases[i].testname);
            } else {
                printf("%s\n", g_testcases[i].testname);
            }
        }
    }

    platform_terminate();
    cache_cleanup();

    return 0;
}
