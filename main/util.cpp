#include "util.h"

long CrgbToLong(CRGB rgb){
    return ((long)rgb.r << 16) | ((long)rgb.g << 8 ) | (long)rgb.b;
}