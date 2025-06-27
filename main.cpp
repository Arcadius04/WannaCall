#include "wannacall.h"

#include <QApplication>
#include <QAudioDevice>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <QAudioSink>
#include <QMediaDevices>
#include <QCryptographicHash>

#include <windows.h>
#include <iostream>
//#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <set>

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
if ((punk) != NULL)  \
    { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

HRESULT RecordAudioStream()
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pwfx = NULL;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE *pData;
    DWORD flags;
    QAudioSink* sink = nullptr;
    for(QAudioDevice dev : QMediaDevices::audioOutputs()){
        if(dev.description().toLower().contains("tv")){
            sink = new QAudioSink(dev,dev.preferredFormat());
        }
    }
    if(sink == nullptr){
        sink = new QAudioSink(QMediaDevices::defaultAudioOutput(), QMediaDevices::defaultAudioOutput().preferredFormat());
    }
    QIODevice* outDev = nullptr;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);
    EXIT_ON_ERROR(hr)

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(
        IID_IAudioCaptureClient,
        (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    // Notify the audio sink which format to use.
    //hr = pMySink->SetFormat(pwfx);
    outDev = sink->start();
    //EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
                        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)

    // Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
        // Sleep for half the buffer duration.
        //Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags, NULL, NULL);
            EXIT_ON_ERROR(hr)

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }

            // Copy the available capture data to the audio sink.
            //hr = pMySink->CopyData(pData, numFramesAvailable, &bDone);
            outDev->write((const char*)pData, numFramesAvailable * pwfx->nBlockAlign);
            qDebug() << pData;
            //EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)
        }
    }

    hr = pAudioClient->Stop();  // Stop recording.
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)

    return hr;
}

int main(int argc, char *argv[])
{
    int ret;
    try{
    QApplication a(argc, argv);
        RecordAudioStream();
    /*QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);

    QAudioDevice input;
    for(QAudioDevice dev : QMediaDevices::audioInputs()){
        if(dev.description().toLower().contains("boom")){
            input = dev;
        }
    }
    if(!input.isFormatSupported(format)){
        qDebug() << "Input doesn't support format";
    }
    QAudioSource source(input,format);
    qDebug() << input.description();

    QAudioDevice output;
    for(QAudioDevice dev : QMediaDevices::audioOutputs()){
        if(dev.description().toLower().contains("boom")){
            output = dev;
        }
    }
    if(!output.isFormatSupported(format)){
        qDebug() << "Output doesn't support format";
    }
    QAudioSink sink(output,format);
    qDebug() << output.description();

    QIODevice* sourceDev = source.start();
    QIODevice* sinkDev = sink.start();

    QObject::connect(sourceDev, &QIODevice::readyRead,[&](){
        QByteArray audioData = sourceDev->readAll();
        if (!audioData.isEmpty()) {
            qint64 written = sinkDev->write(audioData);
            if (written == -1) {
                qCritical() << "Failed to write audio data to sink.";
            }
        }
    });
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, []() {
        qDebug() << "Application is running...";
    });*/
    //timer.start(1000);
    qDebug() << "Creating Window...";
    //WannaCall w;
    qDebug() << "Showing Window...";
    //w.show();
    int ret = a.exec();
    qDebug() << ret;
    } catch (const std::exception &e) {
        qDebug() << "Exception occurred:" << e.what();
        return -1;
    }
    return ret;
}
