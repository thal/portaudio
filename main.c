#include "portaudio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CHECK_ERR(E) if((err = (E)) != paNoError)goto error;


int process( const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    if(statusFlags) {
        printf("0x%lx\n", statusFlags);
    }
    const float *src = input;
    float *dst = output;

    static double theta = 0.0;
    const PaStreamParameters *outparams = userData;
    for(int i = 0; i < frameCount; i++) {
        theta += M_PI / 100;
        for(int j = 0; j < outparams->channelCount; j++)
        {
          // Uncomment for loopback
          //dst[i*outparams->channelCount + j] = src[i] * ((j%2) ? -1 : 1);

          // Uncomment for sine
          double samp = sin(theta);
          dst[i*outparams->channelCount + j] = samp * -.5 * ((j%2) ? -1 : 1);
        }
    }
    return 0;
}

int main()
{
    PaError err = paNoError;
    CHECK_ERR(Pa_Initialize());

    int pulsein = -1, pulseout = -1;

    int numDevices = Pa_GetDeviceCount();
    if(numDevices < 0)
    {
        err = numDevices;
        goto error;
    }

    for(int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if(0 == strcmp(info->name, "Default Sink")) {
            pulseout = i;
        }
        if(0 == strcmp(info->name, "Default Source")) {
            pulsein = i;
        }
        printf("%s\n", info->name);
    }

    if(pulsein == -1 && pulseout == -1)
    {
        printf("Could not find pulse device\n");
        return -1;
    }

    PaStream *stream;
    PaStreamParameters inparams =
    {
        .device = pulsein,
        .channelCount = 1,
        .sampleFormat = paFloat32,
        .suggestedLatency = Pa_GetDeviceInfo(pulsein)->defaultLowInputLatency
    };
    PaStreamParameters outparams =
    {
        .device = pulseout,
        .channelCount = 2,
        .sampleFormat = paFloat32,
        .suggestedLatency = Pa_GetDeviceInfo(pulseout)->defaultLowOutputLatency
    };
    CHECK_ERR(Pa_OpenStream(&stream,
                &inparams,
                &outparams,
                48000,
                256,
                0,
                process,
                &outparams));

    CHECK_ERR(Pa_StartStream(stream));

    sleep(100);

error:
    if(err != paNoError)
        printf("%d\n", err);
    return 0;
}
