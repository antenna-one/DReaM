/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Volker Fischer
 *
 * Description:
 *  See FreqSyncAcq.cpp
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

#if !defined(FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_)
#define FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_

#include "../Parameter.h"
#include "../util/Modul.h"
#include "../matlib/Matlib.h"
#include "../util/Utilities.h"

/* Definitions ****************************************************************/
/* Bound for peak detection between filtered signal (in frequency direction)
   and the unfiltered signal. Define different bounds for different relative
   search window sizes */
#define PEAK_BOUND_FILT2SIGNAL_1        ((CReal) 9)
#define PEAK_BOUND_FILT2SIGNAL_0_042    ((CReal) 7)

/* This value MUST BE AT LEAST 2, because otherwise we would get an overrun
   when we try to add a complete symbol to the buffer! */
#ifdef USE_FRQOFFS_TRACK_GUARDCORR
# define NUM_BLOCKS_4_FREQ_ACQU         30 /* Accuracy must be higher */
#else
# define NUM_BLOCKS_4_FREQ_ACQU         6
#endif

/* Number of block used for averaging */
#define NUM_BLOCKS_USED_FOR_AV          3

/* Lambda for IIR filter for estimating noise floor in frequency domain */
#define LAMBDA_FREQ_IIR_FILT            ((CReal) 0.87)

/* Ratio between highest and second highest peak at the frequency pilot
   positions in the PSD estimation (after peak detection) */
#define MAX_RAT_PEAKS_AT_PIL_POS_HIGH   ((CReal) 0.99)

/* Ratio between highest and lowest peak at frequency pilot positions (see
   above) */
#define MAX_RAT_PEAKS_AT_PIL_POS_LOW    ((CReal) 0.8)

/* Number of blocks storing the squared magnitude of FFT used for
   averaging */
#define NUM_FFT_RES_AV_BLOCKS           (NUM_BLOCKS_4_FREQ_ACQU * (NUM_BLOCKS_USED_FOR_AV - 1) + 1)


/* Classes ********************************************************************/
class FreqOffsetMode
{
public:
    virtual ~FreqOffsetMode(){}
    virtual void init(int iHalfBuffer, int iSampleRate)=0;
    virtual bool calcOffset(const CRealVector& vecrPSD, int& offset)=0;
    void setSearchWindow(_REAL rNewCenterFreq, _REAL rNewWinSize) {
        rCenterFreq = rNewCenterFreq;
        rWinSize = rNewWinSize;
    }
protected:
    double rCenterFreq, rWinSize;
};

class FreqOffsetModeABCD: public FreqOffsetMode
{
public:
    FreqOffsetModeABCD(const FreqOffsetMode& f):FreqOffsetMode(f),veciTableFreqPilots(3)
    {
        setSearchWindow(rCenterFreq, rWinSize);
    }
    FreqOffsetModeABCD():FreqOffsetMode(),veciTableFreqPilots(3)
    {
        setSearchWindow(0.0, 0.0);
    }
    virtual ~FreqOffsetModeABCD(){}
    void init(int iHalfBuffer, int iSampleRate);
    bool calcOffset(const CRealVector& vecrPSD, int& offset);
private:
    int                         iStartDCSearch;
    int                         iEndDCSearch;
    _REAL                       rPeakBoundFiltToSig;

    CVector<int>                veciTableFreqPilots;
    CRealVector                 vecrPSDPilCor;
    CVector<int>                veciPeakIndex;
    int                         iSearchWinSize;
};

class FreqOffsetModeE: public FreqOffsetMode
{
public:
    FreqOffsetModeE(const FreqOffsetMode& f):FreqOffsetMode(f)
    {
        setSearchWindow(rCenterFreq, rWinSize);
    }
    void init(int iHalfBuffer, int iSampleRate);
    bool calcOffset(const CRealVector& vecrPSD, int& offset);
};

class CFreqSyncAcq : public CReceiverModul<_REAL, _COMPLEX>
{
public:
    CFreqSyncAcq() :
        acquired(false), bSyncInput(FALSE),bUseRecFilter(FALSE),
        foMode(new FreqOffsetModeABCD())
    {}
    virtual ~CFreqSyncAcq() {}

    void SetSearchWindow(_REAL rNewCenterFreq, _REAL rNewWinSize);

    void StartAcquisition();
    void StopAcquisition() {
        acquired = true;
    }
    _BOOLEAN GetAcquisition() {
        return !acquired;
    }

    void SetRecFilter(const _BOOLEAN bNewF) {
        bUseRecFilter = bNewF;
    }
    _BOOLEAN GetRecFilter() {
        return bUseRecFilter;
    }
    _BOOLEAN GetUnlockedFrameBoundary() {
        return iFreeSymbolCounter==0;
    }

    /* To set the module up for synchronized DRM input data stream */
    void SetSyncInput(_BOOLEAN bNewS) {
        bSyncInput = bNewS;
    }

protected:
    CShiftRegister<_REAL>       vecrFFTHistory;

    CFftPlans                   FftPlan;
    CRealVector                 vecrFFTInput;
    CRealVector                 vecrSqMagFFTOut;
    CRealVector                 vecrHammingWin;
    CMovingAv<CRealVector>      vvrPSDMovAv;

    int                         iFrAcFFTSize;
    int                         iHistBufSize;

    int                         iFFTSize;

    bool                        acquired;

    int                         iAquisitionCounter;
    int                         iAverageCounter;

    _BOOLEAN                    bSyncInput;

    int                         iHalfBuffer;
    int                         iSampleRate;

    _COMPLEX                    cCurExp;
    _REAL                       rInternIFNorm;

    CDRMBandpassFilt            BPFilter;
    _BOOLEAN                    bUseRecFilter;
    FreqOffsetMode*             foMode;

    /* OPH: counter to count symbols within a frame in order to generate */
    /* RSCI output even when unlocked */
    unsigned int                iFreeSymbolCounter;

    virtual void InitInternal(CParameter& Parameters);
    virtual void ProcessDataInternal(CParameter& Parameters);
};


#endif // !defined(FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_)
