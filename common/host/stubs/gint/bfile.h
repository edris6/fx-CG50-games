#pragma once
#include <stdint.h>
enum { BFile_ReadOnly = 1, BFile_WriteOnly = 2, BFile_File = 1 };
int BFile_Open(const uint16_t *path, int mode);
int BFile_Close(int fd);
int BFile_Create(const uint16_t *path, int type, int *size);
int BFile_Remove(const uint16_t *path);
int BFile_Size(int fd);
int BFile_Read(int fd, void *buf, int size, int whence);
int BFile_Write(int fd, const void *buf, int size);
