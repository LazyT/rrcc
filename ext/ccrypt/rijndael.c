/* Copyright (C) 2000-2018 Peter Selinger.
   This file is part of ccrypt. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* rijndael.c - optimized version of the Rijndeal cipher */

/* derived from original source: rijndael-alg-ref.c   v2.0   August '99
 * Reference ANSI C code for NIST competition
 * authors: Paulo Barreto
 *          Vincent Rijmen
 */

#include "rijndael.h"
#include "tables.h"

static int xshifts[3][2][4] = {
  {{0, 1, 2, 3},
   {0, 3, 2, 1}},

  {{0, 1, 2, 3},
   {0, 5, 4, 3}},

  {{0, 1, 3, 4},
   {0, 7, 5, 4}},
};

/* Exor corresponding text input and round key input bytes */
/* the result is written to res, which can be the same as a */
static inline void xKeyAddition(xword32 res[MAXBC], xword32 a[MAXBC],
			 xword32 rk[MAXBC], int BC)
{
  int j;

  for (j = 0; j < BC; j++) {
    res[j] = a[j] ^ rk[j];
  }
}

#if 0				/* code included for reference */

/* shift rows a, return result in res. This avoids having to copy a
   tmp array back to a. res must not be a. */
static inline void xShiftRow(xword32 res[MAXBC], xword32 a[MAXBC], int shift[4],
		      int BC)
{
  xword8 (*a8)[4] = (xword8 (*)[4]) a;
  xword8 (*res8)[4] = (xword8 (*)[4]) res;

  /* Row 0 remains unchanged
   * The other three rows are shifted a variable amount
   */
  int i, j;
  int s;

  for (j = 0; j < BC; j++) {
    res8[j][0] = a8[j][0];
  }
  for (i = 1; i < 4; i++) {
    s = shift[i];
    for (j = 0; j < BC; j++) {
      res8[j][i] = a8[(j + s) % BC][i];
    }
  }
}

static inline void xSubstitution(xword32 a[MAXBC], xword8 box[256], int BC)
{
  xword8 (*a8)[4] = (xword8 (*)[4]) a;

  /* Replace every byte of the input by the byte at that place
   * in the nonlinear S-box
   */
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < BC; j++) {
      a8[j][i] = box[a[j][i]];
    }
  }
}

#endif				/* code included for reference */

/* profiling shows that the ccrypt program spends about 50% of its
   time in the function xShiftSubst. Splitting the inner "for"
   statement into two parts - versus using the expensive "%" modulo
   operation, makes this function about 44% faster, thereby making the
   entire program about 28% faster. With -O3 optimization, the time
   savings are even more dramatic - ccrypt runs between 55% and 65%
   faster on most platforms. */

/* do ShiftRow and Substitution together. res must not be a. */
static inline void xShiftSubst(xword32 res[MAXBC], xword32 a[MAXBC],
			int shift[4], int BC, xword8 box[256])
{
  int i, j;
  int s;
  xword8 (*a8)[4] = (xword8 (*)[4]) a;
  xword8 (*res8)[4] = (xword8 (*)[4]) res;

  for (j = 0; j < BC; j++) {
    res8[j][0] = box[a8[j][0]];
  }
  for (i = 1; i < 4; i++) {
    s = shift[i];
    for (j = 0; j < BC - s; j++) {
      res8[j][i] = box[a8[(j + s)][i]];
    }
    for (j = BC - s; j < BC; j++) {
      res8[j][i] = box[a8[(j + s) - BC][i]];
    }
  }
}

#if 0				/* code included for reference */

/* Mix the four bytes of every column in a linear way */
/* the result is written to res, which may equal a */
static inline void xMixColumn(xword32 res[MAXBC], xword32 a[MAXBC], int BC)
{
  int j;
  xword32 b;
  xword8 (*a8)[4] = (xword8 (*)[4]) a;

  for (j = 0; j < BC; j++) {
    b = M0[0][a8[j][0]].w32;
    b ^= M0[1][a8[j][1]].w32;
    b ^= M0[2][a8[j][2]].w32;
    b ^= M0[3][a8[j][3]].w32;
    res[j] = b;
  }
}

#endif				/* code included for reference */

/* do MixColumn and KeyAddition together */
static inline void xMixAdd(xword32 res[MAXBC], xword32 a[MAXBC],
		    xword32 rk[MAXBC], int BC)
{
  int j;
  xword32 b;
  xword8 (*a8)[4] = (xword8 (*)[4]) a;

  for (j = 0; j < BC; j++) {
    b = M0[0][a8[j][0]].w32;
    b ^= M0[1][a8[j][1]].w32;
    b ^= M0[2][a8[j][2]].w32;
    b ^= M0[3][a8[j][3]].w32;
    b ^= rk[j];
    res[j] = b;
  }
}

/* Mix the four bytes of every column in a linear way
 * This is the opposite operation of xMixColumn */
/* the result is written to res, which may equal a */
static inline void xInvMixColumn(xword32 res[MAXBC], xword32 a[MAXBC], int BC)
{
  int j;
  xword32 b;
  xword8 (*a8)[4] = (xword8 (*)[4]) a;

  for (j = 0; j < BC; j++) {
    b = M1[0][a8[j][0]].w32;
    b ^= M1[1][a8[j][1]].w32;
    b ^= M1[2][a8[j][2]].w32;
    b ^= M1[3][a8[j][3]].w32;
    res[j] = b;
  }
}

#if 0				/* code included for reference */

/* do KeyAddition and InvMixColumn together */
static inline void xAddInvMix(xword32 res[MAXBC], xword32 a[MAXBC],
		       xword32 rk[MAXBC], int BC)
{
  int j;
  xword32 b;
  xword8 (*a8)[4] = (xword8 (*)[4]) a;

  for (j = 0; j < BC; j++) {
    a[j] = a[j] ^ rk[j];
    b = M1[0][a8[j][0]].w32;
    b ^= M1[1][a8[j][1]].w32;
    b ^= M1[2][a8[j][2]].w32;
    b ^= M1[3][a8[j][3]].w32;
    res[j] = b;
  }
}

#endif				/* code included for reference */

int xrijndaelKeySched(xword32 key[], int keyBits, int blockBits,
		      roundkey *rkk)
{
  /* Calculate the necessary round keys
   * The number of calculations depends on keyBits and blockBits */
  int KC, BC, ROUNDS;
  int i, j, t, rconpointer = 0;
  xword8 (*k8)[4] = (xword8 (*)[4]) key;

  switch (keyBits) {
  case 128:
    KC = 4;
    break;
  case 192:
    KC = 6;
    break;
  case 256:
    KC = 8;
    break;
  default:
    return -1;
  }

  switch (blockBits) {
  case 128:
    BC = 4;
    break;
  case 192:
    BC = 6;
    break;
  case 256:
    BC = 8;
    break;
  default:
    return -2;
  }

  ROUNDS = KC > BC ? KC + 6 : BC + 6;

  t = 0;
  /* copy values into round key array */
  for (j = 0; (j < KC) && (t < (ROUNDS + 1) * BC); j++, t++)
    rkk->rk[t] = key[j];

  while (t < (ROUNDS + 1) * BC) {  /* while not enough round key material */
    /* calculate new values */
    for (i = 0; i < 4; i++) {
      k8[0][i] ^= xS[k8[KC - 1][(i + 1) % 4]];
    }
    k8[0][0] ^= xrcon[rconpointer++];

    if (KC != 8) {
      for (j = 1; j < KC; j++) {
	key[j] ^= key[j - 1];
      }
    } else {
      for (j = 1; j < 4; j++) {
	key[j] ^= key[j - 1];
      }
      for (i = 0; i < 4; i++) {
	k8[4][i] ^= xS[k8[3][i]];
      }
      for (j = 5; j < 8; j++) {
	key[j] ^= key[j - 1];
      }
    }
    /* copy values into round key array */
    for (j = 0; (j < KC) && (t < (ROUNDS + 1) * BC); j++, t++) {
      rkk->rk[t] = key[j];
    }
  }

  /* make roundkey structure */
  rkk->BC = BC;
  rkk->KC = KC;
  rkk->ROUNDS = ROUNDS;
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 4; j++) {
      rkk->shift[i][j] = xshifts[(BC - 4) >> 1][i][j];
    }
  }

  return 0;
}

/* Encryption of one block. */

void xrijndaelEncrypt(xword32 block[], roundkey *rkk)
{
  xword32 block2[MAXBC];		/* hold intermediate result */
  int r;

  int *shift = rkk->shift[0];
  int BC = rkk->BC;
  int ROUNDS = rkk->ROUNDS;
  xword32 *rp = rkk->rk;

  /* begin with a key addition */
  xKeyAddition(block, block, rp, BC);
  rp += BC;

  /* ROUNDS-1 ordinary rounds */
  for (r = 1; r < ROUNDS; r++) {
    xShiftSubst(block2, block, shift, BC, xS);
    xMixAdd(block, block2, rp, BC);
    rp += BC;
  }

  /* Last round is special: there is no xMixColumn */
  xShiftSubst(block2, block, shift, BC, xS);
  xKeyAddition(block, block2, rp, BC);
}

void xrijndaelDecrypt(xword32 block[], roundkey *rkk)
{
  xword32 block2[MAXBC];		/* hold intermediate result */
  int r;

  int *shift = rkk->shift[1];
  int BC = rkk->BC;
  int ROUNDS = rkk->ROUNDS;
  xword32 *rp = rkk->rk + ROUNDS * BC;

  /* To decrypt: apply the inverse operations of the encrypt routine,
   *             in opposite order
   * 
   * (xKeyAddition is an involution: it's equal to its inverse)
   * (the inverse of xSubstitution with table S is xSubstitution with the 
   * inverse table of S)
   * (the inverse of xShiftRow is xShiftRow over a suitable distance)
   */

  /* First the special round:
   *   without xInvMixColumn
   *   with extra xKeyAddition
   */
  xKeyAddition(block2, block, rp, BC);
  xShiftSubst(block, block2, shift, BC, xSi);
  rp -= BC;

  /* ROUNDS-1 ordinary rounds
   */
  for (r = ROUNDS - 1; r > 0; r--) {
    xKeyAddition(block, block, rp, BC);
    xInvMixColumn(block2, block, BC);
    xShiftSubst(block, block2, shift, BC, xSi);
    rp -= BC;
  }

  /* End with the extra key addition
   */

  xKeyAddition(block, block, rp, BC);
}
