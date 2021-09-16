#include "MLX90640.h"

/*!
 *    @brief  Instantiates a new MLX90640 class
 */
void MLX90640(void) {}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_addr
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
boolean begin() {
  mI2C_Start();

  // wire->setClock(400000); // Speed it up, lots to read :)
  MLX90640_I2CRead(MLX90640_I2CADDR_DEFAULT, MLX90640_DEVICEID1, 3, serialNumber);

  uint16_t eeMLX90640[832];
  if (MLX90640_DumpEE(0, eeMLX90640) != 0) {
    return false;
  }

  MLX90640_ExtractParameters(eeMLX90640, &_params);
  // whew!
  return true;
}

/*!
 *    @brief  Read nMemAddressRead words from I2C startAddress into data
 *    @param  slaveAddr Not used - kept to maintain backcompatible API
 *    @param  startAddress I2C memory address to start reading
 *    @param  nMemAddressRead 16-bit words to read
 *    @param  data Location to place data read
 *    @return 0 on success
 */
int MLX90640_I2CRead(uint8_t slaveAddr,
                                        uint16_t startAddress,
                                        uint16_t nMemAddressRead,
                                        uint16_t *data) {
  uint8_t cmd[2];

  while (nMemAddressRead > 0) {
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0x00FF;
    
    mI2C_MasterSendStart(slaveAddr, 0, 10);
    mI2C_MasterWriteByte(cmd[0], 10);
    mI2C_MasterWriteByte(cmd[1], 10);
    mI2C_MasterSendReStart(slaveAddr, 1, 10);
    uint8 data8;
    mI2C_MasterReadByte(CY_SCB_I2C_ACK, &data8, 10);
    *data = data8 << 8;
    mI2C_MasterReadByte(CY_SCB_I2C_NAK, &data8, 10);
    *data |= data8;
    
    // advance buffer
    data++;
    // advance address
    startAddress++;
    // reduce remaining to read
    nMemAddressRead--;
  }
  // success!
  return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr,
                                         uint16_t writeAddress, uint16_t data) {
  uint8_t cmd[4];
  uint16_t dataCheck = 0;

  cmd[0] = writeAddress >> 8;
  cmd[1] = writeAddress & 0x00FF;
  cmd[2] = data >> 8;
  cmd[3] = data & 0x00FF;

  mI2C_MasterSendStart(slaveAddr, 0, 10);
  for(int i = 0; i < 4; i++)
    mI2C_MasterWriteByte(cmd[i], 10);
  CyDelay(1);

  if (MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck) != 0) {
    return -1;
  }

  // check echo
  if (dataCheck != data) {
    return -2;
  }
  // OK!
  return 0;
}

/*!
 *    @brief Get the frame-read mode
 *    @return Chess or interleaved mode
 */
mlx90640_mode_t getMode(void) {
  return (mlx90640_mode_t)MLX90640_GetCurMode(0);
}

/*!
 *    @brief Set the frame-read mode
 *    @param mode Chess or interleaved mode
 */
void setMode(mlx90640_mode_t mode) {
  if (mode == MLX90640_CHESS) {
    MLX90640_SetChessMode(0);
  } else {
    MLX90640_SetInterleavedMode(0);
  }
}

/*!
 *    @brief  Get resolution for temperature precision
 *    @returns The desired resolution (bits)
 */
mlx90640_resolution_t getResolution(void) {
  return (mlx90640_resolution_t)MLX90640_GetCurResolution(0);
}

/*!
 *    @brief  Set resolution for temperature precision
 *    @param res The desired resolution (bits)
 */
void setResolution(mlx90640_resolution_t res) {
  MLX90640_SetResolution(0, (int)res);
}

/*!
 *    @brief  Get max refresh rate
 *    @returns How many pages per second to read (2 pages per frame)
 */
mlx90640_refreshrate_t getRefreshRate(void) {
  return (mlx90640_refreshrate_t)MLX90640_GetRefreshRate(0);
}

/*!
 *    @brief  Set max refresh rate - too fast and we can't read the
 *    the pages in time, start low and then increment while speeding
 *    up I2C!
 *    @param rate How many pages per second to read (2 pages per frame)
 */
void setRefreshRate(mlx90640_refreshrate_t rate) {
  MLX90640_SetRefreshRate(0, (int)rate);
}

/*!
 *    @brief  Read 2 pages, calculate temperatures and place into framebuf
 *    @param  framebuf 24*32 floating point memory buffer
 *    @return 0 on success
 */
int getFrame(float *framebuf) {
  float emissivity = 0.95;
  float tr = 23.15;
  uint16_t mlx90640Frame[834];
  int status;

  for (uint8_t page = 0; page < 2; page++) {
    status = MLX90640_GetFrameData(0, mlx90640Frame);

    if (status < 0) {
      return status;
    }

    tr = MLX90640_GetTa(mlx90640Frame, &_params) -
         OPENAIR_TA_SHIFT; // For a MLX90640 in the open air the shift is -8
                           // degC.
    MLX90640_CalculateTo(mlx90640Frame, &_params, emissivity, tr, framebuf);
  }
  return 0;
}
