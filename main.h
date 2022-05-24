/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#if !defined(MAIN_H)

    #define MAIN_H
    
#endif

/***************************************
*            Constants
****************************************/

// I2C relevant constants
#define I2C_WRITE  (1)
#define I2C_READ   (0)

#define I2C_ERROR  (0u)
#define I2C_SUCCES (1u)
#define I2C_READ_BUFFER_MISMATCH (3)

// MPU-9250 config
#define MPU_ADDRESS         (0x68u)
#define ACCEL_START         (0x3b)
#define GYRO_ARRAY_OFFSET_H (6u)
#define GYRO_ARRAY_OFFSET_L (7u)
#define HIGH_BYTE_OFFSET    (8)
#define LOW_BYTE_OFFSET     (1) 
#define ACCELEROMETER_SENSITIVITY   (16384.0)   // 32768/2g
#define GYROSCOPE_SENSITIVITY       (32.8)      // 32768/1000dps
#define M_PI (3.14)	                    // Pi
#define dt (0.01)							        // 10 ms sample rate!



// system general 
#define TRUE       (1u)
#define FALSE      (0u)


// Debugging
 #define I2C_DEBUG
// #define TIMER_DEBUG

/* [] END OF FILE */
