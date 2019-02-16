#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tests/test_base.h"
#include "tests/test_lambert.h"
#include "tests/test_phong.h"
#include "tests/test_blinn.h"
#include "tests/test_unlit.h"
#include "tests/test_metalness.h"

typedef void testfunc_t(int argc, char *argv[]);
typedef struct {const char *testname; testfunc_t *testfunc;} testcase_t;

static testcase_t g_testcases[] = {
    {"lambert", test_lambert},
    {"phong", test_phong},
    {"blinn", test_blinn},
    {"unlit", test_unlit},
    {"metalness", test_metalness},
};

int main(int argc, char *argv[]) {
    int num_testcases = ARRAY_LENGTH(g_testcases);
    const char *testname = NULL;
    testfunc_t *testfunc = NULL;
    int i;

    srand((unsigned int)time(NULL));

    if (argc > 1) {
        testname = argv[1];
        for (i = 0; i < num_testcases; i++) {
            if (strcmp(testname, g_testcases[i].testname) == 0) {
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
    }

    return 0;
}
