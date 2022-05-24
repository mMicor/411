/*******************************************************************************
* File Name: Master_PM.c
* Version 3.50
*
* Description:
*  This file provides low power mode APIs for the I2C component.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Master_PVT.h"

Master_BACKUP_STRUCT Master_backup =
{
    Master_DISABLE,

#if (Master_FF_IMPLEMENTED)
    Master_DEFAULT_XCFG,
    Master_DEFAULT_CFG,
    Master_DEFAULT_ADDR,
    LO8(Master_DEFAULT_DIVIDE_FACTOR),
    HI8(Master_DEFAULT_DIVIDE_FACTOR),
#else  /* (Master_UDB_IMPLEMENTED) */
    Master_DEFAULT_CFG,
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    Master_DEFAULT_TMOUT_PERIOD,
    Master_DEFAULT_TMOUT_INTR_MASK,
#endif /* (Master_TIMEOUT_ENABLED) */
};

#if ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED))
    volatile uint8 Master_wakeupSource;
#endif /* ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED)) */


/*******************************************************************************
* Function Name: Master_SaveConfig
********************************************************************************
*
* Summary:
*  The Enable wakeup from Sleep Mode selection influences this function
*  implementation:
*   Unchecked: Stores the component non-retention configuration registers.
*   Checked:   Disables the master, if it was enabled before, and enables
*              backup regulator of the I2C hardware. If a transaction intended
*              for component executes during this function call, it waits until
*              the current transaction is completed and I2C hardware is ready
*              to enter sleep mode. All subsequent I2C traffic is NAKed until
*              the device is put into sleep mode.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Master_backup - The global variable used to save the component
*                            configuration and non-retention registers before
*                            entering the sleep mode.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_SaveConfig(void) 
{
#if (Master_FF_IMPLEMENTED)
    #if (Master_WAKEUP_ENABLED)
        uint8 intState;
    #endif /* (Master_WAKEUP_ENABLED) */

    /* Store registers before enter low power mode */
    Master_backup.cfg     = Master_CFG_REG;
    Master_backup.xcfg    = Master_XCFG_REG;
    Master_backup.addr    = Master_ADDR_REG;
    Master_backup.clkDiv1 = Master_CLKDIV1_REG;
    Master_backup.clkDiv2 = Master_CLKDIV2_REG;

#if (Master_WAKEUP_ENABLED)
    /* Disable master */
    Master_CFG_REG &= (uint8) ~Master_ENABLE_MASTER;

    /* Enable backup regulator to keep block powered in low power mode */
    intState = CyEnterCriticalSection();
    Master_PWRSYS_CR1_REG |= Master_PWRSYS_CR1_I2C_REG_BACKUP;
    CyExitCriticalSection(intState);

    /* 1) Set force NACK to ignore I2C transactions;
    *  2) Wait unti I2C is ready go to Sleep; !!
    *  3) These bits are cleared on wake up.
    */
    /* Wait when block is ready for sleep */
    Master_XCFG_REG |= Master_XCFG_FORCE_NACK;
    while (0u == (Master_XCFG_REG & Master_XCFG_RDY_TO_SLEEP))
    {
    }

    /* Setup wakeup interrupt */
    Master_DisableInt();
    (void) CyIntSetVector(Master_ISR_NUMBER, &Master_WAKEUP_ISR);
    Master_wakeupSource = 0u;
    Master_EnableInt();
#endif /* (Master_WAKEUP_ENABLED) */

#else
    /* Store only address match bit */
    Master_backup.control = (Master_CFG_REG & Master_CTRL_ANY_ADDRESS_MASK);
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    Master_TimeoutSaveConfig();
#endif /* (Master_TIMEOUT_ENABLED) */
}


/*******************************************************************************
* Function Name: Master_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred method to prepare the component before device enters
*  sleep mode. The Enable wakeup from Sleep Mode selection influences this
*  function implementation:
*   Unchecked: Checks current I2C component state, saves it, and disables the
*              component by calling I2C_Stop() if it is currently enabled.
*              I2C_SaveConfig() is then called to save the component
*              non-retention configuration registers.
*   Checked:   If a transaction intended for component executes during this
*              function call, it waits until the current transaction is
*              completed. All subsequent I2C traffic intended for component
*              is NAKed until the device is put to sleep mode. The address
*              match event wakes up the device.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_Sleep(void) 
{
#if (Master_WAKEUP_ENABLED)
    /* Do not enable block after exit low power mode if it is wakeup source */
    Master_backup.enableState = Master_DISABLE;

    #if (Master_TIMEOUT_ENABLED)
        Master_TimeoutStop();
    #endif /* (Master_TIMEOUT_ENABLED) */

#else
    /* Store enable state */
    Master_backup.enableState = (uint8) Master_IS_ENABLED;

    if (0u != Master_backup.enableState)
    {
        Master_Stop();
    }
#endif /* (Master_WAKEUP_ENABLED) */

    Master_SaveConfig();
}


/*******************************************************************************
* Function Name: Master_RestoreConfig
********************************************************************************
*
* Summary:
*  The Enable wakeup from Sleep Mode selection influences this function
*  implementation:
*   Unchecked: Restores the component non-retention configuration registers
*              to the state they were in before I2C_Sleep() or I2C_SaveConfig()
*              was called.
*   Checked:   Disables the backup regulator of the I2C hardware. Sets up the
*              regular component interrupt handler and generates the component
*              interrupt if it was wake up source to release the bus and
*              continue in-coming I2C transaction.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Master_backup - The global variable used to save the component
*                            configuration and non-retention registers before
*                            exiting the sleep mode.
*
* Reentrant:
*  No.
*
* Side Effects:
*  Calling this function before Master_SaveConfig() or
*  Master_Sleep() will lead to unpredictable results.
*
*******************************************************************************/
void Master_RestoreConfig(void) 
{
#if (Master_FF_IMPLEMENTED)
    uint8 intState;

    if (Master_CHECK_PWRSYS_I2C_BACKUP)
    /* Low power mode was Sleep - backup regulator is enabled */
    {
        /* Enable backup regulator in active mode */
        intState = CyEnterCriticalSection();
        Master_PWRSYS_CR1_REG &= (uint8) ~Master_PWRSYS_CR1_I2C_REG_BACKUP;
        CyExitCriticalSection(intState);

        /* Restore master */
        Master_CFG_REG = Master_backup.cfg;
    }
    else
    /* Low power mode was Hibernate - backup regulator is disabled. All registers are cleared */
    {
    #if (Master_WAKEUP_ENABLED)
        /* Disable power to block before register restore */
        intState = CyEnterCriticalSection();
        Master_ACT_PWRMGR_REG  &= (uint8) ~Master_ACT_PWR_EN;
        Master_STBY_PWRMGR_REG &= (uint8) ~Master_STBY_PWR_EN;
        CyExitCriticalSection(intState);

        /* Enable component in I2C_Wakeup() after register restore */
        Master_backup.enableState = Master_ENABLE;
    #endif /* (Master_WAKEUP_ENABLED) */

        /* Restore component registers after Hibernate */
        Master_XCFG_REG    = Master_backup.xcfg;
        Master_CFG_REG     = Master_backup.cfg;
        Master_ADDR_REG    = Master_backup.addr;
        Master_CLKDIV1_REG = Master_backup.clkDiv1;
        Master_CLKDIV2_REG = Master_backup.clkDiv2;
    }

#if (Master_WAKEUP_ENABLED)
    Master_DisableInt();
    (void) CyIntSetVector(Master_ISR_NUMBER, &Master_ISR);
    if (0u != Master_wakeupSource)
    {
        /* Generate interrupt to process incoming transaction */
        Master_SetPendingInt();
    }
    Master_EnableInt();
#endif /* (Master_WAKEUP_ENABLED) */

#else
    Master_CFG_REG = Master_backup.control;
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    Master_TimeoutRestoreConfig();
#endif /* (Master_TIMEOUT_ENABLED) */
}


/*******************************************************************************
* Function Name: Master_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred method to prepare the component for active mode
*  operation (when device exits sleep mode). The Enable wakeup from Sleep Mode
*  selection influences this function implementation:
*   Unchecked: Restores the component non-retention configuration registers
*              by calling I2C_RestoreConfig(). If the component was enabled
*              before the I2C_Sleep() function was called, I2C_Wakeup()
*              re-enables it.
*   Checked:   Enables  master functionality if it was enabled before sleep,
*              and disables the backup regulator of the I2C hardware.
*              The incoming transaction continues as soon as the regular
*              I2C interrupt handler is set up (global interrupts has to be
*              enabled to service I2C component interrupt).
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
* Side Effects:
*  Calling this function before Master_SaveConfig() or
*  Master_Sleep() will lead to unpredictable results.
*
*******************************************************************************/
void Master_Wakeup(void) 
{
    Master_RestoreConfig();

    /* Restore component enable state */
    if (0u != Master_backup.enableState)
    {
        Master_Enable();
        Master_EnableInt();
    }
    else
    {
    #if (Master_TIMEOUT_ENABLED)
        Master_TimeoutEnable();
    #endif /* (Master_TIMEOUT_ENABLED) */
    }
}


/* [] END OF FILE */
