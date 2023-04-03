#include "graphics-common.h"

#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    drm_t drm;
    gbm_t gbm;
    egl_t egl;

    if (init_drm(&drm) < 0) return EXIT_FAILURE;

    if (init_gbm(&gbm, &drm) < 0) {
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    if (init_gl(&egl, &gbm) < 0) {
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    char *extension_list = (char *)glGetString(GL_EXTENSIONS);
    if (extension_list) {
        printf("%s\n", extension_list);
    } else {
        printf("glGetString() failed.\n");
    }

    free_graphics(&drm, &gbm);
    return EXIT_SUCCESS;
}