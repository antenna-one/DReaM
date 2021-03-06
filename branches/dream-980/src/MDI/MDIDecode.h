/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Volker Fischer, Oliver Haffenden
 *
 * Description:
 *  see MDIInBuffer.cpp
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

#ifndef MDIDECODE_H_INCLUDED
#define MDIDECODE_H_INCLUDED

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include "../util/Modul.h"
#include "MDIDefinitions.h"
#include "TagPacketDecoderMDI.h"

class CDecodeRSIMDI : public CReceiverModul<_BINARY, _BINARY>
{
public:
    CDecodeRSIMDI():TagPacketDecoderMDI() {}
    virtual ~CDecodeRSIMDI() {}

protected:

    virtual void InitInternal(CParameter& Parameters);
    virtual void ProcessDataInternal(CParameter& Parameters);

    CTagPacketDecoderMDI TagPacketDecoderMDI;
    int iFramesSinceSDC;
    uint32_t last_dlfc;
};

#endif
