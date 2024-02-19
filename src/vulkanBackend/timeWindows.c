#include "timeWindows.h"

uint64_t clockPerformanceFrequency = 0;

int initPerformanceFrequency()
{
    LARGE_INTEGER frequencyLargeInt;

    if (!QueryPerformanceFrequency(&frequencyLargeInt))
    {
        printf("Failed to Query Performance Frequency!\n");
        return 0;
    }

    clockPerformanceFrequency = frequencyLargeInt.QuadPart;

    printf("Queried Performance Frequency was: %llu\n", clockPerformanceFrequency);
    return 1;
}

float getTimeMSFloat()
{
    if(clockPerformanceFrequency == 0)
    {
        if (!(initPerformanceFrequency()))
        {
            return 0;
        }
    }

    LARGE_INTEGER counter;

    if (!(QueryPerformanceCounter(&counter)))
    {
        printf("Failed to Query Performance Counter!\n");
        return UINT64_MAX;
    }

    float timeInMS;

    timeInMS = counter.QuadPart / clockPerformanceFrequency;
    timeInMS += ((float)(counter.QuadPart % clockPerformanceFrequency)) / clockPerformanceFrequency;

    return timeInMS;

}

