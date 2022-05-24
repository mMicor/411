/*******************************************************************************
* File Name: .h
* Version 3.50
*
* Description:
*  This file provides private constants and parameter values for the I2C
*  component.
*
* Note:
*
********************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_I2C_PVT_Master_H)
#define CY_I2C_PVT_Master_H

#include "Master.h"

#define Master_TIMEOUT_ENABLED_INC    (0u)
#if (0u != Master_TIMEOUT_ENABLED_INC)
    #include "Master_TMOUT.h"
#endif /* (0u != Master_TIMEOUT_ENABLED_INC) */


/**********************************
*   Variables with external linkage
**********************************/

extern Master_BACKUP_STRUCT Master_backup;

extern volatile uint8 Master_state;   /* Current state of I2C FSM */

/* Master variables */
#if (Master_MODE_MASTER_ENABLED)
    extern volatile uint8 Master_mstrStatus;   /* Master Status byte  */
    extern volatile uint8 Master_mstrControl;  /* Master Control byte */

    /* Transmit buffer variables */
    extern volatile uint8 * Master_mstrRdBufPtr;   /* Pointer to Master Read buffer */
    extern volatile uint8   Master_mstrRdBufSize;  /* Master Read buffer size       */
    extern volatile uint8   Master_mstrRdBufIndex; /* Master Read buffer Index      */

    /* Receive buffer variables */
    extern volatile uint8 * Master_mstrWrBufPtr;   /* Pointer to Master Write buffer */
    extern volatile uint8   Master_mstrWrBufSize;  /* Master Write buffer size       */
    extern volatile uint8   Master_mstrWrBufIndex; /* Master Write buffer Index      */

#endif /* (Master_MODE_MASTER_ENABLED) */

/* Slave variables */
#if (Master_MODE_SLAVE_ENABLED)
    extern volatile uint8 Master_slStatus;         /* Slave Status  */

    /* Transmit buffer variables */
    extern volatile uint8 * Master_slRdBufPtr;     /* Pointer to Transmit buffer  */
    extern volatile uint8   Master_slRdBufSize;    /* Slave Transmit buffer size  */
    extern volatile uint8   Master_slRdBufIndex;   /* Slave Transmit buffer Index */

    /* Receive buffer variables */
    extern volatile uint8 * Master_slWrBufPtr;     /* Pointer to Receive buffer  */
    extern volatile uint8   Master_slWrBufSize;    /* Slave Receive buffer size  */
    extern volatile uint8   Master_slWrBufIndex;   /* Slave Receive buffer Index */

    #if (Master_SW_ADRR_DECODE)
        extern volatile uint8 Master_slAddress;     /* Software address variable */
    #endif   /* (Master_SW_ADRR_DECODE) */

#endif /* (Master_MODE_SLAVE_ENABLED) */

#if ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED))
    extern volatile uint8 Master_wakeupSource;
#endif /* ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED)) */


#endif /* CY_I2C_PVT_Master_H */


/* [] END OF FILE */
