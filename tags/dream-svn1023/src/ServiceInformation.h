/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Julian Cable
 *
 * Description:
 *  Information about services gathered from SDC, EPG and web schedules.
 *
 *
 *******************************************************************************
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

#ifndef _SERVICE_INFORMATION_H
#define _SERVICE_INFORMATION_H

#include "GlobalDefinitions.h"
#include <set>
#include <map>

class CServiceInformation
{
public:
    CServiceInformation():data(),savepath(""){}
    virtual ~CServiceInformation() {}
    void setPath(string s) { savepath = s; }
    void loadChannels();
    void saveChannels();
    void addChannel (const string& label, uint32_t sid);
    map<uint32_t,set<string> >::const_iterator find(uint32_t sid) const { return data.find(sid); }
    map<uint32_t,set<string> >::const_iterator begin() const { return data.begin(); }
    map<uint32_t,set<string> >::const_iterator end() const { return data.end(); }
protected:
    map<uint32_t, set<string> > data;
    string savepath;
//    uint32_t        id;    /* this is the primary key but we keep it inside too for completeness */
//    set<string>     label; /* gathered from the SDC. Normally the label is static and is the station name, but
//                              it is officially dynamic so we collect all that we see. */
};
#endif
