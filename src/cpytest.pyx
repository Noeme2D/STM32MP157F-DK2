import numpy as np
cimport numpy as np

import cv2
import onnxruntime

DEF WIDTH = 20
DEF HEIGHT = 10

cdef extern from "c-py-test.h":
    ctypedef struct test_t:
        float field[2]

cdef public void run_test(char *c_image, test_t *test_out):
    cdef char [:, :, :] image_view = <char [:WIDTH, :HEIGHT, :3]> c_image
    frame = np.ndarray((WIDTH, HEIGHT, 3), buffer=image_view, order='c', dtype=np.uint8)
    print(frame)
    test_out.field[0] = 5.0