TEMPLATE = app
CONFIG += warn_on
TARGET = dream
OBJECTS_DIR = obj
DEFINES += EXECUTABLE_NAME=$$TARGET
LIBS += -L$$PWD/lib
INCLUDEPATH += $$PWD/include
contains(QT_VERSION, ^4\\..*) {
    CONFIG += qt qt4
    VERSION_MESSAGE = Qt 4
}
contains(QT_VERSION, ^5\\..*) {
    CONFIG += qt qt5
    VERSION_MESSAGE = Qt 5
}
CONFIG(debug, debug|release) {
    DEBUG_MESSAGE = debug
}
else {
    DEBUG_MESSAGE = release
}
console {
    QT -= core gui
    CONFIG -= qt qt4 qt5
    UI_MESSAGE = console mode
    VERSION_MESSAGE = No Qt
}
qtconsole {
    QT -= gui
    QT += xml
    UI_MESSAGE = console mode
}
!console:!qtconsole {
    CONFIG += gui
    UI_MESSAGE = GUI mode
}
gui {
    RESOURCES = src/GUI-QT/res/icons.qrc
    QT += network xml
    qt4:QT += webkit
    qt5:QT += widgets
    INCLUDEPATH += src/GUI-QT
    VPATH += src/GUI-QT
    win32:RC_FILE = windows/dream.rc
    macx:RC_FILE = src/GUI-QT/res/macicons.icns
    CONFIG += qwt
    UI_DIR = ui
    MOC_DIR = moc
}
message($$VERSION_MESSAGE $$DEBUG_MESSAGE $$UI_MESSAGE)
qt:multimedia {
    QT += multimedia
    CONFIG += sound
}
unix:!cross_compile {
    UNAME = $$system(uname -s)
    message(building on $$UNAME)
}
macx {
    QT_CONFIG -= no-pkg-config
    PKG_CONFIG = /usr/local/bin/pkg-config
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    QMAKE_LFLAGS += -F/usr/local/lib
    LIBS += -framework CoreFoundation -framework CoreServices
    LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
    CONFIG += pcap
    #packagesExist(libpulse) {
    #    CONFIG += pulseaudio sound
    #}
    packagesExist(sndfile) {
        CONFIG += sndfile
    }
    packagesExist(hamlib) {
        CONFIG += hamlib
    }
    packagesExist(speex) {
        CONFIG += libspeexdsp
    }
    packagesExist(fdk-aac) {
        CONFIG += fdk-aac
    }
}
linux-* {
    LIBS += -ldl -lrt
}
android {
    CONFIG += openSL sound
    SOURCES += src/android/platform_util.cpp src/android/soundin.cpp src/android/soundout.cpp
    HEADERS += src/android/platform_util.h src/android/soundin.h src/android/soundout.h
    QT -= webkitwidgets
    QT += svg
    LIBS += -lOpenSLES
}
unix {
    target.path = /usr/bin
    documentation.path = /usr/share/man/man1
    documentation.files = linux/dream.1
    INSTALLS += documentation
    INSTALLS += target
    CONFIG += link_pkgconfig
    tui:console {
      CONFIG += consoleio
    }
    LIBS += -lfftw3
    LIBS += -lz
    SOURCES += src/linux/Pacer.cpp
    DEFINES += HAVE_DLFCN_H \
           HAVE_MEMORY_H \
           HAVE_STDINT_H \
           HAVE_STDLIB_H
    DEFINES += HAVE_STRINGS_H \
           HAVE_STRING_H \
           STDC_HEADERS
    DEFINES += HAVE_INTTYPES_H \
           HAVE_STDINT_H \
           HAVE_SYS_STAT_H \
           HAVE_SYS_TYPES_H \
           HAVE_UNISTD_H
    DEFINES += HAVE_LIBZ
}
unix:!cross_compile {
    !sound {
         # check for pulseaudio before portaudio
         exists(/usr/include/pulse/pulseaudio.h) | \
         exists(/usr/local/include/pulse/pulseaudio.h) {
         #packagesExist(libpulse)
          CONFIG += pulseaudio sound
         }
         else {
           exists(/usr/include/portaudio.h) | \
           exists(/usr/local/include/portaudio.h) {
           #packagesExist(portaudio-2.0)
              CONFIG += portaudio sound
           }
        }
    }
    qt5|contains(QT_VERSION, ^4\\.8.*) {
      packagesExist(sndfile) {
        CONFIG += sndfile
      }
      packagesExist(hamlib) {
        CONFIG += hamlib
      }
      packagesExist(libgps) {
        CONFIG += gps
      }
      packagesExist(pcap) {
        CONFIG += pcap
      }
      exists(/usr/include/pcap) {
        CONFIG += pcap
      }
      packagesExist(opus) {
        CONFIG += opus
      }
      packagesExist(speexdsp) {
        CONFIG += speexdsp
      }
      packagesExist(fdk-aac) {
        CONFIG += fdk-aac
      }
    }
    else {
      exists(/usr/include/sndfile.h) | \
      exists(/usr/local/include/sndfile.h) {
        CONFIG += sndfile
      }
      exists(/usr/include/hamlib/rig.h) | \
      exists(/usr/local/include/hamlib/rig.h) {
          CONFIG += hamlib
      }
      exists(/usr/include/gps.h) | \
      exists(/usr/local/include/gps.h) {
        CONFIG += gps
      }
      exists(/usr/include/pcap.h) | \
      exists(/usr/local/include/pcap.h) {
        CONFIG += pcap
      }
      exists(/usr/include/opus/opus.h) | \
      exists(/usr/local/include/opus/opus.h) {
       CONFIG += opus
      }
      exists(/usr/include/speex/speex_preprocess.h) | \
      exists(/usr/local/include/speex/speex_preprocess.h) {
       CONFIG += speexdsp
      }
    }
}
win32 {
    LIBS += -llibfftw3-3
    !multimedia {
        exists($$OUT_PWD/include/portaudio.h) {
          CONFIG += portaudio sound
        }
        else {
          CONFIG += mmsystem sound
        }
    }
    exists($$PWD/include/speex/speex_preprocess.h) {
      CONFIG += speexdsp
    }
    exists($$PWD/include/hamlib/rig.h) {
      CONFIG += hamlib
    }
    exists($$PWD/include/pcap.h) {
      CONFIG += pcap
    }
    exists($$PWD/include/sndfile.h) {
        CONFIG += sndfile
    }
    exists($$PWD/include/opus/opus.h) {
        CONFIG += opus
    }
    exists($$PWD/include/fdk-aac/aacdecoder_lib.h) {
        CONFIG += fdk-aac
    }
    LIBS += -lsetupapi -lwsock32 -lws2_32 -lzlib -ladvapi32 -luser32
    DEFINES += HAVE_SETUPAPI HAVE_LIBZ _CRT_SECURE_NO_WARNINGS
    SOURCES += src/windows/Pacer.cpp src/windows/platform_util.cpp
    HEADERS += src/windows/platform_util.h
    msvc* {
        DEFINES += NOMINMAX
        QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:libcmt.lib
        QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:libcmtd.lib
        QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:libcmt.lib
    }
    g++ {
         DEFINES += HAVE_STDINT_H HAVE_LIBZ
         LIBS += -lz
    }
}
fdk-aac {
     DEFINES += HAVE_LIBFDK_AAC
     LIBS += -lfdk-aac
     HEADERS += src/sourcedecoders/fdk_aac_codec.h
     SOURCES += src/sourcedecoders/fdk_aac_codec.cpp
     message("with fdk-aac")
}
opus {
    DEFINES += HAVE_LIBOPUS USE_OPUS_LIBRARY
    unix:LIBS += -lopus
    win32 {
        CONFIG(debug, debug|release) {
            LIBS += -lopusd
        }
        CONFIG(release, debug|release) {
            LIBS += -lopus
        }
    }
     message("with opus")
}
sndfile {
     DEFINES += HAVE_LIBSNDFILE
     unix:LIBS += -lsndfile
     win32:LIBS += -llibsndfile-1
     message("with libsndfile")
}
speexdsp {
     DEFINES += HAVE_SPEEX
     unix:LIBS += -lspeexdsp
     win32:LIBS += -llibspeexdsp
     message("with libspeexdsp")
}
gps {
     DEFINES += HAVE_LIBGPS
     unix:LIBS += -lgps
     message("with gps")
}
pcap {
     DEFINES += HAVE_LIBPCAP
     unix:LIBS += -lpcap
     win32:LIBS += -lwpcap -lpacket
     message("with pcap")
}
hamlib {
     DEFINES += HAVE_LIBHAMLIB
     macx:LIBS += -framework IOKit
     unix:LIBS += -lhamlib
     win32:LIBS += -llibhamlib-2
     HEADERS += src/util/Hamlib.h
     SOURCES += src/util/Hamlib.cpp
     qt {
       HEADERS += src/util-QT/Rig.h
       SOURCES += src/util-QT/Rig.cpp
     }
     gui {
         HEADERS += src/GUI-QT/RigDlg.h
         SOURCES += src/GUI-QT/RigDlg.cpp
         FORMS += RigDlg.ui
     }
     message("with hamlib")
}
qwt {
    message("with Qwt")
    QT += svg
    macx {
        INCLUDEPATH += /usr/local/lib/qwt.framework/Headers
        LIBS += -framework qwt
    }
    win32 {
        INCLUDEPATH += $$PWD/include/qwt
        CONFIG(debug, debug|release) {
            LIBS += -lqwtd
        }
        CONFIG(release, debug|release) {
            LIBS += -lqwt
        }
    }
    unix!macx {
        # unix | win release
        LIBS += -lqwt-qt5
    }
    !crosscompile {
        exists(/usr/include/qwt/qwt.h) {
            INCLUDEPATH += /usr/include/qwt
        }
        exists(/usr/local/include/qwt/qwt.h) {
            INCLUDEPATH += /usr/local/include/qwt
        }
    }
}
alsa {
    DEFINES += USE_ALSA
    HEADERS += src/linux/soundsrc.h \
    src/linux/soundin.h \
    src/linux/soundout.h
    SOURCES += src/linux/alsa.cpp \
    src/linux/soundsrc.cpp
    message("with alsa")
}
mmsystem {
    HEADERS += src/windows/Sound.h
    SOURCES += src/windows/Sound.cpp
    LIBS += -lwinmm
    message("with mmsystem")
}
portaudio {
    DEFINES += USE_PORTAUDIO
    HEADERS += src/sound/pa_ringbuffer.h \
    src/sound/drm_portaudio.h
    SOURCES += src/sound/drm_portaudio.cpp \
    src/sound/pa_ringbuffer.c
    LIBS += -lportaudio
    unix:PKGCONFIG += portaudio-2.0
    message("with portaudio")
}
pulseaudio {
    DEFINES += USE_PULSEAUDIO
    HEADERS += src/sound/drm_pulseaudio.h
    SOURCES += src/sound/drm_pulseaudio.cpp
    unix {
        macx {
            INCLUDEPATH += /usr/local/Cellar/pulseaudio/12.2/include
            LIBS += -L/usr/local/Cellar/pulseaudio/12.2/lib -lpulse
        }
        else {
            PKGCONFIG += libpulse
        }
    }
    else {
        LIBS += -lpulse
    }
    message("with pulseaudio")
}
openSL {
    DEFINES += USE_OPENSL
    message("with openSL")
}
consoleio {
    DEFINES += USE_CONSOLEIO
    HEADERS += src/linux/ConsoleIO.h
    SOURCES += src/linux/ConsoleIO.cpp
    LIBS += -lpthread
    message("with terminal user interface")
}
HEADERS += \
    src/AMDemodulation.h \
    src/AMSSDemodulation.h \
    src/chanest/ChanEstTime.h \
    src/chanest/ChannelEstimation.h \
    src/chanest/IdealChannelEstimation.h \
    src/chanest/TimeLinear.h \
    src/chanest/TimeWiener.h \
    src/datadecoding/DABMOT.h \
    src/datadecoding/DataDecoder.h \
    src/datadecoding/DataEncoder.h \
    src/datadecoding/epgutil.h \
    src/datadecoding/Experiment.h \
    src/datadecoding/journaline/cpplog.h \
    src/datadecoding/journaline/crc_8_16.h \
    src/datadecoding/journaline/dabdatagroupdecoder.h \
    src/datadecoding/journaline/dabdgdec_impl.h \
    src/datadecoding/Journaline.h \
    src/datadecoding/journaline/log.h \
    src/datadecoding/journaline/newsobject.h \
    src/datadecoding/journaline/newssvcdec.h \
    src/datadecoding/journaline/newssvcdec_impl.h \
    src/datadecoding/journaline/NML.h \
    src/datadecoding/journaline/Splitter.h \
    src/datadecoding/MOTSlideShow.h \
    src/DataIO.h \
    src/drmchannel/ChannelSimulation.h \
    src/DrmReceiver.h \
    src/DRMSignalIO.h \
    src/DrmSimulation.h \
    src/DrmTransceiver.h \
    src/DrmTransmitter.h \
    src/FAC/FAC.h \
    src/GlobalDefinitions.h \
    src/InputResample.h \
    src/interleaver/BlockInterleaver.h \
    src/interleaver/SymbolInterleaver.h \
    src/IQInputFilter.h \
    src/matlib/Matlib.h \
    src/matlib/MatlibSigProToolbox.h \
    src/matlib/MatlibStdToolbox.h \
    src/MDI/AFPacketGenerator.h \
    src/MDI/MDIDecode.h \
    src/MDI/MDIDefinitions.h \
    src/MDI/MDIInBuffer.h \
    src/MDI/MDIRSCI.h \
    src/MDI/MDITagItemDecoders.h \
    src/MDI/MDITagItems.h \
    src/MDI/PacketInOut.h \
    src/MDI/PacketSinkFile.h \
    src/MDI/PacketSocket.h \
    src/MDI/PacketSourceFile.h \
    src/MDI/Pft.h \
    src/MDI/RCITagItems.h \
    src/MDI/RSCITagItemDecoders.h \
    src/MDI/RSISubscriber.h \
    src/MDI/TagItemDecoder.h \
    src/MDI/TagPacketDecoder.h \
    src/MDI/TagPacketDecoderMDI.h \
    src/MDI/TagPacketDecoderRSCIControl.h \
    src/MDI/TagPacketGenerator.h \
    src/mlc/BitInterleaver.h \
    src/mlc/ChannelCode.h \
    src/mlc/ConvEncoder.h \
    src/mlc/EnergyDispersal.h \
    src/mlc/Metric.h \
    src/mlc/MLC.h \
    src/mlc/QAMMapping.h \
    src/mlc/ViterbiDecoder.h \
    src/MSCMultiplexer.h \
    src/ofdmcellmapping/CellMappingTable.h \
    src/ofdmcellmapping/OFDMCellMapping.h \
    src/OFDM.h \
    src/Parameter.h \
    src/PlotManager.h \
    src/ReceptLog.h \
    src/resample/ResampleFilter.h \
    src/resample/Resample.h \
    src/Scheduler.h \
    src/SDC/SDC.h \
    src/SDC/audioparam.h \
    src/ServiceInformation.h \
    src/sound/audiofilein.h \
    src/sound/selectioninterface.h \
    src/sound/sound.h \
    src/sound/soundinterface.h \
    src/sound/soundnull.h \
    src/sourcedecoders/aac_codec.h \
    src/sourcedecoders/AudioCodec.h \
    src/sourcedecoders/AudioSourceDecoder.h \
    src/sourcedecoders/AudioSourceEncoder.h \
    src/sourcedecoders/null_codec.h \
    src/sourcedecoders/opus_codec.h \
    src/sync/FreqSyncAcq.h \
    src/sync/SyncUsingPil.h \
    src/sync/TimeSyncFilter.h \
    src/sync/TimeSync.h \
    src/sync/TimeSyncTrack.h \
    src/tables/TableAMSS.h \
    src/tables/TableCarMap.h \
    src/tables/TableDRMGlobal.h \
    src/tables/TableFAC.h \
    src/tables/TableMLC.h \
    src/tables/TableQAMMapping.h \
    src/tables/TableStations.h \
    src/TextMessage.h \
    src/UpsampleFilter.h \
    src/util/AudioFile.h \
    src/util/Buffer.h \
    src/util/CRC.h \
    src/util/FileTyper.h \
    src/util/LibraryLoader.h \
    src/util/LogPrint.h \
    src/util/Modul.h \
    src/util/Pacer.h \
    src/util/Reassemble.h \
    src/util/Settings.h \
    src/util/Utilities.h \
    src/util/Vector.h \
    src/Version.h \
    src/MSC/logicalframe.h \
    src/MSC/audiosuperframe.h \
    src/MSC/aacsuperframe.h \
    src/MSC/xheaacsuperframe.h \
    src/MSC/frameborderdescription.h \
    src/resample/speexresampler.h \
    src/resample/cspectrumresample.h \
    src/resample/caudioresample.h \
    src/sourcedecoders/reverb.h \
    src/sourcedecoders/caudioreverb.h \
    src/MSC/audiosuperframedecoder.h \
    src/sourcedecoders/audioframe.h \
    src/sourcedecoders/audioframedecoder.h \
    src/sourcedecoders/audioframereceiver.h \
    src/util-QT/rxsignaller.h \
    src/sound/audiooutput.h
SOURCES += \
    src/AMDemodulation.cpp \
    src/AMSSDemodulation.cpp \
    src/chanest/ChanEstTime.cpp \
    src/chanest/ChannelEstimation.cpp \
    src/chanest/IdealChannelEstimation.cpp \
    src/chanest/TimeLinear.cpp \
    src/chanest/TimeWiener.cpp \
    src/datadecoding/DABMOT.cpp \
    src/datadecoding/DataDecoder.cpp \
    src/datadecoding/DataEncoder.cpp \
    src/datadecoding/epgutil.cpp \
    src/datadecoding/Experiment.cpp \
    src/datadecoding/Journaline.cpp \
    src/datadecoding/journaline/crc_8_16.c \
    src/datadecoding/journaline/dabdgdec_impl.c \
    src/datadecoding/journaline/log.c \
    src/datadecoding/journaline/newsobject.cpp \
    src/datadecoding/journaline/newssvcdec_impl.cpp \
    src/datadecoding/journaline/NML.cpp \
    src/datadecoding/journaline/Splitter.cpp \
    src/datadecoding/MOTSlideShow.cpp \
    src/DataIO.cpp \
    src/drmchannel/ChannelSimulation.cpp \
    src/DrmReceiver.cpp \
    src/DRMSignalIO.cpp \
    src/DrmSimulation.cpp \
    src/DrmTransmitter.cpp \
    src/FAC/FAC.cpp \
    src/InputResample.cpp \
    src/interleaver/BlockInterleaver.cpp \
    src/interleaver/SymbolInterleaver.cpp \
    src/IQInputFilter.cpp \
    src/matlib/MatlibSigProToolbox.cpp \
    src/matlib/MatlibStdToolbox.cpp \
    src/MDI/AFPacketGenerator.cpp \
    src/MDI/MDIDecode.cpp \
    src/MDI/MDIInBuffer.cpp \
    src/MDI/MDIRSCI.cpp \
    src/MDI/MDITagItemDecoders.cpp \
    src/MDI/MDITagItems.cpp \
    src/MDI/PacketSinkFile.cpp \
    src/MDI/PacketSocket.cpp \
    src/MDI/PacketSourceFile.cpp \
    src/MDI/Pft.cpp \
    src/MDI/RCITagItems.cpp \
    src/MDI/RSCITagItemDecoders.cpp \
    src/MDI/RSISubscriber.cpp \
    src/MDI/TagPacketDecoder.cpp \
    src/MDI/TagPacketDecoderMDI.cpp \
    src/MDI/TagPacketDecoderRSCIControl.cpp \
    src/MDI/TagPacketGenerator.cpp \
    src/mlc/BitInterleaver.cpp \
    src/mlc/ChannelCode.cpp \
    src/mlc/ConvEncoder.cpp \
    src/mlc/EnergyDispersal.cpp \
    src/mlc/Metric.cpp \
    src/mlc/MLC.cpp \
    src/mlc/QAMMapping.cpp \
    src/mlc/TrellisUpdateMMX.cpp \
    src/mlc/TrellisUpdateSSE2.cpp \
    src/mlc/ViterbiDecoder.cpp \
    src/MSCMultiplexer.cpp \
    src/ofdmcellmapping/CellMappingTable.cpp \
    src/ofdmcellmapping/OFDMCellMapping.cpp \
    src/OFDM.cpp \
    src/Parameter.cpp \
    src/PlotManager.cpp \
    src/ReceptLog.cpp \
    src/resample/Resample.cpp \
    src/resample/ResampleFilter.cpp \
    src/Scheduler.cpp \
    src/SDC/SDCReceive.cpp \
    src/SDC/SDCTransmit.cpp \
    src/SDC/audioparam.cpp \
    src/ServiceInformation.cpp \
    src/SimulationParameters.cpp \
    src/sound/audiofilein.cpp \
    src/sourcedecoders/aac_codec.cpp \
    src/sourcedecoders/AudioCodec.cpp \
    src/sourcedecoders/AudioSourceDecoder.cpp \
    src/sourcedecoders/AudioSourceEncoder.cpp \
    src/sourcedecoders/null_codec.cpp \
    src/sourcedecoders/opus_codec.cpp \
    src/sync/FreqSyncAcq.cpp \
    src/sync/SyncUsingPil.cpp \
    src/sync/TimeSync.cpp \
    src/sync/TimeSyncFilter.cpp \
    src/sync/TimeSyncTrack.cpp \
    src/tables/TableCarMap.cpp \
    src/tables/TableFAC.cpp \
    src/tables/TableStations.cpp \
    src/TextMessage.cpp \
    src/util/CRC.cpp \
    src/util/FileTyper.cpp \
    src/util/LogPrint.cpp \
    src/util/Reassemble.cpp \
    src/util/Settings.cpp \
    src/util/Utilities.cpp \
    src/Version.cpp \
    src/sound/soundnull.cpp \
    src/DrmTransceiver.cpp \
    src/sound/soundinterface.cpp \
    src/sound/selectioninterface.cpp \
    src/MSC/logicalframe.cpp \
    src/MSC/audiosuperframe.cpp \
    src/MSC/aacsuperframe.cpp \
    src/MSC/xheaacsuperframe.cpp \
    src/MSC/frameborderdescription.cpp \
    src/resample/speexresampler.cpp \
    src/resample/cspectrumresample.cpp \
    src/resample/caudioresample.cpp \
    src/sourcedecoders/reverb.cpp \
    src/sourcedecoders/caudioreverb.cpp \
    src/MSC/audiosuperframedecoder.cpp \
    src/sourcedecoders/audioframe.cpp \
    src/sourcedecoders/audioframedecoder.cpp \
    src/sourcedecoders/audioframereceiver.cpp \
    src/util-QT/rxsignaller.cpp \
    src/sound/audiooutput.cpp
!console {
HEADERS += \
    src/GUI-QT/Logging.h \
    src/util-QT/epgdec.h \
    src/util-QT/EPG.h \
    src/util-QT/Util.h
SOURCES += \
    src/GUI-QT/Logging.cpp \
    src/util-QT/EPG.cpp \
    src/util-QT/epgdec.cpp \
    src/util-QT/Util.cpp
}
gui {
    contains(QT,webenginewidgets) {
        FORMS += BWSViewer.ui
        HEADERS += src/GUI-QT/BWSViewer.h
        SOURCES += src/GUI-QT/BWSViewer.cpp
        message('with BWS')
    }
FORMS += \
    AboutDlgbase.ui \
    AMMainWindow.ui \
    AMSSDlgbase.ui \
    AACCodecParams.ui \
    OpusCodecParams.ui \
    DRMMainWindow.ui \
    EPGDlgbase.ui \
    FMMainWindow.ui \
    GeneralSettingsDlgbase.ui \
    JLViewer.ui \
    LiveScheduleWindow.ui \
    MultSettingsDlgbase.ui \
    SlideShowViewer.ui \
    StationsDlgbase.ui \
    systemevalDlgbase.ui \
    TransmDlgbase.ui
HEADERS += \
    src/GUI-QT/AnalogDemDlg.h \
    src/GUI-QT/CWindow.h \
    src/GUI-QT/DialogUtil.h \
    src/GUI-QT/DRMPlot.h \
    src/GUI-QT/EPGDlg.h \
    src/GUI-QT/EvaluationDlg.h \
    src/GUI-QT/fdrmdialog.h \
    src/GUI-QT/fmdialog.h \
    src/GUI-QT/GeneralSettingsDlg.h \
    src/GUI-QT/jlbrowser.h \
    src/GUI-QT/JLViewer.h \
    src/GUI-QT/LiveScheduleDlg.h \
    src/GUI-QT/MultColorLED.h \
    src/GUI-QT/MultSettingsDlg.h \
    src/GUI-QT/Schedule.h \
    src/GUI-QT/SlideShowViewer.h \
    src/GUI-QT/SoundCardSelMenu.h \
    src/GUI-QT/StationsDlg.h \
    src/GUI-QT/OpusCodecParams.h \
    src/GUI-QT/AACCodecParams.h \
    src/GUI-QT/TransmDlg.h
SOURCES += \
    src/GUI-QT/AnalogDemDlg.cpp \
    src/GUI-QT/CWindow.cpp \
    src/GUI-QT/DialogUtil.cpp \
    src/GUI-QT/DRMPlot.cpp \
    src/GUI-QT/EPGDlg.cpp \
    src/GUI-QT/EvaluationDlg.cpp \
    src/GUI-QT/fdrmdialog.cpp \
    src/GUI-QT/fmdialog.cpp \
    src/GUI-QT/GeneralSettingsDlg.cpp \
    src/GUI-QT/jlbrowser.cpp \
    src/GUI-QT/JLViewer.cpp \
    src/GUI-QT/LiveScheduleDlg.cpp \
    src/GUI-QT/MultColorLED.cpp \
    src/GUI-QT/MultSettingsDlg.cpp \
    src/GUI-QT/Schedule.cpp \
    src/GUI-QT/SlideShowViewer.cpp \
    src/GUI-QT/SoundCardSelMenu.cpp \
    src/GUI-QT/StationsDlg.cpp \
    src/GUI-QT/OpusCodecParams.cpp \
    src/GUI-QT/AACCodecParams.cpp \
    src/GUI-QT/TransmDlg.cpp \
    src/GUI-QT/main.cpp
} else {
    SOURCES += src/main.cpp
}
!sound {
    error("no usable audio interface found - install pulseaudio or portaudio dev package")
}

OTHER_FILES += \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/res/values-es/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-et/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-it/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-pl/strings.xml \
    android/AndroidManifest.xml \
    windows/dream.iss

message('Qt modules $$QT')
