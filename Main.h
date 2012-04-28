#pragma once
#include <math.h>
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>

#define PI 3.14159265
#define CLAMP(number, min, max) (((number) < (min)) ? (min) : (((number) > (max)) ? (max) : (number)))