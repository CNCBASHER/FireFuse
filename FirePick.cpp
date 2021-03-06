
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "version.h"
#include "FirePick.h"
#include "FireLog.h"
#include "FirePiCam.h"
#include "FireSight.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace FireSight;

#define STATUS_BUFFER_SIZE 1024

char status_buffer[STATUS_BUFFER_SIZE];

const void* firepick_holes(JPG *pJPG) {
	Mat jpg(1, pJPG->length, CV_8UC1, pJPG->pData);
	Mat matRGB = imdecode(jpg, CV_LOAD_IMAGE_COLOR);
	vector<MatchedRegion> matches;

	HoleRecognizer recognizer(26/1.15, 26*1.15);
  recognizer.scan(matRGB, matches);

	imwrite("/home/pi/camcv.bmp", matRGB);

  return pJPG;
}

const char* firepick_status() {
  time_t current_time = time(NULL);
  char timebuf[70];
  strcpy(timebuf, ctime(&current_time));
  timebuf[strlen(timebuf)-1] = 0;

	const char *errorOrWarn = firelog_lastMessage(FIRELOG_WARN);
	if (strlen(errorOrWarn)) {
		return errorOrWarn;
	}

  sprintf(status_buffer, 
    "{\n"
    " 'timestamp':'%s'\n"
    " 'message':'FirePick OK!',\n"
    " 'version':'FireFuse version %d.%d'\n"
    "}\n",
    timebuf,
    FireFuse_VERSION_MAJOR, FireFuse_VERSION_MINOR);
  return status_buffer;
}

int firepick_camera_daemon(JPG *pJPG) {
  int status = firepicam_create(0, NULL);

  LOGINFO1("firepick_camera_daemon start -> %d", status);

  for (;;) {
    JPG_Buffer buffer;
    buffer.pData = NULL;
    buffer.length = 0;
    
    status = firepicam_acquireImage(&buffer);
    pJPG->pData = buffer.pData;
    pJPG->length = buffer.length;
  }

  LOGINFO1("firepick_camera_daemon exit -> %d", status);
  firepicam_destroy(status);
}


