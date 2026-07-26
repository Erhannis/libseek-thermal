#include "seek.h"
#include "SeekCamCWrapper.h"


int main(int argc, char** argv)
{
    printf("1\n");
    struct FakeMatU16 frame;
    frame.rows = 0;
    frame.cols = 0;
    frame.channels = 0;
    frame.data = nullptr;
    printf("2\n");
    SeekCam* cam = newSeekCam(true, "", "/dev/usb_thermal_left_front_a");
    printf("3\n");

    if (!SeekCam_open(cam)) {
      printf("Failed to open cam\n");
    } else {
      printf("Opened cam\n");
    }

    // if (!cam->open()) {
    //     std::cout << "failed to open " << camtype << " cam" << std::endl;
    //     return -1;
    // }

    // for (i = 0; i < warmup; i++) {
    //     if (!cam->grab()) {
    //         std::cout << "no more LWIR img" << std::endl;
    //         return -1;
    //     }
    //     cam->retrieve(frame_u16);
    //     cv::waitKey(10);
    // }

    // std::cout << "warmup complete" << std::endl;

    // // Aquire frames
    // for (i = 0; i < smoothing; i++) {
    //     if (!cam->grab()) {
    //         std::cout << "no more LWIR img" << std::endl;
    //         return -1;
    //     }

    //     cam->retrieve(frame_u16);
    //     frame_u16.convertTo(frame, CV_32FC1);

    //     if (avg_frame.rows == 0) {
    //         frame.copyTo(avg_frame);
    //     } else {
    //         avg_frame += frame;
    //     }
    //     cv::waitKey(10);
    // }

    // // Average the collected frames
    // avg_frame /= smoothing;
    // avg_frame.convertTo(frame_u16, CV_16UC1);

    // Mat frame_g8, outframe; // Transient Mat containers for processing

    // normalize(frame_u16, frame_u16, 0, 65535, NORM_MINMAX);

    // // Convert seek CV_16UC1 to CV_8UC1
    // frame_u16.convertTo(frame_g8, CV_8UC1, 1.0 / 256.0);

    // // Apply colormap: https://docs.opencv.org/master/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65
    // if (colormap != -1) {
    //     applyColorMap(frame_g8, outframe, colormap);
    // }
    // else {
    //     cv::cvtColor(frame_g8, outframe, cv::COLOR_GRAY2BGR);
    // }

    // // Rotate image
    // if (rotate == 90) {
    //     transpose(outframe, outframe);
    //     flip(outframe, outframe, 1);
    // }
    // else if (rotate == 180) {
    //     flip(outframe, outframe, -1);
    // }
    // else if (rotate == 270) {
    //     transpose(outframe, outframe);
    //     flip(outframe, outframe, 0);
    // }

    // cv::imwrite(outfile, outframe);
    return 0;
}
