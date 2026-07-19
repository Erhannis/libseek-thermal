#include "SeekCam.h"
#include "SeekThermal.h"
#include "SeekThermalPro.h"
#include "SeekCamCWrapper.h"

using namespace LibSeek;

extern "C" {
  SeekCam* newSeekCam(bool pro, const char* ffc_filename, const char* dev_filename) {
    if (pro) {
      return new LibSeek::SeekThermalPro(ffc_filename, dev_filename);
    } else {
      return new LibSeek::SeekThermal(ffc_filename, dev_filename);
    }
  }

  bool SeekCam_open(SeekCam* cam) {
    return cam->open();
  }

  bool SeekCam_isOpened(SeekCam* cam) {
    return cam->isOpened();
  }

  void SeekCam_close(SeekCam* cam) {
    return cam->close();
  }

  bool SeekCam_grab(SeekCam* cam) {
    return cam->grab();
  }

  void SeekCam_retrieve(SeekCam* cam, FakeMatU16* dst) {
    cv::Mat frame_u16; //LEAK Avoid creating a new whatsit
    cam->retrieve(frame_u16);
    dst->cols = frame_u16.cols;
    dst->rows = frame_u16.rows;
    dst->channels = frame_u16.channels();
    //DUMMY ...This is probably results in use-after-free, doesn't it
    dst->data = (short*)(void*) (frame_u16.isContinuous() ? frame_u16.data : frame_u16.clone().data);
    //uint length = frame_u16.total()*frame_u16.channels();
  }

  void SeekCam_convertToGreyScale(SeekCam* cam, FakeMatU16* src, FakeMatU8* dst) {
    cv::Mat frame_u16; //LEAK Avoid creating a new whatsit
    cv::Mat frame_u8(src->rows, src->cols, CV_16U, src->data);
    cam->convertToGreyScale(frame_u8, frame_u16);
    dst->cols = frame_u16.cols;
    dst->rows = frame_u16.rows;
    dst->channels = frame_u16.channels();
    //DUMMY ...This is probably results in use-after-free, doesn't it
    dst->data = (char*)(void*) (frame_u16.isContinuous() ? frame_u16.data : frame_u16.clone().data);
  }

  bool SeekCam_read(SeekCam* cam, FakeMatU16* dst) {
    // Copied from read
    if (!SeekCam_grab(cam))
        return false;

    SeekCam_retrieve(cam, dst);

    return true;
  }

  int SeekCam_frame_counter(SeekCam* cam) {
    return cam->frame_counter();
  }

  void deleteSeekCam(SeekCam* cam) {
    //DUMMY This leaks memory, but also, there's some shenanigans going on with protected destructors and virtual interfaces

    cam->close();
    // delete cam;
  }
}