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


#include "project.h"
#include "main.h"
#include <stdio.h>
#include <math.h>
#include "stdlib.h"



// Function declaration 

uint8 WriteByteToSlave(uint8 slaveAddress, uint8 registerAddress, uint8 wrData );                   // Write to a register on MPU
uint8 ReadBytesFromSlave(uint8 slaveAddress, uint8 registerAddress, uint16* wrData, uint8 cnt );     // Return bytes from MPU register
    
/* Global variable declaration */
    uint16 SensorDrop[12];  // For fetching data from MPU
    int16 AccelXYZ[3];     // XYZ for accelerometer
    int16 GyroXYZ[3];      // XYZ for gyroscope
    int16 accOff[3];
    int16 accPre[3] = {0};

    float acc[10] = {0};      // Accelerometer array
    
    float accAvg = 0;
    int accLim = 0;
    float sum = 0;
    int accPos = 0;
    
    float accCurrent = 0;
    float accTmp = 0;
    
    double roll , pitch;
    
    float phi = 0;
    float theta = 0;
    long int pre_ts=0;
    
    float phi_n_1, theta_n_1;
    float phi_dot, theta_dot, phi_quat, theta_quat;
    long gyro_x_cal, gyro_y_cal;
    
    float Q[4] = {1,0,0,0} ; 
    float Q_dot [4] ;
    float Q_pre [4] = {1,0,0,0};
    
    int filtered_roll = 0;
    int filtered_pitch = 0;
    
    int pitchLim = 0;
    int rollLim = 0;
    
    // Variables for UART tx
    int8 txX = 0;
    int8 txY = 0;
    int8 txZ = 0;
    int8 txRoll = 0;
    int8 txPitch = 0;
    
    uint32 sysStart=0, sysStop=0; // Variables for code timing
    

 CY_ISR(DATA_polling) // periodic polling interrupt
{
    ReadBytesFromSlave(MPU_ADDRESS,0x3b,SensorDrop,6); //read accel data from MPU to an array
    
    ReadBytesFromSlave(MPU_ADDRESS,0x43,&SensorDrop[6],6);
    
    for(uint8 i=0,j=0;i<=2;i++,j+=2)   // combines high and low bytes to one number
    {      
        AccelXYZ[i]=((SensorDrop[j]<< HIGH_BYTE_OFFSET)|(SensorDrop[j+LOW_BYTE_OFFSET]));
        
        GyroXYZ[i]=((SensorDrop[j+GYRO_ARRAY_OFFSET_H]<< HIGH_BYTE_OFFSET)|SensorDrop[j+GYRO_ARRAY_OFFSET_L]);
    }
    
    Polling_timer_ReadStatusRegister(); // reads the status register to clear interrupt
}    
    
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
        
    UART_Start();
    
    /* Initialization/startup code */
    Master_Start();                         // Initialize I2C component
    Poll_intr_StartEx(DATA_polling);        // ISR start call
    Polling_timer_Start();                  // Timer for periodic interrupt
    CySysTickStart();
    
    /* Infinite loop  */
    for(;;)
    {    
        CySysTickClear(); // Clock counter
        sysStart=CySysTickGetValue(); // Start counter for timing
        
        //__Fald detektions modul______________________________________________//
        // check if new data is available and run there is
        if(accPre[0] != AccelXYZ[0] || accPre[1] != AccelXYZ[1] || accPre[2] != AccelXYZ[2])
        {
            accPre[0] = AccelXYZ[0];
            accPre[1] = AccelXYZ[1];
            accPre[2] = AccelXYZ[2];
        
            // Caltulates the absolute power with Pythagoras theorem and saves it to array
            accTmp = acc[accPos];
            
            accCurrent = (sqrt(pow((float)AccelXYZ[0], 2) + pow((float)AccelXYZ[1], 2) + pow((float)AccelXYZ[2], 2))) / ACCELEROMETER_SENSITIVITY;
            acc[accPos] = accCurrent;
            
            sum = (sum + acc[accPos] - accTmp);
            
            accLim = (int)sum * 10; // Ganges med 10 for at kunne finde 5% afvig i grænserne
   
            accPos++;
            
            if (accPos > 9)
            {
                accPos = 0;
            }
      
            //__Orienterings modul______________________________________________//     
            float accelX = AccelXYZ[0];
            float accelY = AccelXYZ[1];
            float accelZ = AccelXYZ[2];
            float gyroX = GyroXYZ[0]/57.3;
            float gyroY = GyroXYZ[1]/57.3;
            float gyroZ = GyroXYZ[2]/57.3;
            
            // NORMALIZE ACCEL VALUES
            float naccel = sqrt(pow(accelX, 2) + pow(accelY, 2) + pow(accelZ, 2));
            accelX = accelX / naccel;
            accelY = accelY / naccel;
            accelZ = accelZ / naccel;
        
            // UART info to transmit
            txX = accelX;
            txY = accelY;
            txZ = accelZ;
            
            
            //  Euler angle from accel
            roll = atan2 (-accelX ,( sqrt((accelY * accelY) + (accelZ * accelZ))));
            pitch = atan2 (accelY ,( sqrt((accelX * accelX) + (accelZ * accelZ))));
            
            // 1st step sensor fusion using complimentary filter
            pitch = (0.98 * (pitch + gyroY * dt / 1000.0f) + 0.02 * (accelY)) * 57.3;
            roll =  (0.98 * (roll + gyroX * dt / 1000.0f) + 0.02 * (accelX)) * 57.3;
            
            // Calculate quaternions
            Q_dot[0] = -0.5* ((gyroX*Q_pre[1]) + (gyroY*Q_pre[2]) + (Q_pre[3]*gyroZ));
            Q_dot[1] =  0.5* ((gyroX*Q_pre[0]) + (gyroZ*Q_pre[2]) - (Q_pre[3]*gyroY));
            Q_dot[2] =  0.5* ((gyroY*Q_pre[0]) - (gyroZ*Q_pre[1]) + (Q_pre[3]*gyroX));
            Q_dot[3] =  0.5* ((gyroZ*Q_pre[0]) + (gyroY*Q_pre[1]) - (Q_pre[2]*gyroX));
            
            // Store quaternions
            Q[0] = Q_pre[0] + (Q_dot[0] * dt / 1000.0);
            Q_pre[0] = Q[0];
            Q[1] = Q_pre[1] + (Q_dot[1] * dt / 1000.0);
            Q_pre[1] = Q[1];
            Q[2] = Q_pre[2] + (Q_dot[2] * dt / 1000.0);
            Q_pre[2] = Q[2];
            Q[3] = Q_pre[3] + (Q_dot[3] * dt / 1000.0);
            Q_pre[3] = Q[3];

            // Normalize quaternions
            double n = (sqrt((Q[0]*Q[0]) + (Q[1]*Q[1]) + (Q[2]*Q[2]) + (Q[3]*Q[3])));
            float Q0 = Q[0] / n;
            float Q1 = Q[1] / n;
            float Q2 = Q[2] / n;
            float Q3 = Q[3] / n;
            
            // Quaternion angles
            phi_quat = atan2 (2*((Q0*Q1)+(Q2*Q3)), (0.5f-(Q1*Q1)-(Q2*Q2)));
            theta_quat = asin (2*((Q0*Q2)-(Q1*Q3)));
            
            // 2nd step sensor fusion using complimentary filter
            phi_quat = (0.98 * (phi_quat + gyroX * dt / 1000.0f) + 0.02 * (accelX)) * 57.3;
            theta_quat = (0.98 * (theta_quat + gyroY * dt / 1000.0f) + 0.02 * (accelY)) * 57.3;

            // Final filtration using complimentary filter
            filtered_roll = 0.99 * (roll + roll * dt / 1000.0f) + 0.01 * (phi_quat);
            filtered_pitch = 0.99 * (pitch + pitch * dt / 1000.0f) + 0.01 * (theta_quat);
            
            // Convert to absolute values
            rollLim = abs(filtered_roll);
            pitchLim = abs(filtered_pitch);
            
            // UART TX variables
            txRoll = rollLim;
            txPitch = pitchLim;
            
            // Offset to generate values form 0-180 instead of +-90
            if(accelZ < 0)
            {
                rollLim = 180 - abs(filtered_roll);
                pitchLim = 180 - abs(filtered_pitch);
            }
            
            // if the average acceleration is between the given values, activate actuator
            if(accLim < 5) // accLim avg of 10 datasets to minimize risk of false positive.
            {
                if(rollLim < 85 && pitchLim < 85)
                {
                    CyDelay(43);
                    LED_GREEN_Write(TRUE);
                }
                if(rollLim > 85 || pitchLim > 85)
                {
                    LED_GREEN_Write(FALSE);
                }
            } 
            if(accLim >= 5)
            {
                LED_GREEN_Write(FALSE);
            }
            
            sysStop=CySysTickGetValue(); // Stop counter for timing
            LED_BLUE_Write(TRUE);
        }        
        
        UART_PutChar(txX);
        UART_PutChar(txY);
        UART_PutChar(txZ);
        UART_PutChar(txRoll);
        UART_PutChar(txPitch);
        
        #ifdef I2C_DEBUG
        
            //CyDelay(500);
            //LED_BLUE_write(FALSE);
            //LED_GREEN_Write(FALSE);
            //LED_RED_Write(FALSE);
        
        #endif
    }    
}

// Funktion definitions ///////////////////////////////////////////////////////////////////////////////

 uint8 WriteByteToSlave(uint8 slaveAddress, uint8 registerAddress, uint8 wrData )
 {
    uint8 status = I2C_ERROR;     
       
    (void) Master_MasterClearStatus();          // clears master status register 
    
    if(Master_MSTR_NO_ERROR == Master_MasterSendStart(slaveAddress,I2C_WRITE))
    {
    Master_MasterWriteByte(registerAddress);    // write the register adress to slave
    Master_MasterWriteByte(wrData);             // Write the actual byte to register
    Master_MasterSendStop();                    // send stop condition
    
    status = I2C_SUCCES;
    }
    
    return (status);
 }

 uint8 ReadBytesFromSlave(uint8 slaveAddress, uint8 registerAddress, uint16 *rData, uint8 cnt)
 { 
   /* 
        This funktion Sends a single byte to a given register, overwriting the register
        of the MPU. The funktion has the following input: 
    
        uint8 slaveAddress      : The address of the slave/MPU
    
        uint8 registerAdresss   : Address of the register being written to
    
        uint8 wrData            : 8 bit byte written to register, given as a interger
    
        if the master starts communication the function will return a 1, and a 0 if failed
    
    */ 
    
   uint8 status = I2C_ERROR;

   (void) Master_MasterClearStatus();          // clears master status register 
    
   if(Master_MSTR_NO_ERROR == Master_MasterSendStart(slaveAddress,I2C_READ))
    {
        Master_MasterWriteByte(registerAddress);    // send the register address to the MPU 
        Master_MasterSendRestart(slaveAddress,1);   // send repeat start and read request
        while(cnt--)
        {
            if(cnt==0)                              // check if its the lasst byte sent, and send NAK
            {
                *rData++ = Master_MasterReadByte(Master_NAK_DATA);
            }
            else                                    // Reads a byte from slave and return ack to slave
            {
                *rData++ = Master_MasterReadByte(Master_ACK_DATA);
            }              
        }
        
        Master_MasterSendStop();                    // Sends the stop condition and 
        
        status = I2C_SUCCES;
    }
    

    
    
    return status;    
 }


//////////////////////////////////////////////////////////////////////////////////////////////////////


/* [] END OF FILE */
