/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Julian Cable
 *
 * Decription:
 * sound interface selection
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

#ifndef SOUND_H
#define SOUND_H

#if defined(WIN32) && !defined(USE_PORTAUDIO) && !defined(USE_JACK) && !defined(USE_QTAUDIO)
/* mmsystem sound interface */
# include "../windows/Sound.h"
# endif

# ifdef USE_ALSA
#  include "../linux/soundin.h"
#  include "../linux/soundout.h"
# endif

# ifdef USE_JACK
#  include "../linux/jack.h"
typedef CSoundInJack CSoundIn;
typedef CSoundOutJack CSoundOut;
# endif

# ifdef USE_PULSEAUDIO
#  include "drm_pulseaudio.h"
typedef CSoundInPulse CSoundIn;
typedef CSoundOutPulse CSoundOut;
# endif

# ifdef USE_PORTAUDIO
#  include "drm_portaudio.h"
typedef CPaIn CSoundIn;
typedef CPaOut CSoundOut;
# endif

# ifdef USE_OPENSL
#  include "../android/soundin.h"
#  include "../android/soundout.h"
typedef COpenSLESIn CSoundIn;
typedef COpenSLESOut CSoundOut;
# endif

# ifdef USE_QTAUDIO
#  include "qtaudio.h"
typedef CSoundInQT CSoundIn;
typedef CSoundOutQT CSoundOut;
# endif

# if !defined(USE_ALSA) && !defined(USE_JACK) && !defined(USE_PULSEAUDIO) && !defined(USE_PORTAUDIO) && !defined(USE_OPENSL) && !defined(USE_QTAUDIO)
#  include "soundnull.h"
typedef CSoundInNull CSoundIn;
typedef CSoundOutNull CSoundOut;
# endif

#endif
