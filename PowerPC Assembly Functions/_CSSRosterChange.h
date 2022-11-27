#ifndef CSS_ROSTER_CHANGE_PPLUS_V1_H
#define CSS_ROSTER_CHANGE_PPLUS_V1_H

#include "PowerPC Assembly Functions.h"
#include "Code Menu.h"
#include "_lavaBytes.h"
#include "stdafx.h"

void WriteByteVec(std::vector<unsigned char> Bytes, u32 Address, unsigned char addressReg, unsigned char manipReg, std::size_t numToWrite = SIZE_MAX);
void cssRosterChange();

#endif