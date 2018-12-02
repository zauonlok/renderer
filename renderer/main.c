#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tests/test_base.h"
#include "tests/test_lambert.h"
#include "tests/test_blinn.h"
#include "tests/test_phong.h"

typedef void testfunc_t(int argc, char *argv[]);
typedef struct {const char *testname; testfunc_t *testfunc;} testcase_t;

static testcase_t testcases[] = {
    {"test_lambert", test_lambert},
    {"test_phong", test_phong},
    {"test_blinn", test_blinn},
};

int main(int argc, char *argv[]) {
    int num_testcases = ARRAY_LENGTH(testcases);
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);

    if (argc > 1) {
        const char *testname = argv[1];
        testfunc_t *testfunc = NULL;
        int i;
        for (i = 0; i < num_testcases; i++) {
            if (strcmp(testname, testcases[i].testname) == 0) {
                testfunc = testcases[i].testfunc;
                break;
            }
        }
        if (testfunc) {
            printf("running test: %s\n", testname);
            testfunc(argc, argv);
        } else {
            printf("test not found: %s\n", testname);
        }
    } else {
        int index = rand() % num_testcases;
        testcase_t testcase = testcases[index];
        printf("running test: %s\n", testcase.testname);
        testcase.testfunc(argc, argv);
    }

    return 0;
}
