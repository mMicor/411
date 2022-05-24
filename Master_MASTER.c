/*******************************************************************************
* File Name: Master_MASTER.c
* Version 3.50
*
* Description:
*  This file provides the source code of APIs for the I2C component master mode.
*
*******************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Master_PVT.h"

#if(Master_MODE_MASTER_ENABLED)

/**********************************
*      System variables
**********************************/

volatile uint8 Master_mstrStatus;     /* Master Status byte  */
volatile uint8 Master_mstrControl;    /* Master Control byte */

/* Transmit buffer variables */
volatile uint8 * Master_mstrRdBufPtr;     /* Pointer to Master Read buffer */
volatile uint8   Master_mstrRdBufSize;    /* Master Read buffer size       */
volatile uint8   Master_mstrRdBufIndex;   /* Master Read buffer Index      */

/* Receive buffer variables */
volatile uint8 * Master_mstrWrBufPtr;     /* Pointer to Master Write buffer */
volatile uint8   Master_mstrWrBufSize;    /* Master Write buffer size       */
volatile uint8   Master_mstrWrBufIndex;   /* Master Write buffer Index      */


/*******************************************************************************
* Function Name: Master_MasterWriteBuf
********************************************************************************
*
* Summary:
*  Automatically writes an entire buffer of data to a slave device. Once the
*  data transfer is initiated by this function, further data transfer is handled
*  by the included ISR in byte by byte mode.
*
* Parameters:
*  slaveAddr: 7-bit slave address.
*  xferData:  Pointer to buffer of data to be sent.
*  cnt:       Size of buffer to send.
*  mode:      Transfer mode defines: start or restart condition generation at
*             begin of the transfer and complete the transfer or halt before
*             generating a stop.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  The included ISR will start a transfer after a start or restart condition is
*  generated.
*
* Global variables:
*  Master_mstrStatus  - The global variable used to store a current
*                                 status of the I2C Master.
*  Master_state       - The global variable used to store a current
*                                 state of the software FSM.
*  Master_mstrControl - The global variable used to control the master
*                                 end of a transaction with or without Stop
*                                 generation.
*  Master_mstrWrBufPtr - The global variable used to store a pointer
*                                  to the master write buffer.
*  Master_mstrWrBufIndex - The global variable used to store current
*                                    index within the master write buffer.
*  Master_mstrWrBufSize - The global variable used to store a master
*                                   write buffer size.
*
* Reentrant:
*  No
*
*******************************************************************************/
uint8 Master_MasterWriteBuf(uint8 slaveAddress, uint8 * wrData, uint8 cnt, uint8 mode)
      
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    if(NULL != wrData)
    {
        /* Check I2C state to allow transfer: valid states are IDLE or HALT */
        if(Master_SM_IDLE == Master_state)
        {
            /* Master is ready for transaction: check if bus is free */
            if(Master_CHECK_BUS_FREE(Master_MCSR_REG))
            {
                errStatus = Master_MSTR_NO_ERROR;
            }
            else
            {
                errStatus = Master_MSTR_BUS_BUSY;
            }
        }
        else if(Master_SM_MSTR_HALT == Master_state)
        {
            /* Master is ready and waiting for ReStart */
            errStatus = Master_MSTR_NO_ERROR;

            Master_ClearPendingInt();
            Master_mstrStatus &= (uint8) ~Master_MSTAT_XFER_HALT;
        }
        else
        {
            /* errStatus = Master_MSTR_NOT_READY was send before */
        }

        if(Master_MSTR_NO_ERROR == errStatus)
        {
            /* Set state to start write transaction */
            Master_state = Master_SM_MSTR_WR_ADDR;

            /* Prepare write buffer */
            Master_mstrWrBufIndex = 0u;
            Master_mstrWrBufSize  = cnt;
            Master_mstrWrBufPtr   = (volatile uint8 *) wrData;

            /* Set end of transaction flag: Stop or Halt (following ReStart) */
            Master_mstrControl = mode;

            /* Clear write status history */
            Master_mstrStatus &= (uint8) ~Master_MSTAT_WR_CMPLT;

            /* Hardware actions: write address and generate Start or ReStart */
            Master_DATA_REG = (uint8) (slaveAddress << Master_SLAVE_ADDR_SHIFT);

            if(Master_CHECK_RESTART(mode))
            {
                Master_GENERATE_RESTART;
            }
            else
            {
                Master_GENERATE_START;
            }

            /* Enable interrupt to complete transfer */
            Master_EnableInt();
        }
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterReadBuf
********************************************************************************
*
* Summary:
*  Automatically writes an entire buffer of data to a slave device. Once the
*  data transfer is initiated by this function, further data transfer is handled
*  by the included ISR in byte by byte mode.
*
* Parameters:
*  slaveAddr: 7-bit slave address.
*  xferData:  Pointer to buffer where to put data from slave.
*  cnt:       Size of buffer to read.
*  mode:      Transfer mode defines: start or restart condition generation at
*             begin of the transfer and complete the transfer or halt before
*             generating a stop.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  The included ISR will start a transfer after start or restart condition is
*  generated.
*
* Global variables:
*  Master_mstrStatus  - The global variable used to store a current
*                                 status of the I2C Master.
*  Master_state       - The global variable used to store a current
*                                 state of the software FSM.
*  Master_mstrControl - The global variable used to control the master
*                                 end of a transaction with or without
*                                 Stop generation.
*  Master_mstrRdBufPtr - The global variable used to store a pointer
*                                  to the master write buffer.
*  Master_mstrRdBufIndex - The global variable  used to store a
*                                    current index within the master
*                                    write buffer.
*  Master_mstrRdBufSize - The global variable used to store a master
*                                   write buffer size.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterReadBuf(uint8 slaveAddress, uint8 * rdData, uint8 cnt, uint8 mode)
      
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    if(NULL != rdData)
    {
        /* Check I2C state to allow transfer: valid states are IDLE or HALT */
        if(Master_SM_IDLE == Master_state)
        {
            /* Master is ready to transaction: check if bus is free */
            if(Master_CHECK_BUS_FREE(Master_MCSR_REG))
            {
                errStatus = Master_MSTR_NO_ERROR;
            }
            else
            {
                errStatus = Master_MSTR_BUS_BUSY;
            }
        }
        else if(Master_SM_MSTR_HALT == Master_state)
        {
            /* Master is ready and waiting for ReStart */
            errStatus = Master_MSTR_NO_ERROR;

            Master_ClearPendingInt();
            Master_mstrStatus &= (uint8) ~Master_MSTAT_XFER_HALT;
        }
        else
        {
            /* errStatus = Master_MSTR_NOT_READY was set before */
        }

        if(Master_MSTR_NO_ERROR == errStatus)
        {
            /* Set state to start write transaction */
            Master_state = Master_SM_MSTR_RD_ADDR;

            /* Prepare read buffer */
            Master_mstrRdBufIndex  = 0u;
            Master_mstrRdBufSize   = cnt;
            Master_mstrRdBufPtr    = (volatile uint8 *) rdData;

            /* Set end of transaction flag: Stop or Halt (following ReStart) */
            Master_mstrControl = mode;

            /* Clear read status history */
            Master_mstrStatus &= (uint8) ~Master_MSTAT_RD_CMPLT;

            /* Hardware actions: write address and generate Start or ReStart */
            Master_DATA_REG = ((uint8) (slaveAddress << Master_SLAVE_ADDR_SHIFT) |
                                                  Master_READ_FLAG);

            if(Master_CHECK_RESTART(mode))
            {
                Master_GENERATE_RESTART;
            }
            else
            {
                Master_GENERATE_START;
            }

            /* Enable interrupt to complete transfer */
            Master_EnableInt();
        }
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterSendStart
********************************************************************************
*
* Summary:
*  Generates Start condition and sends slave address with read/write bit.
*
* Parameters:
*  slaveAddress:  7-bit slave address.
*  R_nW:          Zero, send write command, non-zero send read command.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  This function is entered without a "byte complete" bit set in the I2C_CSR
*  register. It does not exit until it is set.
*
* Global variables:
*  Master_state - The global variable used to store a current state of
*                           the software FSM.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterSendStart(uint8 slaveAddress, uint8 R_nW)
      
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    /* If IDLE, check if bus is free */
    if(Master_SM_IDLE == Master_state)
    {
        /* If bus is free, generate Start condition */
        if(Master_CHECK_BUS_FREE(Master_MCSR_REG))
        {
            /* Disable interrupt for manual master operation */
            Master_DisableInt();

            /* Set address and read/write flag */
            slaveAddress = (uint8) (slaveAddress << Master_SLAVE_ADDR_SHIFT);
            if(0u != R_nW)
            {
                slaveAddress |= Master_READ_FLAG;
                Master_state = Master_SM_MSTR_RD_ADDR;
            }
            else
            {
                Master_state = Master_SM_MSTR_WR_ADDR;
            }

            /* Hardware actions: write address and generate Start */
            Master_DATA_REG = slaveAddress;
            Master_GENERATE_START_MANUAL;

            /* Wait until address is transferred */
            while(Master_WAIT_BYTE_COMPLETE(Master_CSR_REG))
            {
            }

        #if(Master_MODE_MULTI_MASTER_SLAVE_ENABLED)
            if(Master_CHECK_START_GEN(Master_MCSR_REG))
            {
                Master_CLEAR_START_GEN;

                /* Start condition was not generated: reset FSM to IDLE */
                Master_state = Master_SM_IDLE;
                errStatus = Master_MSTR_ERR_ABORT_START_GEN;
            }
            else
        #endif /* (Master_MODE_MULTI_MASTER_SLAVE_ENABLED) */

        #if(Master_MODE_MULTI_MASTER_ENABLED)
            if(Master_CHECK_LOST_ARB(Master_CSR_REG))
            {
                Master_BUS_RELEASE_MANUAL;

                /* Master lost arbitrage: reset FSM to IDLE */
                Master_state = Master_SM_IDLE;
                errStatus = Master_MSTR_ERR_ARB_LOST;
            }
            else
        #endif /* (Master_MODE_MULTI_MASTER_ENABLED) */

            if(Master_CHECK_ADDR_NAK(Master_CSR_REG))
            {
                /* Address has been NACKed: reset FSM to IDLE */
                Master_state = Master_SM_IDLE;
                errStatus = Master_MSTR_ERR_LB_NAK;
            }
            else
            {
                /* Start was sent without errors */
                errStatus = Master_MSTR_NO_ERROR;
            }
        }
        else
        {
            errStatus = Master_MSTR_BUS_BUSY;
        }
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterSendRestart
********************************************************************************
*
* Summary:
*  Generates ReStart condition and sends slave address with read/write bit.
*
* Parameters:
*  slaveAddress:  7-bit slave address.
*  R_nW:          Zero, send write command, non-zero send read command.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  This function is entered without a "byte complete" bit set in the I2C_CSR
*  register. It does not exit until it is set.
*
* Global variables:
*  Master_state - The global variable used to store a current state of
*                           the software FSM.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterSendRestart(uint8 slaveAddress, uint8 R_nW)
      
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    /* Check if START condition was generated */
    if(Master_CHECK_MASTER_MODE(Master_MCSR_REG))
    {
        /* Set address and read/write flag */
        slaveAddress = (uint8) (slaveAddress << Master_SLAVE_ADDR_SHIFT);
        if(0u != R_nW)
        {
            slaveAddress |= Master_READ_FLAG;
            Master_state = Master_SM_MSTR_RD_ADDR;
        }
        else
        {
            Master_state = Master_SM_MSTR_WR_ADDR;
        }

        /* Hardware actions: write address and generate ReStart */
        Master_DATA_REG = slaveAddress;
        Master_GENERATE_RESTART_MANUAL;

        /* Wait until address has been transferred */
        while(Master_WAIT_BYTE_COMPLETE(Master_CSR_REG))
        {
        }

    #if(Master_MODE_MULTI_MASTER_ENABLED)
        if(Master_CHECK_LOST_ARB(Master_CSR_REG))
        {
            Master_BUS_RELEASE_MANUAL;

            /* Master lost arbitrage: reset FSM to IDLE */
            Master_state = Master_SM_IDLE;
            errStatus = Master_MSTR_ERR_ARB_LOST;
        }
        else
    #endif /* (Master_MODE_MULTI_MASTER_ENABLED) */

        if(Master_CHECK_ADDR_NAK(Master_CSR_REG))
        {
            /* Address has been NACKed: reset FSM to IDLE */
            Master_state = Master_SM_IDLE;
            errStatus = Master_MSTR_ERR_LB_NAK;
        }
        else
        {
            /* ReStart was sent without errors */
            errStatus = Master_MSTR_NO_ERROR;
        }
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterSendStop
********************************************************************************
*
* Summary:
*  Generates I2C Stop condition on bus. Function do nothing if Start or Restart
*  condition was failed before call this function.
*
* Parameters:
*  None.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  Stop generation is required to complete the transaction.
*  This function does not wait until a Stop condition is generated.
*
* Global variables:
*  Master_state - The global variable used to store a current state of
*                           the software FSM.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterSendStop(void) 
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    /* Check if master is active on bus */
    if(Master_CHECK_MASTER_MODE(Master_MCSR_REG))
    {
        Master_GENERATE_STOP_MANUAL;
        Master_state = Master_SM_IDLE;

        /* Wait until stop has been generated */
        while(Master_WAIT_STOP_COMPLETE(Master_CSR_REG))
        {
        }

        errStatus = Master_MSTR_NO_ERROR;

    #if(Master_MODE_MULTI_MASTER_ENABLED)
        if(Master_CHECK_LOST_ARB(Master_CSR_REG))
        {
            Master_BUS_RELEASE_MANUAL;

            /* NACK was generated by instead Stop */
            errStatus = Master_MSTR_ERR_ARB_LOST;
        }
    #endif /* (Master_MODE_MULTI_MASTER_ENABLED) */
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterWriteByte
********************************************************************************
*
* Summary:
*  Sends one byte to a slave. A valid Start or ReStart condition must be
*  generated before this call this function. Function do nothing if Start or
*  Restart condition was failed before call this function.
*
* Parameters:
*  data:  The data byte to send to the slave.
*
* Return:
*  Status error - Zero means no errors.
*
* Side Effects:
*  This function is entered without a "byte complete" bit set in the I2C_CSR
*  register. It does not exit until it is set.
*
* Global variables:
*  Master_state - The global variable used to store a current state of
*                           the software FSM.
*
*******************************************************************************/
uint8 Master_MasterWriteByte(uint8 theByte) 
{
    uint8 errStatus;

    errStatus = Master_MSTR_NOT_READY;

    /* Check if START condition was generated */
    if(Master_CHECK_MASTER_MODE(Master_MCSR_REG))
    {
        Master_DATA_REG = theByte;   /* Write DATA register */
        Master_TRANSMIT_DATA_MANUAL; /* Set transmit mode   */
        Master_state = Master_SM_MSTR_WR_DATA;

        /* Wait until data byte has been transmitted */
        while(Master_WAIT_BYTE_COMPLETE(Master_CSR_REG))
        {
        }

    #if(Master_MODE_MULTI_MASTER_ENABLED)
        if(Master_CHECK_LOST_ARB(Master_CSR_REG))
        {
            Master_BUS_RELEASE_MANUAL;

            /* Master lost arbitrage: reset FSM to IDLE */
            Master_state = Master_SM_IDLE;
            errStatus = Master_MSTR_ERR_ARB_LOST;
        }
        /* Check LRB bit */
        else
    #endif /* (Master_MODE_MULTI_MASTER_ENABLED) */

        if(Master_CHECK_DATA_ACK(Master_CSR_REG))
        {
            Master_state = Master_SM_MSTR_HALT;
            errStatus = Master_MSTR_NO_ERROR;
        }
        else
        {
            Master_state = Master_SM_MSTR_HALT;
            errStatus = Master_MSTR_ERR_LB_NAK;
        }
    }

    return(errStatus);
}


/*******************************************************************************
* Function Name: Master_MasterReadByte
********************************************************************************
*
* Summary:
*  Reads one byte from a slave and ACK or NACK the transfer. A valid Start or
*  ReStart condition must be generated before this call this function. Function
*  do nothing if Start or Restart condition was failed before call this
*  function.
*
* Parameters:
*  acknNack:  Zero, response with NACK, if non-zero response with ACK.
*
* Return:
*  Byte read from slave.
*
* Side Effects:
*  This function is entered without a "byte complete" bit set in the I2C_CSR
*  register. It does not exit until it is set.
*
* Global variables:
*  Master_state - The global variable used to store a current
*                           state of the software FSM.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterReadByte(uint8 acknNak) 
{
    uint8 theByte;

    theByte = 0u;

    /* Check if START condition was generated */
    if(Master_CHECK_MASTER_MODE(Master_MCSR_REG))
    {
        /* When address phase needs to release bus and receive byte,
        * then decide ACK or NACK
        */
        if(Master_SM_MSTR_RD_ADDR == Master_state)
        {
            Master_READY_TO_READ_MANUAL;
            Master_state = Master_SM_MSTR_RD_DATA;
        }

        /* Wait until data byte has been received */
        while(Master_WAIT_BYTE_COMPLETE(Master_CSR_REG))
        {
        }

        theByte = Master_DATA_REG;

        /* Command ACK to receive next byte and continue transfer.
        *  Do nothing for NACK. The NACK will be generated by
        *  Stop or ReStart routine.
        */
        if(acknNak != 0u) /* Generate ACK */
        {
            Master_ACK_AND_RECEIVE_MANUAL;
        }
        else              /* Do nothing for the follwong NACK */
        {
            Master_state = Master_SM_MSTR_HALT;
        }
    }

    return(theByte);
}


/*******************************************************************************
* Function Name: Master_MasterStatus
********************************************************************************
*
* Summary:
*  Returns the master's communication status.
*
* Parameters:
*  None.
*
* Return:
*  Current status of I2C master.
*
* Global variables:
*  Master_mstrStatus - The global variable used to store a current
*                                status of the I2C Master.
*
*******************************************************************************/
uint8 Master_MasterStatus(void) 
{
    uint8 status;

    Master_DisableInt(); /* Lock from interrupt */

    /* Read master status */
    status = Master_mstrStatus;

    if (Master_CHECK_SM_MASTER)
    {
        /* Set transfer in progress flag in status */
        status |= Master_MSTAT_XFER_INP;
    }

    Master_EnableInt(); /* Release lock */

    return (status);
}


/*******************************************************************************
* Function Name: Master_MasterClearStatus
********************************************************************************
*
* Summary:
*  Clears all status flags and returns the master status.
*
* Parameters:
*  None.
*
* Return:
*  Current status of I2C master.
*
* Global variables:
*  Master_mstrStatus - The global variable used to store a current
*                                status of the I2C Master.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 Master_MasterClearStatus(void) 
{
    uint8 status;

    Master_DisableInt(); /* Lock from interrupt */

    /* Read and clear master status */
    status = Master_mstrStatus;
    Master_mstrStatus = Master_MSTAT_CLEAR;

    Master_EnableInt(); /* Release lock */

    return (status);
}


/*******************************************************************************
* Function Name: Master_MasterGetReadBufSize
********************************************************************************
*
* Summary:
*  Returns the amount of bytes that has been transferred with an
*  I2C_MasterReadBuf command.
*
* Parameters:
*  None.
*
* Return:
*  Byte count of transfer. If the transfer is not yet complete, it will return
*  the byte count transferred so far.
*
* Global variables:
*  Master_mstrRdBufIndex - The global variable stores current index
*                                    within the master read buffer.
*
*******************************************************************************/
uint8 Master_MasterGetReadBufSize(void) 
{
    return (Master_mstrRdBufIndex);
}


/*******************************************************************************
* Function Name: Master_MasterGetWriteBufSize
********************************************************************************
*
* Summary:
*  Returns the amount of bytes that has been transferred with an
*  I2C_MasterWriteBuf command.
*
* Parameters:
*  None.
*
* Return:
*  Byte count of transfer. If the transfer is not yet complete, it will return
*  the byte count transferred so far.
*
* Global variables:
*  Master_mstrWrBufIndex -  The global variable used to stores current
*                                     index within master write buffer.
*
*******************************************************************************/
uint8 Master_MasterGetWriteBufSize(void) 
{
    return (Master_mstrWrBufIndex);
}


/*******************************************************************************
* Function Name: Master_MasterClearReadBuf
********************************************************************************
*
* Summary:
*  Resets the read buffer pointer back to the first byte in the buffer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  Master_mstrRdBufIndex - The global variable used to stores current
*                                    index within master read buffer.
*  Master_mstrStatus - The global variable used to store a current
*                                status of the I2C Master.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_MasterClearReadBuf(void) 
{
    Master_DisableInt(); /* Lock from interrupt */

    Master_mstrRdBufIndex = 0u;
    Master_mstrStatus    &= (uint8) ~Master_MSTAT_RD_CMPLT;

    Master_EnableInt(); /* Release lock */
}


/*******************************************************************************
* Function Name: Master_MasterClearWriteBuf
********************************************************************************
*
* Summary:
*  Resets the write buffer pointer back to the first byte in the buffer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  Master_mstrRdBufIndex - The global variable used to stote current
*                                    index within master read buffer.
*  Master_mstrStatus - The global variable used to store a current
*                                status of the I2C Master.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Master_MasterClearWriteBuf(void) 
{
    Master_DisableInt(); /* Lock from interrupt */

    Master_mstrWrBufIndex = 0u;
    Master_mstrStatus    &= (uint8) ~Master_MSTAT_WR_CMPLT;

    Master_EnableInt(); /* Release lock */
}

#endif /* (Master_MODE_MASTER_ENABLED) */


/* [] END OF FILE */
