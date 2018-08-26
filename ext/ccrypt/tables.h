/* Copyright (C) 2000-2018 Peter Selinger.
   This file is part of ccrypt. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

#ifndef TABLES_H
#define TABLES_H

#include "rijndael.h"

extern xword8x4 M0[4][256];
extern xword8x4 M1[4][256];
extern int xrcon[30];
extern xword8 xS[256];
extern xword8 xSi[256];

#endif /* TABLES_H */
