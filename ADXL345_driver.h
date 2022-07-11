#include <stdint.h>
#include <stdbool.h>


int Pinmux_Config();
int I2C0_Init();
void ADXL345_Init();
int ADXL345_REG_READ(uint8_t target_address, uint8_t* value);
int ADXL345_REG_WRITE(uint8_t target_address, uint8_t value);
int ADXL345_REG_MULTI_READ(uint8_t base_address, uint8_t values[], uint8_t len);
void ADXL345_XYZ_READ(int16_t szData16[3]);
bool ADXL345_IsDataReady();
