#include <stdio.h>
#include <math.h>

#include "address_map_arm.h"
#include "driver_functions.h"

#define SAMPLE_RATE 48000
#define PI 3.141592
#define FACTOR 2


int Audio_Init()
{
        void* LW_BRIDGE_virtual_base;
        int fd = -1;

        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(LW_BRIDGE_virtual_base = map_physical(fd,LW_BRIDGE_BASE,LW_BRIDGE_SPAN)))
                return (-1);
        volatile int* AUDIO_virtual_ptr = (int*) (LW_BRIDGE_virtual_base + AUDIO_BASE);

        *(AUDIO_virtual_ptr) = 0xC;
        *(AUDIO_virtual_ptr) = 0x0;

        unmap_physical(LW_BRIDGE_virtual_base, LW_BRIDGE_SPAN);
        close_physical(fd);
        return 0;
}

int write_to_audio_port(double freq)
{
        int fd = -1;
        void* LW_BRIDGE_virtual_base;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(LW_BRIDGE_virtual_base = map_physical(fd,LW_BRIDGE_BASE,LW_BRIDGE_SPAN)))
                return (-1);

        volatile int* AUDIO_virtual_ptr = (int*) (LW_BRIDGE_virtual_base + AUDIO_BASE);

        int nth_sample;
        //double freq = 261.626;

        int vol = 0x7FFFFFFF;

        for (nth_sample = 0; nth_sample < SAMPLE_RATE/FACTOR; nth_sample++)
        {
                *(AUDIO_virtual_ptr + 2) = vol*sin(nth_sample*freq*2*PI/SAMPLE_RATE);
                *(AUDIO_virtual_ptr + 3) = vol*sin(nth_sample*freq*2*PI/SAMPLE_RATE);
        }

        unmap_physical(LW_BRIDGE_virtual_base, LW_BRIDGE_SPAN);
        close_physical(fd);
        return 0;
}
