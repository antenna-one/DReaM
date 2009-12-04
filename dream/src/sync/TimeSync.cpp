/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Time synchronization
 * This module can have different amounts of input data. If two
 * possible FFT-window positions are found, the next time no new block is
 * requested.
 *
 * Robustness-mode detection
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

#include "TimeSync.h"


/* Implementation *************************************************************/
void CTimeSync::ProcessDataInternal(CParameter& ReceiverParam)
{
	int				i, j, k;
	int				iMaxIndex;
	int				iIntDiffToCenter;
	int				iCurPos;
	int				iDecInpuSize;
	CReal			rMaxValue;
	CReal			rMaxValRMCorr;
	CReal			rSecHighPeak;
	//CReal			rFreqOffsetEst;
	CComplexVector	cvecInpTmp;
	CRealVector		rResMode(NUM_ROBUSTNESS_MODES);
	int				iNewStIndCount = 0;
	/* Max number of detected peaks ("5" for safety reasons. Could be "2") */
	CVector<int>	iNewStartIndexField(5);

	/* Init start index (in case no timing could be detected or init phase) */
	int iStartIndex = iSymbolBlockSize;

	/* Write new block of data at the end of shift register */
	HistoryBuf.AddEnd(*pvecInputData, iInputBlockSize);

	/* In case the time domain frequency offset estimation method is activated,
	   the hilbert filtering of input signal must always be applied */
#ifndef USE_FRQOFFS_TRACK_GUARDCORR
	if ((bTimingAcqu == true) || (bRobModAcqu == true))
#endif
	{
		/* ---------------------------------------------------------------------
		   Data must be band-pass-filtered before applying the algorithms,
		   because we do not know which mode is active when we synchronize
		   the timing and we must assume the worst-case, therefore use only
		   from DC to 4.5 kHz. If the macro "USE_10_KHZ_HILBFILT" is defined,
		   a bandwith of approx. 10 kHz is used. In this case, we have better
		   performance with the most probable 10 kHz mode but may have worse
		   performance with the 4.5 or 5 kHz modes (for the acquisition) */

		/* The FIR filter intermediate buffer must be adjusted to the new
		   input block size since the size can be vary with time */
		cvecInpTmp.Init(iInputBlockSize);

		/* Copy CVector data in CMatlibVector */
		for (i = 0; i < iInputBlockSize; i++)
			cvecInpTmp[i] = (*pvecInputData)[i];

		/* Complex Hilbert filter. We use the copy constructor for storing
		   the result since the sizes of the output vector varies with time.
		   We decimate the signal with this function, too, because we only
		   analyze a spectrum bandwith of approx. 5 [10] kHz */
		CComplexVector cvecOutTmp(
			FirFiltDec(cvecB, cvecInpTmp, cvecZ, GRDCRR_DEC_FACT));

		/* Get size of new output vector */
		iDecInpuSize = cvecOutTmp.Size();

		/* Copy data from Matlib vector in regular vector for storing in
		   shift register
		   TODO: Make vector types compatible (or maybe only use matlib vectors
		   everywhere) */
		cvecOutTmpInterm.Init(iDecInpuSize);
		for (i = 0; i < iDecInpuSize; i++)
			cvecOutTmpInterm[i] = cvecOutTmp[i];

		/* Write new block of data at the end of shift register */
		HistoryBufCorr.AddEnd(cvecOutTmpInterm, iDecInpuSize);


#ifdef USE_FRQOFFS_TRACK_GUARDCORR
		/* ---------------------------------------------------------------------
		   Frequency offset tracking estimation method based on guard-interval
		   correlation.
		   Problem: The tracking frequency range is "GRDCRR_DEC_FACT"-times
		   smaller than the one in the frequency domain tracking algorithm.
		   Therefore, the acquisition unit must work more precisely
		   (see FreqSyncAcq.h) */

		/* Guard-interval correlation at ML estimated timing position */
		/* Calculate start points for correlation. Consider delay from
		   Hilbert-filter */
		const int iHalHilFilDelDec = NUM_TAPS_HILB_FILT / 2 / GRDCRR_DEC_FACT;
		const int iCorrPosFirst = iDecSymBS + iHalHilFilDelDec;
		const int iCorrPosSec =
			iDecSymBS + iHalHilFilDelDec + iLenUsefPart[iSelectedMode];

		/* Actual correlation over entire guard-interval */
		CComplex cGuardCorrFreqTrack = 0.0;
		for (i = 0; i < iLenGuardInt[iSelectedMode]; i++)
		{
			/* Use start point from ML timing estimation. The input stream is
			   automatically adjusted to have this point at "iDecSymBS" */
			cGuardCorrFreqTrack += HistoryBufCorr[i + iCorrPosFirst] *
				Conj(HistoryBufCorr[i + iCorrPosSec]);
		}

		/* Average vector, real and imaginary part separately */
		IIR1(cFreqOffAv, cGuardCorrFreqTrack, rLamFreqOff);

		/* Calculate argument */
		const CReal rFreqOffsetEst = Angle(cFreqOffAv);

		/* Correct measurement average for actually applied frequency
		   correction */
		cFreqOffAv *= CComplex(Cos(-rFreqOffsetEst), Sin(-rFreqOffsetEst));

		/* Integrate the result for controling the frequency offset, normalize
		   estimate */
		ReceiverParam.Lock();
		ReceiverParam.rFreqOffsetTrack -= rFreqOffsetEst * rNormConstFOE;
		ReceiverParam.Unlock();
#endif


		if ((bTimingAcqu == true) || (bRobModAcqu == true))
		{
			/* Guard-interval correlation ----------------------------------- */
			/* Set position pointer back for this block */
			iTimeSyncPos -= iDecInpuSize;

			/* Init start-index count */
			iNewStIndCount = 0;

			/* We use the block in the middle of the buffer for observation */
			for (i = iDecSymBS + iDecSymBS - iDecInpuSize;
				i < iDecSymBS + iDecSymBS; i++)
			{
				/* Only every "iStepSizeGuardCorr"'th value is calculated for
				   efficiency reasons */
				if (i == iTimeSyncPos)
				{
					/* Do the following guard interval correlation for all
					   possible robustness modes (this is needed for robustness
					   mode detection) */
					for (j = 0; j < NUM_ROBUSTNESS_MODES; j++)
					{
						/* Guard-interval correlation ----------------------- */
						/* Speed optimized calculation of the guard-interval
						   correlation. We devide the total block, which has to
						   be computed, in parts of length "iStepSizeGuardCorr".
						   The results of these blocks are stored in a vector.
						   Now, only one new part has to be calculated and one
						   old one has to be subtracted from the global result.
						   Special care has to be taken since "iGuardSize" must
						   not be a multiple of "iStepSizeGuardCorr". Therefore
						   the "if"-condition */
						/* First subtract correlation values shifted out */
						cGuardCorr[j] -=
							veccIntermCorrRes[j][iPosInIntermCResBuf[j]];
						rGuardPow[j] -=
							vecrIntermPowRes[j][iPosInIntermCResBuf[j]];

						/* Calculate new block and add in memory */
						for (k = iLengthOverlap[j]; k < iLenGuardInt[j]; k++)
						{
							/* Actual correlation */
							iCurPos = iTimeSyncPos + k;
							cGuardCorrBlock[j] += HistoryBufCorr[iCurPos] *
								Conj(HistoryBufCorr[iCurPos + iLenUsefPart[j]]);

							/* Energy calculation for ML solution */
							rGuardPowBlock[j] +=
								SqMag(HistoryBufCorr[iCurPos]) +
								SqMag(HistoryBufCorr[iCurPos + iLenUsefPart[j]]);

							/* If one complete block is ready -> store it. We
							   need to add "1" to the k, because otherwise
							   "iLengthOverlap" would satisfy the
							   "if"-condition */
							if (((k + 1) % iStepSizeGuardCorr) == 0)
							{
								veccIntermCorrRes[j][iPosInIntermCResBuf[j]] =
									cGuardCorrBlock[j];

								vecrIntermPowRes[j][iPosInIntermCResBuf[j]] =
									rGuardPowBlock[j];

								/* Add the new block to the global result */
								cGuardCorr[j] += cGuardCorrBlock[j];
								rGuardPow[j] += rGuardPowBlock[j];

								/* Reset block result */
								cGuardCorrBlock[j] = (CReal) 0.0;
								rGuardPowBlock[j] = (CReal) 0.0;

								/* Increase position pointer and test if wrap */
								iPosInIntermCResBuf[j]++;
								if (iPosInIntermCResBuf[j] == iLengthIntermCRes[j])
									iPosInIntermCResBuf[j] = 0;
							}
						}

						/* Save correlation results in shift register */
						for (k = 0; k < iRMCorrBufSize - 1; k++)
							vecrRMCorrBuffer[j][k] = vecrRMCorrBuffer[j][k + 1];

						/* ML solution */
						vecrRMCorrBuffer[j][iRMCorrBufSize - 1] =
							abs(cGuardCorr[j] + cGuardCorrBlock[j]) -
							(rGuardPow[j] + rGuardPowBlock[j]) / 2;
					}

					/* Energy of guard intervall calculation and detection of
					   peak is only needed if timing aquisition is true */
					if (bTimingAcqu == true)
					{
						/* Start timing detection not until initialization phase
						   is finished */
						if (iTiSyncInitCnt > 1)
						{
							/* Decrease counter */
							iTiSyncInitCnt--;
						}
						else
						{
							/* Average the correlation results */
							IIR1(vecCorrAvBuf[iCorrAvInd],
								vecrRMCorrBuffer[iSelectedMode][iRMCorrBufSize - 1],
								1 - rLambdaCoAv);


							/* Energy of guard-interval correlation calculation
							   (this is simply a moving average operation) */
							vecrGuardEnMovAv.Add(vecCorrAvBuf[iCorrAvInd]);


							/* Taking care of correlation average buffer ---- */
							/* We use a "cyclic buffer" structure. This index
							   defines the position in the buffer */
							iCorrAvInd++;
							if (iCorrAvInd == iMaxDetBufSize)
							{
								/* Adaptation of the lambda parameter for
								   guard-interval correlation averaging IIR
								   filter. With this adaptation we achieve
								   better averaging results. A lower bound is
								   defined for this parameter */
								if (rLambdaCoAv <= 0.1)
									rLambdaCoAv = 0.1;
								else
									rLambdaCoAv /= 2;

								iCorrAvInd = 0;
							}


							/* Detection buffer ----------------------------- */
							/* Update buffer for storing the moving average
							   results */
							pMaxDetBuffer.AddEnd(vecrGuardEnMovAv.GetAverage());

							/* Search for maximum */
							iMaxIndex = 0;
							rMaxValue = (CReal) -numeric_limits<_REAL>::max(); /* Init value */
							for (k = 0; k < iMaxDetBufSize; k++)
							{
								if (pMaxDetBuffer[k] > rMaxValue)
								{
									rMaxValue = pMaxDetBuffer[k];
									iMaxIndex = k;
								}
							}

							/* If maximum is in the middle of the interval, mark
							   position as the beginning of the FFT window */
							if (iMaxIndex == iCenterOfMaxDetBuf)
							{
								/* The optimal start position for the FFT-window
								   is the middle of the "MaxDetBuffer" */
								iNewStartIndexField[iNewStIndCount] =
									iTimeSyncPos * GRDCRR_DEC_FACT -
									iSymbolBlockSize / 2 -
									/* Compensate for Hilbert-filter delay. The
									   delay is introduced in the downsampled
									   domain, therefore devide it by
									   "GRDCRR_DEC_FACT" */
									NUM_TAPS_HILB_FILT / 2 / GRDCRR_DEC_FACT;

								iNewStIndCount++;
							}
						}
					}

					/* Set position pointer to next step */
					iTimeSyncPos += iStepSizeGuardCorr;
				}
			}


			/* Robustness mode detection ------------------------------------ */
			if (bRobModAcqu == true)
			{
				/* Start robustness mode detection not until the buffer is
				   filled */
				if (iRobModInitCnt > 1)
				{
					/* Decrease counter */
					iRobModInitCnt--;
				}
				else
				{
					int	iDetectedRModeInd=-1; /* ensures test fails if no assigment in loop */
					/* Correlation of guard-interval correlation with prepared
					   cos-vector. Store highest peak */
					rMaxValRMCorr = (CReal) 0.0;
					for (j = 0; j < NUM_ROBUSTNESS_MODES; j++)
					{
						/* Correlation with symbol rate frequency (Correlations
						   must be normalized to be comparable!
						   ("/ iGuardSizeX")) */
						rResMode[j] =
							Abs(Sum(vecrRMCorrBuffer[j] * vecrCos[j])) /
							iLenGuardInt[j];

						/* Search for maximum */
						if (rResMode[j] > rMaxValRMCorr)
						{
							rMaxValRMCorr = rResMode[j];
							iDetectedRModeInd = j;
						}
					}

					/* Get second highest peak */
					rSecHighPeak = (CReal) 0.0;
					for (j = 0; j < NUM_ROBUSTNESS_MODES; j++)
					{
						if ((rResMode[j] > rSecHighPeak) &&
							(iDetectedRModeInd != j))
						{
							rSecHighPeak = rResMode[j];
						}
					}

					/* Find out if we have a reliable measure
					   (distance to next peak) */
					if ((rMaxValRMCorr / rSecHighPeak) > THRESHOLD_RELI_MEASURE)
					{
						/* Reset aquisition flag for robustness mode detection */
						bRobModAcqu = false;

						/* Set wave mode */
						ReceiverParam.Lock();
						ERobMode eRobm = GetRModeFromInd(iDetectedRModeInd);
						if(eRobm != ReceiverParam.Channel.eRobustness)
						{
						    ReceiverParam.NextConfig.Channel = ReceiverParam.Channel;
						    ReceiverParam.NextConfig.Channel.eRobustness = eRobm;
						    ReceiverParam.RxEvent = ChannelReconfiguration;
							/* Reset output cyclic-buffer because wave mode has
							   changed and the data written in the buffer is not
							   valid anymore */
							SetBufReset1();
						}
						ReceiverParam.Unlock();
					}
				}
			}
		}
	}

	if (bTimingAcqu == true)
	{
		/* Use all measured FFT-window start points for determining the "real"
		   one */
		for (i = 0; i < iNewStIndCount; i++)
		{
			/* Check if new measurement is in range of predefined bound. This
			   bound shall eliminate outliers for the calculation of the
			   filtered result */
			if (((iNewStartIndexField[i] < (iCenterOfBuf + TIMING_BOUND_ABS)) &&
				(iNewStartIndexField[i] > (iCenterOfBuf - TIMING_BOUND_ABS))))
			{
				/* New measurement is in range -> use it for filtering */
				/* Low-pass filter detected start of frame */
				IIR1(rStartIndex, (CReal) iNewStartIndexField[i],
					LAMBDA_LOW_PASS_START);

				/* Reset counters for non-linear correction algorithm */
				iCorrCounter = 0;
				iAveCorr = 0;

				/* GUI message that timing is ok */
				ReceiverParam.Lock();
				ReceiverParam.ReceiveStatus.TSync.SetStatus(RX_OK);
				ReceiverParam.Unlock();

				/* Acquisition was successful, reset init flag (just in case it
				   was not reset by the non-linear correction unit */
				bInitTimingAcqu = false;
			}
			else
			{
				/* Non-linear correction of the filter-output to ged rid of
				   large differences between current measurement and
				   filter-output */
				iCorrCounter++;

				/* Average the NUM_SYM_BEFORE_RESET measurements for reset
				   rStartIndex */
				iAveCorr += iNewStartIndexField[i];

				/* If pre-defined number of outliers is exceed, correct */
				if (iCorrCounter > NUM_SYM_BEFORE_RESET)
				{
					/* If this is the first correction after an initialization
					   was done, reset flag and do not show red light */
					if (bInitTimingAcqu == true)
					{
						/* Reset flag */
						bInitTimingAcqu = false;

						/* Right after initialization, the first estimate is
						   used for correction */
						rStartIndex = (CReal) iNewStartIndexField[i];
					}
					else
					{
						/* Correct filter-output */
						rStartIndex =
							(CReal) iAveCorr / (NUM_SYM_BEFORE_RESET + 1);

						/* GUI message that timing was corrected (red light) */
						ReceiverParam.Lock();
						ReceiverParam.ReceiveStatus.TSync.SetStatus(CRC_ERROR);
						ReceiverParam.Unlock();
					}

					/* Reset counters */
					iCorrCounter = 0;
					iAveCorr = 0;
				}
				else
				{
					/* GUI message that timing is yet ok (yellow light). Do not
					   show any light if init was done right before this */
					if (bInitTimingAcqu == false)
					{
						ReceiverParam.Lock();
						ReceiverParam.ReceiveStatus.TSync.SetStatus(DATA_ERROR);
						ReceiverParam.Unlock();
					}
				}
			}

#ifdef _DEBUG_
/* Save estimated positions of timing (acquisition) */
static FILE* pFile = fopen("test/testtime.dat", "w");
fprintf(pFile, "%d %d\n", iNewStartIndexField[i], iInputBlockSize);
fflush(pFile);
#endif
		}

		/* Convert result to integer format for cutting out the FFT-window */
		iStartIndex = (int) rStartIndex;
	}
	else
	{
		ReceiverParam.Lock();
		/* Detect situation when acquisition was deactivated right now */
		if (bAcqWasActive == true)
		{
			bAcqWasActive = false;

			/* Reset also the tracking value since the tracking could not get
			   right parameters since the timing was not yet correct */
			ReceiverParam.iTimingOffsTrack = 0;
		}

		/* In case of tracking only, use final acquisition result "rStartIndex"
		   (which is not updated any more) and add tracking correction */
		iStartIndex = (int) rStartIndex + ReceiverParam.iTimingOffsTrack;

		/* Timing acquisition was successfully finished, show always green
		   light */
		ReceiverParam.ReceiveStatus.TSync.SetStatus(RX_OK);

		ReceiverParam.Unlock();

#ifdef _DEBUG_
/* Save estimated positions of timing (tracking) */
static FILE* pFile = fopen("test/testtimetrack.dat", "w");
static int iTimeTrackAbs = 0;
iTimeTrackAbs += ReceiverParam.iTimingOffsTrack; /* Integration */
fprintf(pFile, "%d\n", iTimeTrackAbs);
fflush(pFile);
#endif
	}


	/* -------------------------------------------------------------------------
	   Cut out the estimated optimal time window and write it to the output
	   vector. Add the acquisition and tracking offset together for the final
	   timing */
	/* Check range of "iStartIndex" to prevent from vector overwrites. It must
	   be larger than "0" since then the input block size would be also "0" and
	   than the processing routine of the modul would not be called any more */
	const int i2SymBlSize = iSymbolBlockSize + iSymbolBlockSize;
	if (iStartIndex <= 0)
		iStartIndex = 1;
	if (iStartIndex >= i2SymBlSize)
		iStartIndex = i2SymBlSize;

	/* Cut out the useful part of the OFDM symbol */
	for (k = iStartIndex; k < iStartIndex + iDFTSize; k++)
		(*pvecOutputData)[k - iStartIndex] = HistoryBuf[k];

	/* If synchronized DRM input stream is used, overwrite the detected
	   timing */
	if (bSyncInput == true)
	{
		/* Set fixed timing position */
		iStartIndex = iSymbolBlockSize;

		/* Cut out guard-interval at right position -> no channel estimation
		   needed when having only one path. No delay introduced in this
		   module  */
		for (k = iGuardSize; k < iSymbolBlockSize; k++)
		{
			(*pvecOutputData)[k - iGuardSize] =
				HistoryBuf[iTotalBufferSize - iInputBlockSize + k];
		}
	}


	/* -------------------------------------------------------------------------
	   Adjust filtered measurement so that it is back in the center of the
	   buffer */
	/* Integer part of the difference between filtered measurement and the
	   center of the buffer */
	iIntDiffToCenter = iCenterOfBuf - iStartIndex;

	/* Set input block size for next block and reset filtered measurement. This
	   is equal to a global shift of the observation window by the
	   rearrangement of the filtered measurement */
	iInputBlockSize = iSymbolBlockSize - iIntDiffToCenter;

	/* In acquisition mode, correct start index */
	if (bTimingAcqu == true)
		rStartIndex += (CReal) iIntDiffToCenter;


	/* -------------------------------------------------------------------------
	   The channel estimation needs information about timing corrections,
	   because it is using information from the symbol memory. After a
	   timing correction all old symbols must be corrected by a phase
	   rotation as well */
	(*pvecOutputData).GetExData().iCurTimeCorr = iIntDiffToCenter;
}

void CTimeSync::InitInternal(CParameter& ReceiverParam)
{
	int		i, j;
	int		iMaxSymbolBlockSize;
	int		iObservedFreqBin;
	CReal	rArgTemp;
	int		iCorrBuffSize;

	ReceiverParam.Lock();

	/* Get parameters from info class */
	iGuardSize = ReceiverParam.CellMappingTable.iGuardSize;
	iDFTSize = ReceiverParam.CellMappingTable.iFFTSizeN;
	iSymbolBlockSize = ReceiverParam.CellMappingTable.iSymbolBlockSize;

	/* Decimated symbol block size */
	iDecSymBS = iSymbolBlockSize / GRDCRR_DEC_FACT;

	/* Calculate maximum symbol block size (This is Rob. Mode A) */
	iMaxSymbolBlockSize = RMA_FFT_SIZE_N +
		RMA_FFT_SIZE_N * RMA_ENUM_TG_TU / RMA_DENOM_TG_TU;

	/* We need at least two blocks of data for determining the timing */
	iTotalBufferSize = 2 * iSymbolBlockSize + iMaxSymbolBlockSize;
	iCorrBuffSize = iTotalBufferSize / GRDCRR_DEC_FACT;

	/* Set step size of the guard-interval correlation */
	iStepSizeGuardCorr = STEP_SIZE_GUARD_CORR;

	/* Size for moving average buffer for guard-interval correlation */
	iMovAvBufSize =
		(int) ((CReal) iGuardSize / GRDCRR_DEC_FACT / iStepSizeGuardCorr);

	/* Size of buffer, storing the moving-average results for
	   maximum detection */
	iMaxDetBufSize =
		(int) ((CReal) iDecSymBS / iStepSizeGuardCorr);

	/* Center of maximum detection buffer */
	iCenterOfMaxDetBuf = (iMaxDetBufSize - 1) / 2;

	/* Init Energy calculation after guard-interval correlation (moving
	   average) */
	vecrGuardEnMovAv.Init(iMovAvBufSize);

	/* Start position of this value must be at the end of the observation
       window because we reset it at the beginning of the loop */
	iTimeSyncPos = 2 * iDecSymBS;

	/* Calculate center of buffer */
	iCenterOfBuf = iSymbolBlockSize;

	/* Init rStartIndex only if acquisition was activated */
	if (bTimingAcqu == true)
		rStartIndex = (CReal) iCenterOfBuf;

	/* Some inits */
	iAveCorr = 0;
	bInitTimingAcqu = true; /* Flag to show that init was done */

	/* Set correction counter so that a non-linear correction is performed right
	   at the beginning */
	iCorrCounter = NUM_SYM_BEFORE_RESET;


	/* Allocate memory for vectors and zero out */
	HistoryBuf.Init(iTotalBufferSize, (CReal) 0.0);
	pMaxDetBuffer.Init(iMaxDetBufSize, (CReal) 0.0);
	HistoryBufCorr.Init(iCorrBuffSize, (CReal) 0.0);


	/* Inits for averaging the guard-interval correlation */
	vecCorrAvBuf.Init(iMaxDetBufSize, (CReal) 0.0);
	iCorrAvInd = 0;


	/* Set the selected robustness mode index */
	iSelectedMode = GetIndFromRMode(ReceiverParam.Channel.eRobustness);

	/* Init init count for timing sync (one symbol) */
	iTiSyncInitCnt = iDecSymBS / iStepSizeGuardCorr;


	/* Inits for guard-interval correlation and robustness mode detection --- */
	/* Size for robustness mode correlation buffer */
	iRMCorrBufSize = (int) ((CReal) NUM_BLOCKS_FOR_RM_CORR * iDecSymBS
		/ STEP_SIZE_GUARD_CORR);

	/* Init init count for robustness mode detection (do not use the very first
	   block) */
	iRobModInitCnt = NUM_BLOCKS_FOR_RM_CORR + 1;

	for (i = 0; i < NUM_ROBUSTNESS_MODES; i++)
	{
		cGuardCorr[i] = (CReal) 0.0;
		cGuardCorrBlock[i] = (CReal) 0.0;
		rGuardPow[i] = (CReal) 0.0;
		rGuardPowBlock[i] = (CReal) 0.0;
		iPosInIntermCResBuf[i] = 0;

		/* Set length of the useful part of the symbol and guard size */
		switch (i)
		{
		case 0:
			iLenUsefPart[i] = RMA_FFT_SIZE_N / GRDCRR_DEC_FACT;
			iLenGuardInt[i] = (int) ((CReal) RMA_FFT_SIZE_N *
				RMA_ENUM_TG_TU / RMA_DENOM_TG_TU / GRDCRR_DEC_FACT);
			break;

		case 1:
			iLenUsefPart[i] = RMB_FFT_SIZE_N / GRDCRR_DEC_FACT;
			iLenGuardInt[i] = (int) ((CReal) RMB_FFT_SIZE_N *
				RMB_ENUM_TG_TU / RMB_DENOM_TG_TU / GRDCRR_DEC_FACT);
			break;

		case 2:
			iLenUsefPart[i] = RMC_FFT_SIZE_N / GRDCRR_DEC_FACT;
			iLenGuardInt[i] = (int) ((CReal) RMC_FFT_SIZE_N *
				RMC_ENUM_TG_TU / RMC_DENOM_TG_TU / GRDCRR_DEC_FACT);
			break;

		case 3:
			iLenUsefPart[i] = RMD_FFT_SIZE_N / GRDCRR_DEC_FACT;
			iLenGuardInt[i] = (int) ((CReal) RMD_FFT_SIZE_N *
				RMD_ENUM_TG_TU / RMD_DENOM_TG_TU / GRDCRR_DEC_FACT);
			break;
		}

		/* Number of correlation result blocks to be stored in a vector. This is
		   the total length of the guard-interval divided by the step size.
		   Since the guard-size must not be a multiple of "iStepSizeGuardCorr",
		   we need to cut-off the fractional part */
		iLengthIntermCRes[i] = (int) ((CReal) iLenGuardInt[i] /
			iStepSizeGuardCorr);

		/* This length is the start point for the "for"-loop */
		iLengthOverlap[i] = iLenGuardInt[i] -
			iStepSizeGuardCorr;

		/* Intermediate correlation results vector (init, zero out) */
		veccIntermCorrRes[i].Init(iLengthIntermCRes[i], (CReal) 0.0);
		vecrIntermPowRes[i].Init(iLengthIntermCRes[i], (CReal) 0.0);

		/* Allocate memory for correlation input buffers */
		vecrRMCorrBuffer[i].Init(iRMCorrBufSize);

		/* Tables for sin and cos function for the desired frequency band */
		/* First, allocate memory for vector */
		vecrCos[i].Init(iRMCorrBufSize);

		/* Build table */
		for (j = 0; j < iRMCorrBufSize; j++)
		{
			/* Calculate frequency bins which has to be observed for each
			   mode.
			   Mode A: f_A = 1 / T_s = 1 / 26.66 ms = 37.5 Hz
			   Mode B: f_B = 1 / T_s = 1 / 26.66 ms = 37.5 Hz
			   Mode C: f_C = 1 / T_s = 1 / 20 ms = 50 Hz
			   Mode D: f_D = 1 / T_s = 1 / 16.66 ms = 60 Hz */
			iObservedFreqBin =
				(int) ((CReal) iRMCorrBufSize * STEP_SIZE_GUARD_CORR /
				(iLenUsefPart[i] + iLenGuardInt[i]));

			rArgTemp = (CReal) 2.0 * crPi / iRMCorrBufSize * j;

			vecrCos[i][j] = cos(rArgTemp * iObservedFreqBin);
		}
	}

#ifdef USE_FRQOFFS_TRACK_GUARDCORR
	/* Init vector for averaging the frequency offset estimation */
	cFreqOffAv = CComplex((CReal) 0.0, (CReal) 0.0);

	/* Init time constant for IIR filter for frequency offset estimation */
	rLamFreqOff = IIR1Lam(TICONST_FREQ_OFF_EST_GUCORR,
		(CReal) SOUNDCRD_SAMPLE_RATE / ReceiverParam.iSymbolBlockSize);

	/* Nomalization constant for frequency offset estimation */
	rNormConstFOE = (CReal) 1.0 /
		((CReal) 2.0 * crPi * ReceiverParam.iFFTSizeN * GRDCRR_DEC_FACT);
#endif

	/* Define block-sizes for input and output */
	iInputBlockSize = iSymbolBlockSize; /* For the first loop */
	iOutputBlockSize = iDFTSize;

	ReceiverParam.Unlock();
}

void CTimeSync::StartAcquisition()
{

// TODO: check which initialization should be done here and which should be
// moved to / from the "InitInternal()" function

	/* The regular acquisition flags */
	bTimingAcqu = true;
	bRobModAcqu = true;

	/* Set the init flag so that the "rStartIndex" can be initialized with the
	   center of the buffer and other important settings can be done */
	SetInitFlag();

	/* This second flag is to determine the moment when the acquisition just
	   finished. In this case, the tracking value must be reset */
	bAcqWasActive = true;

	iCorrCounter = NUM_SYM_BEFORE_RESET;

	/* Reset the buffers which are storing data for correlation (for robustness
	   mode detection) */
	for (int i = 0; i < NUM_ROBUSTNESS_MODES; i++)
		vecrRMCorrBuffer[i] = Zeros(iRMCorrBufSize);

	/* Reset lambda for averaging the guard-interval correlation results */
	rLambdaCoAv = (CReal) 1.0;
	iCorrAvInd = 0;
}

void CTimeSync::SetFilterTaps(const CReal rNewOffsetNorm)
{
#ifdef USE_10_KHZ_HILBFILT
	float * fHilLPProt = fHilLPProt10;
#else
	float * fHilLPProt = fHilLPProt5;

	/* The filter should be on the right of the DC carrier in 5 kHz mode */
	rNewOffsetNorm += (CReal) HILB_FILT_BNDWIDTH / 2 / SOUNDCRD_SAMPLE_RATE;
#endif

	/* Calculate filter taps for complex Hilbert filter */
	cvecB.Init(NUM_TAPS_HILB_FILT);

	for (int i = 0; i < NUM_TAPS_HILB_FILT; i++)
		cvecB[i] = CComplex(
			fHilLPProt[i] * Cos((CReal) 2.0 * crPi * rNewOffsetNorm * i),
			fHilLPProt[i] * Sin((CReal) 2.0 * crPi * rNewOffsetNorm * i));

	/* Init state vector for filtering with zeros */
	cvecZ.Init(NUM_TAPS_HILB_FILT - 1, (CReal) 0.0);
}

CTimeSync::CTimeSync() : iTimeSyncPos(0), bSyncInput(false), bTimingAcqu(false),
	bRobModAcqu(false), bAcqWasActive(false), rLambdaCoAv((CReal) 1.0),
	iLengthIntermCRes(NUM_ROBUSTNESS_MODES),
	iPosInIntermCResBuf(NUM_ROBUSTNESS_MODES),
	iLengthOverlap(NUM_ROBUSTNESS_MODES), iLenUsefPart(NUM_ROBUSTNESS_MODES),
	iLenGuardInt(NUM_ROBUSTNESS_MODES), cGuardCorr(NUM_ROBUSTNESS_MODES),
	cGuardCorrBlock(NUM_ROBUSTNESS_MODES), rGuardPow(NUM_ROBUSTNESS_MODES),
	rGuardPowBlock(NUM_ROBUSTNESS_MODES)
{
	/* Init Hilbert filter. Since the frequency offset correction was
	   done in the previous module, the offset for the filter is
	   always "VIRTUAL_INTERMED_FREQ" */
	SetFilterTaps((_REAL) VIRTUAL_INTERMED_FREQ / SOUNDCRD_SAMPLE_RATE);
}

int CTimeSync::GetIndFromRMode(ERobMode eNewMode)
{
	/* Get the robustness mode index. We define:
	   A: 0, B: 1, C: 2, D: 3 */
	switch (eNewMode)
	{
	case RM_ROBUSTNESS_MODE_A:
		return 0;
	case RM_ROBUSTNESS_MODE_B:
		return 1;
	case RM_ROBUSTNESS_MODE_C:
		return 2;
	case RM_ROBUSTNESS_MODE_D:
		return 3;
	default:
		return 0;
	}
}

ERobMode CTimeSync::GetRModeFromInd(int iNewInd)
{
	/* Get the robustness mode index. We define:
	   A: 0, B: 1, C: 2, D: 3 */
	switch (iNewInd)
	{
	case 0:
		return RM_ROBUSTNESS_MODE_A;
	case 1:
		return RM_ROBUSTNESS_MODE_B;
	case 2:
		return RM_ROBUSTNESS_MODE_C;
	case 3:
		return RM_ROBUSTNESS_MODE_D;
	default:
		return RM_ROBUSTNESS_MODE_A;
	}
}
