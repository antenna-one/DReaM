/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Volker Fischer
 *
 * Description:
 *  Tables for carrier mapping
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#if !defined(TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
#define TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_

#include "../GlobalDefinitions.h"


/* Definitions ****************************************************************/
/* FAC ************************************************************************/
#define NUM_FAC_CELLS_DRM30         65
#define NUM_FAC_CELLS_DRMPLUS       244

/* FAC positions. The two numbers are {symbol no, carrier no} */
extern const int iTableFACRobModA[NUM_FAC_CELLS_DRM30][2];

extern const int iTableFACRobModB[NUM_FAC_CELLS_DRM30][2];

extern const int iTableFACRobModC[NUM_FAC_CELLS_DRM30][2];

extern const int iTableFACRobModD[NUM_FAC_CELLS_DRM30][2];

extern const int iTableFACRobModE[NUM_FAC_CELLS_DRMPLUS][2];

/* Frequency pilots ***********************************************************/
#define NUM_FREQ_PILOTS         3
extern const int iTableFreqPilRobModA[NUM_FREQ_PILOTS][2];

extern const int iTableFreqPilRobModB[NUM_FREQ_PILOTS][2];

extern const int iTableFreqPilRobModC[NUM_FREQ_PILOTS][2];

extern const int iTableFreqPilRobModD[NUM_FREQ_PILOTS][2];

/* Time pilots ****************************************************************/
/* The two numbers are: {carrier no, phase} (Phases are normalized to 1024) */
#define RMA_NUM_TIME_PIL    21
extern const int iTableTimePilRobModA[RMA_NUM_TIME_PIL][2];

#define RMB_NUM_TIME_PIL    19
extern const int iTableTimePilRobModB[RMB_NUM_TIME_PIL][2];

#define RMC_NUM_TIME_PIL    19
extern const int iTableTimePilRobModC[RMC_NUM_TIME_PIL][2];

#define RMD_NUM_TIME_PIL    21
extern const int iTableTimePilRobModD[RMD_NUM_TIME_PIL][2];

#define RME_NUM_TIME_PIL    21
extern const int iTableTimePilRobModE[RME_NUM_TIME_PIL][2];

/* Scattered pilots ***********************************************************/
/* Definitions for the positions of scattered pilots */
#define SCAT_PIL_FREQ_INT   1
#define SCAT_PIL_TIME_INT   2
#if 0
extern const int iTableGainCellSubset[4][5];
#else
struct GainCellSubset {
    int c;
    int f;
    int t;
    int m;
};
extern const GainCellSubset gainCellSubsets[5];
#endif

/* Phase definitions of scattered pilots ------------------------------------ */
extern const int iTableScatPilConstRobModA[3];

extern const int iTableScatPilConstRobModB[3];

extern const int iTableScatPilConstRobModC[3];

extern const int iTableScatPilConstRobModD[3];

extern const int iTableScatPilConstRobModE[3];

#define SIZE_ROW_WZ_ROB_MOD_A   5
#define SIZE_COL_WZ_ROB_MOD_A   3
extern const int iScatPilWRobModA[SIZE_ROW_WZ_ROB_MOD_A][SIZE_COL_WZ_ROB_MOD_A];
extern const int iScatPilZRobModA[SIZE_ROW_WZ_ROB_MOD_A][SIZE_COL_WZ_ROB_MOD_A];
extern const int iScatPilQRobModA[SIZE_ROW_WZ_ROB_MOD_A][SIZE_COL_WZ_ROB_MOD_A];

#define SIZE_ROW_WZ_ROB_MOD_B   3
#define SIZE_COL_WZ_ROB_MOD_B   5
extern const int iScatPilWRobModB[SIZE_ROW_WZ_ROB_MOD_B][SIZE_COL_WZ_ROB_MOD_B];
extern const int iScatPilZRobModB[SIZE_ROW_WZ_ROB_MOD_B][SIZE_COL_WZ_ROB_MOD_B];
extern const int iScatPilQRobModB[SIZE_ROW_WZ_ROB_MOD_B][SIZE_COL_WZ_ROB_MOD_B];

#define SIZE_ROW_WZ_ROB_MOD_C   2
#define SIZE_COL_WZ_ROB_MOD_C   10
extern const int iScatPilWRobModC[SIZE_ROW_WZ_ROB_MOD_C][SIZE_COL_WZ_ROB_MOD_C];
extern const int iScatPilZRobModC[SIZE_ROW_WZ_ROB_MOD_C][SIZE_COL_WZ_ROB_MOD_C];
extern const int iScatPilQRobModC[SIZE_ROW_WZ_ROB_MOD_C][SIZE_COL_WZ_ROB_MOD_C];

#define SIZE_ROW_WZ_ROB_MOD_D   3
#define SIZE_COL_WZ_ROB_MOD_D   8
extern const int iScatPilWRobModD[SIZE_ROW_WZ_ROB_MOD_D][SIZE_COL_WZ_ROB_MOD_D];
extern const int iScatPilZRobModD[SIZE_ROW_WZ_ROB_MOD_D][SIZE_COL_WZ_ROB_MOD_D];
extern const int iScatPilQRobModD[SIZE_ROW_WZ_ROB_MOD_D][SIZE_COL_WZ_ROB_MOD_D];

#define SIZE_ROW_WZQ_ROB_MOD_E  4
#define SIZE_COL_WZQ_ROB_MOD_E  10
extern const int ModeEW1024[SIZE_ROW_WZQ_ROB_MOD_E][SIZE_COL_WZQ_ROB_MOD_E];
extern const int ModeEZ256[SIZE_ROW_WZQ_ROB_MOD_E][SIZE_COL_WZQ_ROB_MOD_E];
extern const int ModeEQ1024[SIZE_ROW_WZQ_ROB_MOD_E][SIZE_COL_WZQ_ROB_MOD_E];

/* Gain definitions of scattered pilots ------------------------------------- */
#define NUM_BOOSTED_SCAT_PILOTS     4
extern const int iScatPilGainRobModA[6][NUM_BOOSTED_SCAT_PILOTS];
extern const int iScatPilGainRobModB[6][NUM_BOOSTED_SCAT_PILOTS];
extern const int iScatPilGainRobModC[6][NUM_BOOSTED_SCAT_PILOTS];
extern const int iScatPilGainRobModD[6][NUM_BOOSTED_SCAT_PILOTS];
extern const int iScatPilGainRobModE[6][NUM_BOOSTED_SCAT_PILOTS];

/* Dummy cells for the MSC ****************************************************/
/* Already normalized */
extern const _COMPLEX cDummyCells64QAM[2];

extern const _COMPLEX cDummyCells16QAM[2];


/* Spectrum occupancy, carrier numbers for each mode **************************/
extern const int iTableCarrierKmin[6][5];

extern const int iTableCarrierKmax[6][5];

#endif // !defined(TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
