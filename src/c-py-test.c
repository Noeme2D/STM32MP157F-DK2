#include "c-py-test.h"

#include "cpytest.h"

#include <Python.h>
#include <stdlib.h>

int main() {
    int err = PyImport_AppendInittab("cpytest", PyInit_cpytest);
    if (err) {
        printf("PyImport failed.\n");
        return -1;
    }
    Py_Initialize();
    PyImport_ImportModule("cpytest");

    char *dummy_image = malloc(20 * 10 * 3);
    test_t test;

    run_test(dummy_image, &test);

    printf("%f\n", test.field[0]);

    Py_Finalize();
    free(dummy_image);
    return 0;
}