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
    
  }

  void SeekCam_convertToGreyScale(SeekCam* cam, FakeMatU16* src, FakeMatU8* dst) {

  }

  bool SeekCam_read(SeekCam* cam, FakeMatU16* dst) {

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