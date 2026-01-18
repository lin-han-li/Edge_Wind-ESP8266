#ifndef SD_H
#define SD_H

#include "main.h"
#include "usart.h"
#include "stdio.h"
#include "ff.h"

void file_write_float(TCHAR* filename,float* data,int length);
void file_read_float(TCHAR* filename,float* data,int length);

#endif
