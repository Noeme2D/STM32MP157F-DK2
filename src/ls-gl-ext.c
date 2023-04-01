#include "drm_fourcc.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

static struct {
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
} egl;

static struct {
    struct gbm_device *dev;
    struct gbm_surface *surface;
} gbm;

static struct {
    int fd;
    drmModeModeInfo *mode;
    uint32_t crtc_id;
    uint32_t connector_id;
} drm;

static EGLint const attribute_list[] = {EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, EGL_NONE};
static EGLint const context_attib[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE};

void fail(const char *msg) {
    printf("%s\n", msg);
    close(drm.fd);
    exit(EXIT_FAILURE);
}

int main() {
    drm.fd = open("/dev/dri/card0", O_RDWR);
    if (drm.fd < 0) {
        printf("/dev/dri/card0 open failure.\n");
        return EXIT_FAILURE;
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
    drm.connector_id = 34;
    drm.crtc_id = 41;
    drmModeConnector *connector = drmModeGetConnector(drm.fd, drm.connector_id);
    drmModeEncoder *encoder = drmModeGetEncoder(drm.fd, drm.crtc_id);

    bool mode_set = false;
    for (int i = 0; i < connector->count_modes; i++) {
        drmModeModeInfo *mode = connector->modes + i;
        if (strcmp(mode->name, "480x800") == 0 && mode->vrefresh == 60) {
            mode_set = true;
            drm.mode = mode;
            break;
        }
    }
    if (!mode_set) {
        fail("Malformed mode string.");
    }

    gbm.dev = gbm_create_device(drm.fd);
    // DRM_FORMAT_XRGB8888:
    // https://github.com/STMicroelectronics/st-openstlinux-application/blob/master/weston-cube/src/cube-common.c:42
    gbm.surface = gbm_surface_create(gbm.dev, drm.mode->hdisplay, drm.mode->vdisplay, DRM_FORMAT_XRGB8888,
                                     GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!gbm.surface) {
        fail("gbm_surface_create() failed.");
    }

    egl.display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm.dev, NULL);
    if (egl.display == EGL_NO_DISPLAY) {
        printf("eglGetDisplay() failed.\n");
        return EXIT_FAILURE;
    }

    if (eglInitialize(egl.display, NULL, NULL) != EGL_TRUE) {
        printf("eglInitialize() failed.\n");
        return EXIT_FAILURE;
    }

    // Returns a list of configs that match the given attribute requirements
    // The number of matches is written into num_config
    // The list is sorted by some criteria, from best to worst
    EGLint num_config;
    if (eglChooseConfig(egl.display, attribute_list, &(egl.config), 1, &num_config) != EGL_TRUE) {
        printf("eglChooseConfig() failed.\n");
        return EXIT_FAILURE;
    }

    // set context API to create by eglCreateContext()
    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        printf("eglBindAPI() failed.\n");
        return EXIT_FAILURE;
    }
    // EGL_NO_CONTEXT means the context is not created by sharing with others
    EGLContext context = eglCreateContext(egl.display, egl.config, EGL_NO_CONTEXT, context_attib);
    if (context == EGL_NO_CONTEXT) {
        printf("eglCreateContext() failed.\n");
        return EXIT_FAILURE;
    }

    // enable the OpenGL ES 2.0 context just set up
    if (eglMakeCurrent(egl.display, EGL_NO_CONTEXT, EGL_NO_CONTEXT, context) != EGL_TRUE) {
        printf("eglMakeCurrent() failed with error code %x\n", eglGetError());
        return EXIT_FAILURE;
    }

    char *extension_list = (char *)glGetString(GL_EXTENSIONS);
    if (extension_list) {
        printf("%s\n", extension_list);
    } else {
        printf("glGetString() failed.\n");
    }

    close(drm.fd);
    return EXIT_SUCCESS;
}