/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2007
 *
 * Author(s):
 *	Volker Fischer, Andrew Murphy, Andrea Russo, Oliver Haffenden
 *
 * Description:
 *	This class takes care of keeping the history plots as well as interfacing to the
 *  relevant module in the case of the other plots, including getting data either from
 *  the RSCI input or the demodulator
 *
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

#include "PlotManager.h"
#include "DrmReceiver.h"
#include <iostream>

CPlotManager::CPlotManager() :
        pReceiver(0),
        vecrFreqSyncValHist(LEN_HIST_PLOT_SYNC_PARMS),
        vecrSamOffsValHist(LEN_HIST_PLOT_SYNC_PARMS),
        vecrLenIRHist(LEN_HIST_PLOT_SYNC_PARMS),
        vecrDopplerHist(LEN_HIST_PLOT_SYNC_PARMS),
        vecrSNRHist(LEN_HIST_PLOT_SYNC_PARMS),
        veciCDAudHist(LEN_HIST_PLOT_SYNC_PARMS), iSymbolCount(0),
        rSumDopplerHist((_REAL) 0.0), rSumSNRHist((_REAL) 0.0), iCurrentCDAud(0)
{
}

/* Parameter histories for plot --------------------------------------------- */

void CPlotManager::Init()
{
    iSymbolCount = 0;
    rSumDopplerHist = (_REAL) 0.0;
    rSumSNRHist = (_REAL) 0.0;
}

void
CPlotManager::UpdateParamHistories(ERecState eReceiverState)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();

    /* TODO: do not use the shift register class, build a new
       one which just increments a pointer in a buffer and put
       the new value at the position of the pointer instead of
       moving the total data all the time -> special care has
       to be taken when reading out the data */

    /* Only update histories if the receiver is in tracking mode */
    if (eReceiverState == RS_TRACKING)
    {
        ReceiverParam.Lock();
        _REAL rFreqOffsetTrack = ReceiverParam.rFreqOffsetTrack;
        _REAL rResampleOffset = ReceiverParam.rResampleOffset;
        _REAL rSNR = ReceiverParam.GetSNR();
        _REAL rSigmaEstimate = ReceiverParam.rSigmaEstimate;
        _REAL iNumSymPerFrame = ReceiverParam.CellMappingTable.iNumSymPerFrame;
        _REAL rMeanDelay = (ReceiverParam.rMinDelay +	ReceiverParam.rMaxDelay) / 2.0;
        ReceiverParam.Unlock();

#ifdef USE_QT_GUI
        MutexHist.lock();
#endif

        /* Frequency offset tracking values */
        vecrFreqSyncValHist.AddEnd(rFreqOffsetTrack * SOUNDCRD_SAMPLE_RATE);

        /* Sample rate offset estimation */
        vecrSamOffsValHist.AddEnd(rResampleOffset);
        /* Signal to Noise ratio estimates */
        rSumSNRHist += rSNR;

        /* TODO - reconcile this with Ollies RSCI Doppler code in ChannelEstimation */
        /* Average Doppler estimate */
        rSumDopplerHist += rSigmaEstimate;

        /* Only evaluate Doppler and delay once in one DRM frame */
        iSymbolCount++;
        if (iSymbolCount == iNumSymPerFrame)
        {
            /* Apply averaged values to the history vectors */
            vecrLenIRHist.AddEnd(rMeanDelay);

            vecrSNRHist.AddEnd(rSumSNRHist / iNumSymPerFrame);

            vecrDopplerHist.AddEnd(rSumDopplerHist / iNumSymPerFrame);

            /* At the same time, add number of correctly decoded audio blocks.
               This number is updated once a DRM frame. Since the other
               parameters like SNR is also updated once a DRM frame, the two
               values are synchronized by one DRM frame */
            veciCDAudHist.AddEnd(iCurrentCDAud);

            /* Reset parameters used for averaging */
            iSymbolCount = 0;
            rSumDopplerHist = (_REAL) 0.0;
            rSumSNRHist = (_REAL) 0.0;
        }

#ifdef USE_QT_GUI
        MutexHist.unlock();
#endif
    }
}

void
CPlotManager::UpdateParamHistoriesRSIIn()
{
    /* This function is only called once per RSI frame, so process every time */

    CParameter& ReceiverParam = *pReceiver->GetParameters();

    ReceiverParam.Lock();
    _REAL rDelay = _REAL(0.0);
    if (ReceiverParam.vecrRdelIntervals.GetSize() > 0)
        rDelay = ReceiverParam.vecrRdelIntervals[0];
    _REAL rMER = ReceiverParam.rMER;
    _REAL rRdop = ReceiverParam.rRdop;
    ReceiverParam.Unlock();

#ifdef USE_QT_GUI
    MutexHist.lock();
#endif

    /* Apply averaged values to the history vectors */
    vecrLenIRHist.AddEnd(rDelay);
    vecrSNRHist.AddEnd(rMER);
    vecrDopplerHist.AddEnd(rRdop);

    /* At the same time, add number of correctly decoded audio blocks.
       This number is updated once a DRM frame. Since the other
       parameters like SNR is also updated once a DRM frame, the two
       values are synchronized by one DRM frame */
    veciCDAudHist.AddEnd(iCurrentCDAud);
    /* Reset parameters used for averaging */
    iSymbolCount = 0;
    rSumDopplerHist = (_REAL) 0.0;
    rSumSNRHist = (_REAL) 0.0;

#ifdef USE_QT_GUI
    MutexHist.unlock();
#endif
}

void
CPlotManager::GetFreqSamOffsHist(CVector < _REAL > &vecrFreqOffs,
                                 CVector < _REAL > &vecrSamOffs,
                                 CVector < _REAL > &vecrScale,
                                 _REAL & rFreqAquVal)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();

    ReceiverParam.Lock();
    /* Duration of OFDM symbol */
    const _REAL rTs = (CReal) (ReceiverParam.CellMappingTable.iFFTSizeN + ReceiverParam.CellMappingTable.iGuardSize) / SOUNDCRD_SAMPLE_RATE;
    /* Value from frequency acquisition */
    rFreqAquVal = ReceiverParam.rFreqOffsetAcqui * SOUNDCRD_SAMPLE_RATE;
    ReceiverParam.Unlock();

    /* Init output vectors */
    vecrFreqOffs.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrSamOffs.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

    /* Lock resources */
#ifdef USE_QT_GUI
    MutexHist.lock();
#endif

    /* Simply copy history buffers in output buffers */
    vecrFreqOffs = vecrFreqSyncValHist;
    vecrSamOffs = vecrSamOffsValHist;

    /* Calculate time scale */
    for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
        vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rTs;

    /* Release resources */
#ifdef USE_QT_GUI
    MutexHist.unlock();
#endif
}

void
CPlotManager::GetDopplerDelHist(CVector < _REAL > &vecrLenIR,
                                CVector < _REAL > &vecrDoppler,
                                CVector < _REAL > &vecrScale)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();

    /* Init output vectors */
    vecrLenIR.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrDoppler.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

    ReceiverParam.Lock();
    /* Duration of DRM frame */
    const _REAL rDRMFrameDur = (CReal) (ReceiverParam.CellMappingTable.iFFTSizeN
                                        + ReceiverParam.CellMappingTable.iGuardSize) /
                               SOUNDCRD_SAMPLE_RATE * ReceiverParam.CellMappingTable.iNumSymPerFrame;
    ReceiverParam.Unlock();

    /* Lock resources */
#ifdef USE_QT_GUI
    MutexHist.lock();
#endif

    /* Simply copy history buffers in output buffers */
    vecrLenIR = vecrLenIRHist;
    vecrDoppler = vecrDopplerHist;


    /* Calculate time scale in minutes */
    for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
        vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rDRMFrameDur / 60;

    /* Release resources */
#ifdef USE_QT_GUI
    MutexHist.unlock();
#endif
}

void
CPlotManager::GetSNRHist(CVector < _REAL > &vecrSNR,
                         CVector < _REAL > &vecrCDAud,
                         CVector < _REAL > &vecrScale)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();
    /* Duration of DRM frame */
    ReceiverParam.Lock();
    /* Duration of DRM frame */
    const _REAL rDRMFrameDur = (CReal) (ReceiverParam.CellMappingTable.iFFTSizeN + ReceiverParam.CellMappingTable.iGuardSize) /
                               SOUNDCRD_SAMPLE_RATE * ReceiverParam.CellMappingTable.iNumSymPerFrame;
    ReceiverParam.Unlock();

    /* Init output vectors */
    vecrSNR.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrCDAud.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
    vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

    /* Lock resources */
#ifdef USE_QT_GUI
    MutexHist.lock();
#endif

    /* Simply copy history buffer in output buffer */
    vecrSNR = vecrSNRHist;

    /* Calculate time scale. Copy correctly decoded audio blocks history (must
       be transformed from "int" to "real", therefore we need a for-loop */
    for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
    {
        /* Scale in minutes */
        vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rDRMFrameDur / 60;

        /* Correctly decoded audio blocks */
        vecrCDAud[i] = (_REAL) veciCDAudHist[i];
    }

    /* Release resources */
#ifdef USE_QT_GUI
    MutexHist.unlock();
#endif
}

void
CPlotManager::GetInputPSD(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();

    if (pReceiver->GetRSIIn()->GetInEnabled())
    {
        // read it from the parameter structure
        ReceiverParam.Lock();
        CVector<_REAL>& vecrPSD = ReceiverParam.vecrPSD;
        ReceiverParam.Unlock();

        int iVectorLen = vecrPSD.Size();
        vecrData.Init(iVectorLen);
        vecrScale.Init(iVectorLen);

        // starting frequency and frequency step as defined in TS 102 349
        // plot expects the scale values in kHz
        _REAL f = _REAL(-7.875) + VIRTUAL_INTERMED_FREQ/_REAL(1000.0);
        const _REAL fstep =_REAL(0.1875);

        for (int i=0; i<iVectorLen; i++)
        {
            vecrData[i] = vecrPSD[i];
            vecrScale[i] = f;
            f += fstep;
        }

    }
    else
    {
        pReceiver->GetReceiveData()->GetInputPSD(vecrData, vecrScale);
    }
}

void CPlotManager::GetTransferFunction(CVector<_REAL>& vecrData,
                                       CVector<_REAL>& vecrGrpDly,	CVector<_REAL>& vecrScale)
{
    pReceiver->GetChannelEstimation()->GetTransferFunction(vecrData, vecrGrpDly, vecrScale);
}


void CPlotManager::GetAvPoDeSp(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale,
                               _REAL& rLowerBound, _REAL& rHigherBound,
                               _REAL& rStartGuard, _REAL& rEndGuard, _REAL& rPDSBegin,
                               _REAL& rPDSEnd)
{
    CParameter& ReceiverParam = *pReceiver->GetParameters();

    if (pReceiver->GetRSIIn()->GetInEnabled())
    {
        // read it from the parameter structure
        ReceiverParam.Lock();
        CVector<_REAL>& vecrPIR = ReceiverParam.vecrPIR;
        _REAL rPIRStart = ReceiverParam.rPIRStart;
        _REAL rPIREnd = ReceiverParam.rPIREnd;
        ReceiverParam.Unlock();

        int iVectorLen = vecrPIR.Size();
        vecrData.Init(iVectorLen);
        vecrScale.Init(iVectorLen);

        // starting frequency and frequency step as defined in TS 102 349
        // plot expects the scale values in kHz
        _REAL t = rPIRStart;
        const _REAL tstep = (rPIREnd-rPIRStart)/(_REAL(iVectorLen)-1);

        for (int i=0; i<iVectorLen; i++)
        {
            vecrData[i] = vecrPIR[i];
            vecrScale[i] = t;
            t += tstep;
        }

        rLowerBound = _REAL(0);
        rHigherBound = _REAL(0);
        rStartGuard = _REAL(0);
        rEndGuard = _REAL(0);
        rPDSBegin = _REAL(0);
        rPDSEnd = _REAL(0);

    }
    else
    {
        pReceiver->GetChannelEstimation()->GetAvPoDeSp(vecrData, vecrScale, rLowerBound, rHigherBound,
                rStartGuard, rEndGuard, rPDSBegin, rPDSEnd);
    }
}

void CPlotManager::GetSNRProfile(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    pReceiver->GetChannelEstimation()->GetSNRProfile(vecrData, vecrScale);
}

