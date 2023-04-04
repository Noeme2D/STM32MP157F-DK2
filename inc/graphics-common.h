#pragma once

#include <EGL/egl.h>
#include <gbm.h>
#include <xf86drmMode.h>

typedef struct {
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
} egl_t;

typedef struct {
    struct gbm_device *dev;
    struct gbm_surface *surface;
} gbm_t;

typedef struct {
    int fd;
    drmModeModeInfo *mode;
    uint32_t crtc_id;
    uint32_t connector_id;
} drm_t;

// 0 on success, -1 on failure
int init_drm(drm_t *drm);
int init_gbm(gbm_t *gbm, drm_t *drm);
int init_gl(egl_t *egl, gbm_t *gbm);

void free_graphics(drm_t *drm, gbm_t *gbm);