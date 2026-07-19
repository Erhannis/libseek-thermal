#ifndef SEEK_CAM_C_WRAPPER_H
#define SEEK_CAM_C_WRAPPER_H

using namespace LibSeek;

#ifdef __cplusplus
extern "C" {
#endif

struct FakeMatU8 {
  int width;
  int height;
  char* data;
};

struct FakeMatU16 {
  int width;
  int height;
  short* data;
};

typedef struct SeekCam SeekCam;

/*
  *  pro:
  *      Is pro camera, vs non-pro camera
  *
  *  ffc_filename:
  *      Filename for additional flat field calibration and corner
  *      gradient elimination. If provided and found, the image will
  *      be subtracted from each retrieved frame. If not, no additional
  *      flat field calibration will be applied
  *  
  *  dev_filename:
  *      Filename for camera device.  Some restrictions may apply,
  *      such as "must be in /dev".
  */
SeekCam* newSeekCam(bool pro, const char* ffc_filename, const char* dev_filename);


/*
  *  Initialize the camera
  *  Returns true on success
  */
bool SeekCam_open(SeekCam* cam);

/*
  *  Returns true when camera is initialized
  */
bool SeekCam_isOpened(SeekCam* cam);

/*
  *  Close the camera
  */
void SeekCam_close(SeekCam* cam);

/*
  *  Grab a frame
  *  Returns true on success
  */
bool SeekCam_grab(SeekCam* cam);

/*
  *  Retrieve the last grabbed 14-bit frame
  *  Returns true on success
  */
void SeekCam_retrieve(SeekCam* cam, FakeMatU16* dst);

/*
  *  Convert a 14-bit thermal measurement to an
  *  enhanced 8-bit greyscale image for visual inspection
  */
void SeekCam_convertToGreyScale(SeekCam* cam, FakeMatU16* src, FakeMatU8* dst);

/*
  *  Read grabs and retrieves a frame
  *  Returns true on success
  */
bool SeekCam_read(SeekCam* cam, FakeMatU16* dst);

/*
  *  Get the frame counter value
  */
int SeekCam_frame_counter(SeekCam* cam);

void deleteSeekCam(SeekCam* cam);


#ifdef __cplusplus
}
#endif
#endif /* SEEK_CAM_C_WRAPPER_H */
