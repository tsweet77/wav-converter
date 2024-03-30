//#include <iostream>
#include <windows.h>
#include <stdio.h>
//#include <fstream>
//#include <iostream>
#include "AudioFile.h"
#include "tinywav.h"

#include "cxxopts.h"

// default values
#define DEFAULT_DURATION "00:00:10"
#define SAMPLING_RATE  "96000"
#define VOLUME_LEVEL "0.950" 

// byte sizes
#define KB 1024
#define MB 1048576
#define GB 1073741824
#define TB 1099511627776
// human sizes.
#define KB_h 1000
#define MB_h 1000000
#define GB_h 1000000000
#define TB_h 1000000000000
