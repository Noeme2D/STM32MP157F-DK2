#include "graphics-common.h"

#include <GLES2/gl2.h>
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drmMode.h>

int main() {
    drm_t drm;
    gbm_t gbm;
    egl_t egl;
    // storing return codes
    int ret;

    if (init_drm(&drm) < 0) return EXIT_FAILURE;

    if (init_gbm(&gbm, &drm) < 0) {
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    if (init_gl(&egl, &gbm) < 0) {
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    // clear to blue
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    eglSwapBuffers(egl.display, egl.surface);

    // gbm_surface_lock_front_buffer() must be called exactly ONCE and AFTER eglSwapBuffers()
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm.surface);
    if (bo == NULL) {
        printf("gbm_surface_lock_front_buffer() failed.\n");
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    uint32_t fb_id;
    // 24: 8 8 8 for RGB
    ret = drmModeAddFB(drm.fd, drm.mode->hdisplay, drm.mode->vdisplay, 24, gbm_bo_get_bpp(bo), gbm_bo_get_stride(bo),
                       gbm_bo_get_handle(bo).u32, &fb_id);
    if (ret) {
        printf("drmModeAddFB() failed.\n");
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    // where cloned display can be set
    ret = drmModeSetCrtc(drm.fd, drm.crtc_id, fb_id, 0, 0, &drm.connector_id, 1, drm.mode);
    if (ret) {
        printf("drmModeSetCrtc() failed.\n");
        gbm_surface_release_buffer(gbm.surface, bo);
        free_graphics(&drm, &gbm);
        return EXIT_FAILURE;
    }

    sleep(5);

    gbm_surface_release_buffer(gbm.surface, bo);
    free_graphics(&drm, &gbm);
    return EXIT_SUCCESS;
}