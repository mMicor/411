/*******************************************************************************
* File Name: Master.c
* Version 3.50
*
* Description:
*  This file provides the source code of APIs for the I2C component.
*  The actual protocol and operation code resides in the interrupt service
*  routine file.
*
*******************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Master_PVT.h"


/**********************************
*      System variables
**********************************/

uint8 Master_initVar = 0u; /* Defines if component was initialized */

volatile uint8 Master_state;  /* Current state of I2C FSM */


/*******************************************************************************
* Function Name: Master_Init
********************************************************************************
*
* Summary:
*  Initializes I2C registers with initial values provided from customizer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_Init(void) 
{
#if (Master_FF_IMPLEMENTED)
    /* Configure fixed function block */
    Master_CFG_REG  = Master_DEFAULT_CFG;
    Master_XCFG_REG = Master_DEFAULT_XCFG;
    Master_ADDR_REG = Master_DEFAULT_ADDR;
    Master_CLKDIV1_REG = LO8(Master_DEFAULT_DIVIDE_FACTOR);
    Master_CLKDIV2_REG = HI8(Master_DEFAULT_DIVIDE_FACTOR);

#else
    uint8 intState;

    /* Configure control and interrupt sources */
    Master_CFG_REG      = Master_DEFAULT_CFG;
    Master_INT_MASK_REG = Master_DEFAULT_INT_MASK;

    /* Enable interrupt generation in status */
    intState = CyEnterCriticalSection();
    Master_INT_ENABLE_REG |= Master_INTR_ENABLE;
    CyExitCriticalSection(intState);

    /* Configure bit counter */
    #if (Master_MODE_SLAVE_ENABLED)
        Master_PERIOD_REG = Master_DEFAULT_PERIOD;
    #endif  /* (Master_MODE_SLAVE_ENABLED) */

    /* Configure clock generator */
    #if (Master_MODE_MASTER_ENABLED)
        Master_MCLK_PRD_REG = Master_DEFAULT_MCLK_PRD;
        Master_MCLK_CMP_REG = Master_DEFAULT_MCLK_CMP;
    #endif /* (Master_MODE_MASTER_ENABLED) */
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    Master_TimeoutInit();
#endif /* (Master_TIMEOUT_ENABLED) */

    /* Configure internal interrupt */
    CyIntDisable    (Master_ISR_NUMBER);
    CyIntSetPriority(Master_ISR_NUMBER, Master_ISR_PRIORITY);
    #if (Master_INTERN_I2C_INTR_HANDLER)
        (void) CyIntSetVector(Master_ISR_NUMBER, &Master_ISR);
    #endif /* (Master_INTERN_I2C_INTR_HANDLER) */

    /* Set FSM to default state */
    Master_state = Master_SM_IDLE;

#if (Master_MODE_SLAVE_ENABLED)
    /* Clear status and buffers index */
    Master_slStatus = 0u;
    Master_slRdBufIndex = 0u;
    Master_slWrBufIndex = 0u;

    /* Configure matched address */
    Master_SlaveSetAddress(Master_DEFAULT_ADDR);
#endif /* (Master_MODE_SLAVE_ENABLED) */

#if (Master_MODE_MASTER_ENABLED)
    /* Clear status and buffers index */
    Master_mstrStatus = 0u;
    Master_mstrRdBufIndex = 0u;
    Master_mstrWrBufIndex = 0u;
#endif /* (Master_MODE_MASTER_ENABLED) */
}


/*******************************************************************************
* Function Name: Master_Enable
********************************************************************************
*
* Summary:
*  Enables I2C operations.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  None.
*
*******************************************************************************/
void Master_Enable(void) 
{
#if (Master_FF_IMPLEMENTED)
    uint8 intState;

    /* Enable power to block */
    intState = CyEnterCriticalSection();
    Master_ACT_PWRMGR_REG  |= Master_ACT_PWR_EN;
    Master_STBY_PWRMGR_REG |= Master_STBY_PWR_EN;
    CyExitCriticalSection(intState);
#else
    #if (Master_MODE_SLAVE_ENABLED)
        /* Enable bit counter */
        uint8 intState = CyEnterCriticalSection();
        Master_COUNTER_AUX_CTL_REG |= Master_CNT7_ENABLE;
        CyExitCriticalSection(intState);
    #endif /* (Master_MODE_SLAVE_ENABLED) */

    /* Enable slave or master bits */
    Master_CFG_REG |= Master_ENABLE_MS;
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    Master_TimeoutEnable();
#endif /* (Master_TIMEOUT_ENABLED) */
}


/*******************************************************************************
* Function Name: Master_Start
********************************************************************************
*
* Summary:
*  Starts the I2C hardware. Enables Active mode power template bits or clock
*  gating as appropriate. It is required to be executed before I2C bus
*  operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  This component automatically enables its interrupt.  If I2C is enabled !
*  without the interrupt enabled, it can lock up the I2C bus.
*
* Global variables:
*  Master_initVar - This variable is used to check the initial
*                             configuration, modified on the first
*                             function call.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_Start(void) 
{
    if (0u == Master_initVar)
    {
        Master_Init();
        Master_initVar = 1u; /* Component initialized */
    }

    Master_Enable();
    Master_EnableInt();
}


/*******************************************************************************
* Function Name: Master_Stop
********************************************************************************
*
* Summary:
*  Disables I2C hardware and disables I2C interrupt. Disables Active mode power
*  template bits or clock gating as appropriate.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Master_Stop(void) 
{
    Master_DisableInt();

#if (Master_TIMEOUT_ENABLED)
    Master_TimeoutStop();
#endif  /* End (Master_TIMEOUT_ENABLED) */

#if (Master_FF_IMPLEMENTED)
    {
        uint8 intState;
        uint16 blockResetCycles;

        /* Store registers effected by block disable */
        Master_backup.addr    = Master_ADDR_REG;
        Master_backup.clkDiv1 = Master_CLKDIV1_REG;
        Master_backup.clkDiv2 = Master_CLKDIV2_REG;

        /* Calculate number of cycles to reset block */
        blockResetCycles = ((uint16) ((uint16) Master_CLKDIV2_REG << 8u) | Master_CLKDIV1_REG) + 1u;

        /* Disable block */
        Master_CFG_REG &= (uint8) ~Master_CFG_EN_SLAVE;
        /* Wait for block reset before disable power */
        CyDelayCycles((uint32) blockResetCycles);

        /* Disable power to block */
        intState = CyEnterCriticalSection();
        Master_ACT_PWRMGR_REG  &= (uint8) ~Master_ACT_PWR_EN;
        Master_STBY_PWRMGR_REG &= (uint8) ~Master_STBY_PWR_EN;
        CyExitCriticalSection(intState);

        /* Enable block */
        Master_CFG_REG |= (uint8) Master_ENABLE_MS;

        /* Restore registers effected by block disable. Ticket ID#198004 */
        Master_ADDR_REG    = Master_backup.addr;
        Master_ADDR_REG    = Master_backup.addr;
        Master_CLKDIV1_REG = Master_backup.clkDiv1;
        Master_CLKDIV2_REG = Master_backup.clkDiv2;
    }
#else

    /* Disable slave or master bits */
    Master_CFG_REG &= (uint8) ~Master_ENABLE_MS;

#if (Master_MODE_SLAVE_ENABLED)
    {
        /* Disable bit counter */
        uint8 intState = CyEnterCriticalSection();
        Master_COUNTER_AUX_CTL_REG &= (uint8) ~Master_CNT7_ENABLE;
        CyExitCriticalSection(intState);
    }
#endif /* (Master_MODE_SLAVE_ENABLED) */

    /* Clear interrupt source register */
    (void) Master_CSR_REG;
#endif /* (Master_FF_IMPLEMENTED) */

    /* Disable interrupt on stop (enabled by write transaction) */
    Master_DISABLE_INT_ON_STOP;
    Master_ClearPendingInt();

    /* Reset FSM to default state */
    Master_state = Master_SM_IDLE;

    /* Clear busy statuses */
#if (Master_MODE_SLAVE_ENABLED)
    Master_slStatus &= (uint8) ~(Master_SSTAT_RD_BUSY | Master_SSTAT_WR_BUSY);
#endif /* (Master_MODE_SLAVE_ENABLED) */
}


/* [] END OF FILE */
