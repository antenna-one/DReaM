/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Oliver Haffenden
 *
 * Description:
 *  see RSISubscriber.cpp
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

#ifndef RSI_SUBSCRIBER_H_INCLUDED
#define RSI_SUBSCRIBER_H_INCLUDED

#include "../GlobalDefinitions.h"
#include "TagPacketDecoderRSCIControl.h"
#include "PacketInOut.h"
#include "PacketSinkFile.h"
#include "PacketInOut.h"
#include "AFPacketGenerator.h"

class CPacketSink;
class CDRMReceiver;
class CTagPacketGenerator;

class CRSISubscriber : public CPacketSocket
{
public:
    CRSISubscriber(CPacketSink *pSink = nullptr);

    /* provide a pointer to the receiver for incoming RCI commands */
    /* leave it set to NULL if you want incoming commands to be ignored */
    void SetReceiver(CDRMReceiver *pReceiver);

    virtual bool SetOrigin(const string&) {
        return false;   // only relevant for network subscribers
    }

    /* Set the profile for this subscriber - could be different for different subscribers */
    void SetProfile(const char c);
    char GetProfile(void) const {
        return cProfile;
    }

    void SetPFTFragmentSize(const int iFrag=-1);

    /* Generate and send a packet */
    void TransmitPacket(CTagPacketGenerator& Generator);

    void SetAFPktCRC(const bool bNAFPktCRC) {
        bUseAFCRC = bNAFPktCRC;
    }


    /* from CPacketSink interface */
    virtual void SendPacket(const vector<_BYTE>& vecbydata, uint32_t addr=0, uint16_t port=0);

    /* from CPacketSource, but we really want it for RSCI control */
    virtual void poll()=0;

protected:
    CPacketSink *pPacketSink;
    char cProfile;
    bool bNeedPft;
    size_t fragment_size;
    CTagPacketDecoderRSCIControl TagPacketDecoderRSCIControl;
private:
    CDRMReceiver *pDRMReceiver;
    CAFPacketGenerator AFPacketGenerator;

    bool bUseAFCRC;
    uint16_t sequence_counter;
};


class CRSISubscriberSocket : public CRSISubscriber
{
public:
    CRSISubscriberSocket(CPacketSink *pSink = nullptr);
    virtual ~CRSISubscriberSocket();

    bool SetOrigin(const string& str);
    bool GetOrigin(string& addr);
    bool SetDestination(const string& str);
    bool GetDestination(string& addr);
    void SetPacketSink(CPacketSink *pSink) {
        (void)pSink;
    }
    void ResetPacketSink() {}
    void poll();

private:
    CPacketSocket* pSocket;
    string strDestination;
    // uint32_t uIf;
    // uint32_t uAddr;
    // uint16_t uPort;
};


class CRSISubscriberFile : public CRSISubscriber
{
public:
    CRSISubscriberFile();

    bool SetDestination(const string& strFName);
    void StartRecording();
    void StopRecording();
    void poll() {} // Do Nothing

    bool GetDestination(string& addr);
    bool GetOrigin(string& addr) {
        (void)addr;
        return false;
    }
    void SetPacketSink(CPacketSink *pSink) {
        (void)pSink;
    }
    void ResetPacketSink() {}
private:
    CPacketSinkFile* pPacketSinkFile;
};

#endif

