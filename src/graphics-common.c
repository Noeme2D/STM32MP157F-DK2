#include "graphics-common.h"

#include "drm_fourcc.h"
#include <EGL/eglext.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int init_drm(drm_t *drm) {
    drm->fd = open("/dev/dri/card0", O_RDWR);
    if (drm->fd < 0) {
        printf("/dev/dri/card0 open failure.\n");
        return -1;
    }
    /*
    ~# modetest -M stm
    Connectors:
    id      encoder status          name            size (mm)       modes   encoders
    32      0       disconnected    HDMI-A-1        0x0             0       31
        modes:
        index name refresh (Hz) hdisp hss hse htot vdisp vss vse vtot
        #0 480x800 50.00 480 578 610 708 800 815 825 839 29700 flags: nhsync, nvsync; type: preferred, driver
        #1 480x800 60.00 480 550 582 654 800 815 825 841 33000 flags: nhsync, nvsync; type: driver
    34      0       connected       DSI-1           52x86           2       33
        modes:
        index name refresh (Hz) hdisp hss hse htot vdisp vss vse vtot
        #0 480x800 50.00 480 578 610 708 800 815 825 839 29700 flags: nhsync, nvsync; type: preferred, driver
        #1 480x800 60.00 480 550 582 654 800 815 825 841 33000 flags: nhsync, nvsync; type: driver
    CRTCs:
    id      fb      pos     size
    41      0       (0,0)   (0x0)
    */
    drm->connector_id = 34;
    drm->crtc_id = 41;
    drmModeConnector *connector = drmModeGetConnector(drm->fd, drm->connector_id);
    drmModeEncoder *encoder = drmModeGetEncoder(drm->fd, drm->crtc_id);

    bool mode_set = false;
    for (int i = 0; i < connector->count_modes; i++) {
        drmModeModeInfo *mode = connector->modes + i;
        if (strcmp(mode->name, "480x800") == 0 && mode->vrefresh == 60) {
            mode_set = true;
            drm->mode = mode;
            break;
        }
    }
    if (!mode_set) {
        printf("Malformed mode string.\n");
        close(drm->fd);
        return -1;
    }

    return 0;
}

int init_gbm(gbm_t *gbm, drm_t *drm) {
    gbm->dev = gbm_create_device(drm->fd);
    if (!gbm->dev) {
        printf("gbm_create_device() failed.\n");
        return -1;
    }

    // DRM_FORMAT_XRGB8888:
    // https://github.com/STMicroelectronics/st-openstlinux-application/blob/master/weston-cube/src/cube-common.c:42
    gbm->surface = gbm_surface_create(gbm->dev, drm->mode->hdisplay, drm->mode->vdisplay, DRM_FORMAT_XRGB8888,
                                      GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!gbm->surface) {
        printf("gbm_surface_create() failed.\n");
        gbm_device_destroy(gbm->dev);
        return -1;
    }

    return 0;
}

static EGLint const egl_attrib[] = {EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, EGL_NONE};
static EGLint const context_attrib[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE};

int init_gl(egl_t *egl, gbm_t *gbm) {
    egl->display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm->dev, NULL);
    if (egl->display == EGL_NO_DISPLAY) {
        printf("eglGetDisplay() failed.\n");
        return -1;
    }

    if (eglInitialize(egl->display, NULL, NULL) != EGL_TRUE) {
        printf("eglInitialize() failed.\n");
        return -1;
    }

    // Returns a list of configs that match the given attribute requirements
    // The number of matches is written into num_config
    // The list is sorted by some criteria, from best to worst
    EGLint num_config;
    if (eglChooseConfig(egl->display, egl_attrib, &(egl->config), 1, &num_config) != EGL_TRUE) {
        printf("eglChooseConfig() failed.\n");
        return -1;
    }

    // set context API to create by eglCreateContext()
    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        printf("eglBindAPI() failed.\n");
        return -1;
    }
    // EGL_NO_CONTEXT means the context is not created by sharing with others
    EGLContext context = eglCreateContext(egl->display, egl->config, EGL_NO_CONTEXT, context_attrib);
    if (context == EGL_NO_CONTEXT) {
        printf("eglCreateContext() failed.\n");
        return -1;
    }

    // enable the OpenGL ES 2.0 context just set up
    if (eglMakeCurrent(egl->display, EGL_NO_CONTEXT, EGL_NO_CONTEXT, context) != EGL_TRUE) {
        printf("eglMakeCurrent() failed with error code %x\n", eglGetError());
        return -1;
    }

    return 0;
}

void free_graphics(drm_t *drm, gbm_t *gbm) {
    if (gbm->dev) gbm_device_destroy(gbm->dev);
    close(drm->fd);
}