#include <fcntl.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PER_PIXEL 2

char *buffer;

// prints the choices that the user can capture in.
int print_caps(int fd) {
    struct v4l2_capability caps = {};
    if (ioctl(fd, VIDIOC_QUERYCAP, &caps)) {
        perror("Querying Capabilities");
        return -1;
    }

    printf("Driver Caps:\n"
           "  Driver: \"%s\"\n"
           "  Card: \"%s\"\n"
           "  Bus: \"%s\"\n"
           "  Version: %d.%d\n"
           "  Capabilities: %08x\n",
           caps.driver, caps.card, caps.bus_info, (caps.version >> 16) & 0xff, (caps.version >> 24) & 0xff,
           caps.capabilities);

    struct v4l2_cropcap cropcap = {0};
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        perror("Querying Cropping Capabilities");
        return -1;
    }

    printf("Camera Cropping:\n"
           "  Bounds: %dx%d+%d+%d\n"
           "  Default: %dx%d+%d+%d\n"
           "  Aspect: %d/%d\n",
           cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top, cropcap.defrect.width,
           cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top, cropcap.pixelaspect.numerator,
           cropcap.pixelaspect.denominator);

    struct v4l2_fmtdesc fmtdesc = {0};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    char fourcc[5] = {0};
    printf("Supported Pixel Formats:\n");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        printf("  %d: %s\n", fmtdesc.index, fmtdesc.description);
        fmtdesc.index++;
    }

    return 0;
}

int set_pix_fmt(int fd, int pix_fmt) {
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = pix_fmt;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt)) {
        perror("Setting Pixel Format");
        return -1;
    }

    return 0;
}

int set_focus(int fd, int focus) {
    struct v4l2_ext_controls ctrls = {0};
    struct v4l2_ext_control ctrl = {0};

    ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
    ctrls.count = 1;
    ctrls.controls = &ctrl;
    ctrl.id = V4L2_CID_FOCUS_AUTO;
    ctrl.value = 0;
    if (ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
        perror("Shutting Down Auto-Focus");
        return -1;
    }

    ctrls.which = V4L2_CTRL_WHICH_CUR_VAL;
    ctrls.count = 1;
    ctrls.controls = &ctrl;
    ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;
    ctrl.value = focus;
    if (ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
        perror("Setting Focus");
        return -1;
    }

    return 0;
}

// Requests and queries the buffer.
int init_mmap(int fd) {
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req)) {
        perror("Requesting Buffer");
        return -1;
    }

    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
        perror("Querying Buffer");
        return -1;
    }

    buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    printf("Buffer:\n  Length: %d\n  Address: %p\n", buf.length, buffer);

    return 0;
}

int capture_image(int fd) {
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QBUF, &buf)) {
        perror("Query Buffer");
        return -1;
    }

    if (ioctl(fd, VIDIOC_STREAMON, &buf.type)) {
        perror("Start Capture");
        return -1;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, 2000) == -1) {
        perror("Waiting for Frame");
        return -1;
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buf)) {
        perror("Retrieving Frame");
        return 1;
    }
    printf("Saving Image.\n");
    FILE *fp;
    fp = fopen("out.yuyv", "wb");
    fwrite(buffer, WIDTH * HEIGHT * BYTES_PER_PIXEL, 1, fp);
    fclose(fp);

    return 0;
}

int main(int argc, char *argv[]) {
    int fd;

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Opening video device");
        return 1;
    }
    if (print_caps(fd)) return -1;
    if (set_pix_fmt(fd, V4L2_PIX_FMT_YUYV)) return -1;
    if (set_focus(fd, 200)) return -1;
    if (init_mmap(fd)) return -1;
    if (capture_image(fd)) return -1;

    close(fd);
    return 0;
}
