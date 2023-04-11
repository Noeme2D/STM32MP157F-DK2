import sys
import numpy as np
import cv2

WIDTH = 640
HEIGHT = 480
BYTES_PER_PIX = 2

with open('out.yuyv', 'rb') as fd:
    f = np.fromfile(fd, dtype=np.uint8,count=WIDTH*HEIGHT*BYTES_PER_PIX)
    yuyv = f.reshape((HEIGHT, WIDTH, BYTES_PER_PIX))
    rgb = cv2.cvtColor(yuyv, cv2.COLOR_YUV2RGB_YUYV)
    
    cv2.imshow('YUYV (YUV 4:2:2) Image Viewer', rgb[:, :, ::-1])
    cv2.waitKey()
    cv2.destroyAllWindows()
