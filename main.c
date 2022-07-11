#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <linux/input.h>

#include "ADXL345_driver.h"
#include "audio_driver.h"

void inthand(int signum);

volatile double freq = 261.626;
pthread_mutex_t mutex_freq;
pthread_t tid;

volatile sig_atomic_t stop = 0;

void* audio_thread()
{
        int err;
        if ((err = Audio_Init()) == -1)
                printf("ERROR: Audio_Init() failed ... \n");

        double sampled_freq;
        while(1)
        {
                pthread_mutex_lock(&mutex_freq);
                sampled_freq = freq;
                pthread_mutex_unlock(&mutex_freq);
                write_to_audio_port(sampled_freq);
        if((err = write_to_audio_port()) == -1)
                printf("ERROR: write_to_audio_port() failed ... \n");
        }
}

int main(void)
{
        uint8_t devid;
        int16_t mg_per_lsb = 4;
        int16_t XYZ[3];

        int err;
        signal(SIGINT, inthand);

        if ((err = pthread_create(&tid, NULL, &audio_thread, NULL)) !=0)
                printf("pthread_create failed");

        if ((err = Pinmux_Config()) == -1)
        {
                printf("ERROR: Pinmux_Config() failed ... \n");
                return (-1);
        }
        if ((err = I2C0_Init()) == -1)
        {
                printf("ERROR: I2C0_Init() failed ... \n");
                return (-1);
        }
        ADXL345_REG_READ(0x00, &devid);
        if (devid == 0xE5)
        {
                printf("devid recognized \n");
                ADXL345_Init();
                while(!stop)
                {
                        if (ADXL345_IsDataReady())
                        {
                                ADXL345_XYZ_READ(XYZ);
                                double X_freq = (XYZ[0]/100)*(XYZ[0]/100);
                                double Y_freq = (XYZ[1]/100)*(XYZ[1]/100);
                                double Z_freq = (XYZ[2]/100)*(XYZ[2]/100);
                                double Magnitude = sqrt(X_freq + Y_freq + Z_freq);
                                pthread_mutex_lock(&mutex_freq);
                                freq = Magnitude;
                                pthread_mutex_unlock(&mutex_freq);
                                printf("X=%d mg, Y=%d mg, Z=%d mg\n", XYZ[0]*mg_per_lsb, XYZ[1]*mg_per_lsb, XYZ[2]*mg_per_lsb);
                        }
                }
        }
        else
                printf("incorrect device ID\n");

        return 0;
}

void inthand(int signum)
{
        stop = 1;
        pthread_cancel(tid);
        pthread_join(tid, NULL);
}
