/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014, 2001-2014
 *
 * Author(s):
 *  Julian Cable, David Flamand
 *
 * Decription:
 *  sound interfaces
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

#ifndef _SELECTIONINTERFACE_H
#define _SELECTIONINTERFACE_H

#include "../GlobalDefinitions.h"
#include <vector>
#include <map>
#include <string>

#define DEFAULT_DEVICE_NAME ""

typedef struct {
    string          name; /* unique name that identify the device */
    string          desc; /* optional device description, set to empty string when not used */
    map<int,bool>   samplerates; /* supported sample rates by the device */
} deviceprop;

// TODO update sound interface backend that use old api, remove old Enumerate(names, descs) and make this class pure virtual

class CSelectionInterface
{
public:
    virtual ~CSelectionInterface();
    /* new/updated interface should reimplement that one */
    virtual void        Enumerate(vector<deviceprop>& devs, const int* desiredsamplerates);
    /* for backward compatibility */
    virtual void        Enumerate(vector<string>& names, vector<string>& descriptions)=0;
    virtual string      GetDev()=0;
    virtual void        SetDev(string sNewDev)=0;
};

#endif
