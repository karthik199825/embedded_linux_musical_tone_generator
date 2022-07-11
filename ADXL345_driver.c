#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "address_map_arm.h"
#include "driver_functions.h"

int Pinmux_Config()
{
        void* SYSMGR_virtual_base;
        int fd = -1;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(SYSMGR_virtual_base = map_physical(fd, SYSMGR_BASE,SYSMGR_SPAN)))
                return (-1);

        volatile int* SYSMGR_GENERALIO7_virtual_ptr = (int*) SYSMGR_virtual_base + SYSMGR_GENERALIO7;
        volatile int* SYSMGR_GENERALIO8_virtual_ptr = (int*) SYSMGR_virtual_base + SYSMGR_GENERALIO8;
        volatile int* SYSMGR_I2C0USEFPGA_virtual_ptr = (int*) SYSMGR_virtual_base + SYSMGR_I2C0USEFPGA;

        *(SYSMGR_GENERALIO7_virtual_ptr) = 1;
        *(SYSMGR_GENERALIO8_virtual_ptr) = 1;
        *(SYSMGR_I2C0USEFPGA_virtual_ptr) = 0;

        unmap_physical(SYSMGR_virtual_base, SYSMGR_SPAN);
        close_physical(fd);
        return 0;
}

int I2C0_Init()
{
        void* I2C0_virtual_base;
        int fd = -1;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(I2C0_virtual_base = map_physical(fd, I2C0_BASE, I2C0_SPAN)))
                return (-1);

        volatile int* I2C0_virtual_ptr = (int*) I2C0_virtual_base;

        *(I2C0_virtual_ptr + I2C0_ENABLE) = 2;
        while(*(I2C0_virtual_ptr + I2C0_ENABLE_STATUS)&0x1) {}
         *(I2C0_virtual_ptr + I2C0_CON) = 0x65;
        *(I2C0_virtual_ptr + I2C0_TAR) = 0x53;

        *(I2C0_virtual_ptr + I2C0_FS_SCL_HCNT) = 60 + 30;
        *(I2C0_virtual_ptr + I2C0_FS_SCL_LCNT) = 130 + 30;

        *(I2C0_virtual_ptr + I2C0_ENABLE) = 1;
        while(!(*(I2C0_virtual_ptr + I2C0_ENABLE_STATUS)&0x1)) {}

        unmap_physical(I2C0_virtual_base, I2C0_SPAN);
        close_physical(fd);
        return 0;
}

int ADXL345_REG_READ(uint8_t target_address, uint8_t* value)
{
        void* I2C0_virtual_base;
        int fd = -1;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(I2C0_virtual_base = map_physical(fd, I2C0_BASE, I2C0_SPAN)))
                return (-1);

        volatile int* I2C0_virtual_ptr = (int*) I2C0_virtual_base;

        *(I2C0_virtual_ptr + I2C0_DATA_CMD) = target_address + 0x400;
        *(I2C0_virtual_ptr + I2C0_DATA_CMD) = 0x100;

        while(!(*(I2C0_virtual_ptr + I2C0_RXFLR))) {}

        *value = *(I2C0_virtual_ptr + I2C0_DATA_CMD);

        unmap_physical(I2C0_virtual_base, I2C0_SPAN);
        close_physical(fd);
        return 0;
}

int ADXL345_REG_WRITE(uint8_t target_address, uint8_t value)
{
        void* I2C0_virtual_base;
        int fd = -1;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(I2C0_virtual_base = map_physical(fd, I2C0_BASE, I2C0_SPAN)))
                return (-1);
        volatile int* I2C0_virtual_ptr = (int*) I2C0_virtual_base;

        *(I2C0_virtual_ptr + I2C0_DATA_CMD) = target_address + 0x400;
        *(I2C0_virtual_ptr + I2C0_DATA_CMD) = value;


        unmap_physical(I2C0_virtual_base, I2C0_SPAN);
        close_physical(fd);
        return 0;
}

int ADXL345_REG_MULTI_READ(uint8_t base_address, uint8_t values[], uint8_t len)
{
        void* I2C0_virtual_base;
        int fd = -1;
        if ((fd = open_physical(fd)) == -1)
                return (-1);
        if (!(I2C0_virtual_base = map_physical(fd, I2C0_BASE, I2C0_SPAN)))
                return (-1);

        volatile int* I2C0_virtual_ptr = (int*) I2C0_virtual_base;

        *(I2C0_virtual_ptr + I2C0_DATA_CMD) = base_address + 0x400;
        int i;
        for (i = 0; i < len; i++)
                *(I2C0_virtual_ptr + I2C0_DATA_CMD) = 0x100;

        int nth_byte = 0;
        while(len)
        {
                if (*(I2C0_virtual_ptr + I2C0_RXFLR) > 0)
                {
                        values[nth_byte] = *(I2C0_virtual_ptr + I2C0_DATA_CMD);
                        nth_byte++;
                        len--;
                }
        }

        unmap_physical(I2C0_virtual_base, I2C0_SPAN);
        close_physical(fd);
        return 0;
}

void ADXL345_XYZ_READ(int16_t szData16[3])
{
        uint8_t szData8[6];
        ADXL345_REG_MULTI_READ(0x32, (uint8_t*)&szData8, sizeof(szData8));

        szData16[0] = (szData8[1] << 8) | szData8[0];
        szData16[1] = (szData8[3] << 8) | szData8[2];
        szData16[2] = (szData8[5] << 8) | szData8[4];
}

bool ADXL345_IsDataReady()
{
        bool bReady = false;
        uint8_t data8;

        ADXL345_REG_READ(ADXL345_REG_INT_SOURCE, &data8);
        if (data8 & 0x10)
                bReady = true;

        return bReady;
}

void ADXL345_Init()
{
        ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, 0x0E);
        ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, 0x08);

        ADXL345_REG_WRITE(ADXL345_REG_THRESH_ACT, 0x04);
        ADXL345_REG_WRITE(ADXL345_REG_THRESH_INACT, 0x02);
        ADXL345_REG_WRITE(ADXL345_REG_TIME_INACT, 0x02);
        ADXL345_REG_WRITE(ADXL345_REG_ACT_INACT_CTL, 0xFF);

        ADXL345_REG_WRITE(ADXL345_REG_INT_ENABLE, 0x18);
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, 0x00);
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, 0x08);
}
