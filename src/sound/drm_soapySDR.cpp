#include "drm_soapySDR.h"
#include <cstdio>	//standard output
#include <cstdlib>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >

#include <iostream>

#include "../Parameter.h"

// Constructor(s)
CSoapySDRIn::CSoapySDRIn() : currentDev(""), iSampleRate(96000), iBufferSize(0), iFrequency(0), pDevice(nullptr), pStream(nullptr)
{
    //pFile = fopen("raw_input.iq", "w");
}

CSoapySDRIn::~CSoapySDRIn()
{
    //fclose(pFile);
}

// CSoundInInterface methods
bool CSoapySDRIn::Init(int iNewSampleRate, int iNewBufferSize, bool bNewBlocking)
{
    SoapySDR::KwargsList results = SoapySDR::Device::enumerate();


    std::vector<std::string> names;
    std::vector<std::string> descriptions;
    std::string defaultDevice;
    Enumerate(names, descriptions, defaultDevice);

    unsigned int deviceIndex= 0;
    for (unsigned int i=0; i<names.size(); i++)
    {
        if (names[i] == currentDev)
            deviceIndex = i;
    }
    SoapySDR::Kwargs args = results[deviceIndex];
    pDevice = SoapySDR::Device::make(args);

    if (!pDevice)
    {
        // flag error somehow
        return false; // NB return value is "changed" not "success"
    }

    iSampleRate = 96000; //iNewSampleRate;
    pDevice->setSampleRate(SOAPY_SDR_RX, 0, iSampleRate);

    pDevice->setFrequency(SOAPY_SDR_RX, 0, iFrequency);

    iBufferSize = iNewBufferSize;

    (void) bNewBlocking; // not sure if or how this can be used

    std::string mapping = pDevice->getFrontendMapping(SOAPY_SDR_RX);
    fprintf(stderr, "front end mapping: %s\n", mapping.c_str());

    pStream = pDevice->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CS16);

    int res = pDevice->activateStream(pStream);
    if (res)
        fprintf(stderr, "Activating stream failed");
    else
        fprintf(stderr, "Activating stream succeeded\n");
    return true;

}

bool CSoapySDRIn::Read(CVector<short>& psData, CParameter &Parameter)
{
    void *buffs[] = {&psData[0]};
    int flags;
    long long time_ns=0;
    int elementsRemaining = iBufferSize/2;
    while (elementsRemaining>0)
    {
        buffs[0]={&psData[iBufferSize-2*elementsRemaining]};
        int elementsRead = pDevice->readStream(pStream, buffs, size_t(elementsRemaining), flags, time_ns, 1e9); // divide by 2 because complex
        elementsRemaining -= elementsRead;
    }
    //fwrite(&psData[0], 2, size_t(iBufferSize), pFile);

    _REAL r = - pDevice->getGain(SOAPY_SDR_RX, 0); //negative because the amount of gain should be subtracted from the signal level to get the input power
    Parameter.Lock();
    r += Parameter.rSigStrengthCorrection;
    Parameter.SigStrstat.addSample(r);
    Parameter.Unlock();
    //emit sigstr(r);

    return false; // false=OK, true=bad
}

void CSoapySDRIn::Close()
{
    fprintf(stderr, "Closing device\n");
    if (pDevice)
    {
       pDevice->deactivateStream(pStream, 0,0);
       pDevice->closeStream(pStream);
       SoapySDR::Device::unmake(pDevice);
       pDevice = nullptr;
    }

}

std::string	CSoapySDRIn::GetVersion()
{
    return "SoapySDR";
}

// CSelectionInterface methods
void CSoapySDRIn::Enumerate(std::vector<std::string>& names, std::vector<std::string>& descriptions, std::string& defaultDevice)
{
    // 0. enumerate devices (list all devices' information)
    SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
    SoapySDR::Kwargs::iterator it;

    names.clear();
    for( unsigned int i = 0; i < results.size(); ++i)
    {
        std::stringstream ss;
        ss << "SoapySDR device #" << i <<": ";
        names.push_back(ss.str());

        ss.str("");

        for( it = results[i].begin(); it != results[i].end(); ++it)
        {
            ss << it->first << "=" <<it->second << " ";
        }
        descriptions.push_back(ss.str());
    }
    defaultDevice = names[0];
}

std::string	CSoapySDRIn::GetDev()
{
    return currentDev;

}

void CSoapySDRIn::SetDev(std::string sNewDev)
{
    currentDev = sNewDev;
}

void CSoapySDRIn::SetFrequency(int freq)
{
    iFrequency = freq * 1000;
    fprintf(stderr, "CSoapySDRIn::SetFrequency(%d)\n", iFrequency);
    if (pDevice)
        pDevice->setFrequency(SOAPY_SDR_RX, 0, iFrequency);
}
