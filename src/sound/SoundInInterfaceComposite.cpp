#include "SoundInInterfaceComposite.h"

CSoundInInterfaceComposite::CSoundInInterfaceComposite():
    pCurrentInterface(nullptr)
{
}

CSoundInInterfaceComposite::~CSoundInInterfaceComposite()
{
    for (CSoundInInterface *pInterface: vecSoundInInterfaces)
        delete pInterface;

}

CTuner *  CSoundInInterfaceComposite::GetTuner()
{
    if (pCurrentInterface == nullptr)
        return nullptr;
    else {
        return pCurrentInterface->GetTuner();
    }
}

void CSoundInInterfaceComposite::AddInterface(CSoundInInterface *pInterface)
{
    vecSoundInInterfaces.push_back(pInterface);
}

bool CSoundInInterfaceComposite::Init(int iSampleRate, int iNewBufferSize, bool bNewBlocking)
{
    if (pCurrentInterface != nullptr)
        return pCurrentInterface->Init(iSampleRate, iNewBufferSize, bNewBlocking);
    else {
        return false;
    }

}

bool CSoundInInterfaceComposite::Read(CVector<short>& psData, CParameter& Parameters)
{
    if (pCurrentInterface != nullptr)
        return pCurrentInterface->Read(psData, Parameters);
    else
        return false;

}

void     CSoundInInterfaceComposite::Close()
{
    if (pCurrentInterface != nullptr)
        pCurrentInterface->Close();

}

std::string	CSoundInInterfaceComposite::GetVersion()
{
    if (pCurrentInterface != nullptr)
        return pCurrentInterface->GetVersion();
    else
        return "";

}

// CSelectionInterface methods
void CSoundInInterfaceComposite::Enumerate(std::vector<std::string>& names, std::vector<std::string>& descriptions, std::string& defaultDevice)
{
    names.clear();
    descriptions.clear();
    defaultDevice = "";

    for (CSoundInInterface *pInterface: vecSoundInInterfaces)
    {
        std::vector<std::string> interfaceNames;
        std::vector<std::string> interfaceDescriptions;
        std::string interfaceDefaultDevice;
        pInterface->Enumerate(interfaceNames, interfaceDescriptions, interfaceDefaultDevice);
        names.insert(names.end(), interfaceNames.begin(), interfaceNames.end());
        descriptions.insert(descriptions.end(),interfaceDescriptions.begin(), interfaceDescriptions.end());
        if (defaultDevice.length()==0)
            defaultDevice = interfaceDefaultDevice;
    }

}

std::string	CSoundInInterfaceComposite::GetDev()
{
    if (pCurrentInterface != nullptr)
        return pCurrentInterface->GetDev();
    else
        return "";

}

void CSoundInInterfaceComposite::SetDev(std::string sNewDev)
{
    for (CSoundInInterface *pInterface: vecSoundInInterfaces)
    {
        std::vector<std::string> interfaceNames;
        std::vector<std::string> interfaceDescriptions;
        std::string interfaceDefaultDevice;
        pInterface->Enumerate(interfaceNames, interfaceDescriptions, interfaceDefaultDevice);

        for (std::string candDev: interfaceNames)
        {
            if (candDev == sNewDev)
            {
                if (pCurrentInterface != pInterface)
                {
                    // Do something to pCurrentInterface. Close()? Stop()?
                }
                pCurrentInterface = pInterface;
                pInterface->SetDev(sNewDev);
                return;
            }
        }
    }


}
