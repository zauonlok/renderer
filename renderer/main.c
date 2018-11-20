#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tests/test_phong.h"

typedef void testfunc_t(int argc, char *argv[]);
typedef struct {const char *testname; testfunc_t *testfunc;} testcase_t;

static testcase_t testcases[] = {
    {"test_phong", test_phong},
};

int main(int argc, char *argv[]) {
    int num_testcases = sizeof(testcases) / sizeof(testcase_t);

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
        testcase_t testcase;
        unsigned int seed;
        int index;
        seed = (unsigned int)time(NULL);
        srand(seed);
        index = rand() % num_testcases;
        testcase = testcases[index];
        printf("running test: %s\n", testcase.testname);
        testcase.testfunc(argc, argv);
    }

    return 0;
}
