/*******************************************************************************
* File Name: Master_INT.c
* Version 3.50
*
* Description:
*  This file provides the source code of Interrupt Service Routine (ISR)
*  for the I2C component.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Master_PVT.h"
#include "cyapicallbacks.h"


/*******************************************************************************
*  Place your includes, defines and code here.
********************************************************************************/
/* `#START Master_ISR_intc` */

/* `#END` */


/*******************************************************************************
* Function Name: Master_ISR
********************************************************************************
*
* Summary:
*  The handler for the I2C interrupt. The slave and master operations are
*  handled here.
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
CY_ISR(Master_ISR)
{
#if (Master_MODE_SLAVE_ENABLED)
   uint8  tmp8;
#endif  /* (Master_MODE_SLAVE_ENABLED) */

    uint8  tmpCsr;
    
#ifdef Master_ISR_ENTRY_CALLBACK
    Master_ISR_EntryCallback();
#endif /* Master_ISR_ENTRY_CALLBACK */
    

#if(Master_TIMEOUT_FF_ENABLED)
    if(0u != Master_TimeoutGetStatus())
    {
        Master_TimeoutReset();
        Master_state = Master_SM_EXIT_IDLE;
        /* Master_CSR_REG should be cleared after reset */
    }
#endif /* (Master_TIMEOUT_FF_ENABLED) */


    tmpCsr = Master_CSR_REG;      /* Make copy as interrupts clear */

#if(Master_MODE_MULTI_MASTER_SLAVE_ENABLED)
    if(Master_CHECK_START_GEN(Master_MCSR_REG))
    {
        Master_CLEAR_START_GEN;

        /* Set transfer complete and error flags */
        Master_mstrStatus |= (Master_MSTAT_ERR_XFER |
                                        Master_GET_MSTAT_CMPLT);

        /* Slave was addressed */
        Master_state = Master_SM_SLAVE;
    }
#endif /* (Master_MODE_MULTI_MASTER_SLAVE_ENABLED) */


#if(Master_MODE_MULTI_MASTER_ENABLED)
    if(Master_CHECK_LOST_ARB(tmpCsr))
    {
        /* Set errors */
        Master_mstrStatus |= (Master_MSTAT_ERR_XFER     |
                                        Master_MSTAT_ERR_ARB_LOST |
                                        Master_GET_MSTAT_CMPLT);

        Master_DISABLE_INT_ON_STOP; /* Interrupt on Stop is enabled by write */

        #if(Master_MODE_MULTI_MASTER_SLAVE_ENABLED)
            if(Master_CHECK_ADDRESS_STS(tmpCsr))
            {
                /* Slave was addressed */
                Master_state = Master_SM_SLAVE;
            }
            else
            {
                Master_BUS_RELEASE;

                Master_state = Master_SM_EXIT_IDLE;
            }
        #else
            Master_BUS_RELEASE;

            Master_state = Master_SM_EXIT_IDLE;

        #endif /* (Master_MODE_MULTI_MASTER_SLAVE_ENABLED) */
    }
#endif /* (Master_MODE_MULTI_MASTER_ENABLED) */

    /* Check for master operation mode */
    if(Master_CHECK_SM_MASTER)
    {
    #if(Master_MODE_MASTER_ENABLED)
        if(Master_CHECK_BYTE_COMPLETE(tmpCsr))
        {
            switch (Master_state)
            {
            case Master_SM_MSTR_WR_ADDR:  /* After address is sent, write data */
            case Master_SM_MSTR_RD_ADDR:  /* After address is sent, read data */

                tmpCsr &= ((uint8) ~Master_CSR_STOP_STATUS); /* Clear Stop bit history on address phase */

                if(Master_CHECK_ADDR_ACK(tmpCsr))
                {
                    /* Setup for transmit or receive of data */
                    if(Master_state == Master_SM_MSTR_WR_ADDR)   /* TRANSMIT data */
                    {
                        /* Check if at least one byte to transfer */
                        if(Master_mstrWrBufSize > 0u)
                        {
                            /* Load the 1st data byte */
                            Master_DATA_REG = Master_mstrWrBufPtr[0u];
                            Master_TRANSMIT_DATA;
                            Master_mstrWrBufIndex = 1u;   /* Set index to 2nd element */

                            /* Set transmit state until done */
                            Master_state = Master_SM_MSTR_WR_DATA;
                        }
                        /* End of buffer: complete writing */
                        else if(Master_CHECK_NO_STOP(Master_mstrControl))
                        {
                            /* Set write complete and master halted */
                            Master_mstrStatus |= (Master_MSTAT_XFER_HALT |
                                                            Master_MSTAT_WR_CMPLT);

                            Master_state = Master_SM_MSTR_HALT; /* Expect ReStart */
                            Master_DisableInt();
                        }
                        else
                        {
                            Master_ENABLE_INT_ON_STOP; /* Enable interrupt on Stop, to catch it */
                            Master_GENERATE_STOP;
                        }
                    }
                    else  /* Master receive data */
                    {
                        Master_READY_TO_READ; /* Release bus to read data */

                        Master_state  = Master_SM_MSTR_RD_DATA;
                    }
                }
                /* Address is NACKed */
                else if(Master_CHECK_ADDR_NAK(tmpCsr))
                {
                    /* Set Address NAK error */
                    Master_mstrStatus |= (Master_MSTAT_ERR_XFER |
                                                    Master_MSTAT_ERR_ADDR_NAK);

                    if(Master_CHECK_NO_STOP(Master_mstrControl))
                    {
                        Master_mstrStatus |= (Master_MSTAT_XFER_HALT |
                                                        Master_GET_MSTAT_CMPLT);

                        Master_state = Master_SM_MSTR_HALT; /* Expect RESTART */
                        Master_DisableInt();
                    }
                    else  /* Do normal Stop */
                    {
                        Master_ENABLE_INT_ON_STOP; /* Enable interrupt on Stop, to catch it */
                        Master_GENERATE_STOP;
                    }
                }
                else
                {
                    /* Address phase is not set for some reason: error */
                    #if(Master_TIMEOUT_ENABLED)
                        /* Exit interrupt to take chance for timeout timer to handle this case */
                        Master_DisableInt();
                        Master_ClearPendingInt();
                    #else
                        /* Block execution flow: unexpected condition */
                        CYASSERT(0u != 0u);
                    #endif /* (Master_TIMEOUT_ENABLED) */
                }
                break;

            case Master_SM_MSTR_WR_DATA:

                if(Master_CHECK_DATA_ACK(tmpCsr))
                {
                    /* Check if end of buffer */
                    if(Master_mstrWrBufIndex  < Master_mstrWrBufSize)
                    {
                        Master_DATA_REG =
                                                 Master_mstrWrBufPtr[Master_mstrWrBufIndex];
                        Master_TRANSMIT_DATA;
                        Master_mstrWrBufIndex++;
                    }
                    /* End of buffer: complete writing */
                    else if(Master_CHECK_NO_STOP(Master_mstrControl))
                    {
                        /* Set write complete and master halted */
                        Master_mstrStatus |= (Master_MSTAT_XFER_HALT |
                                                        Master_MSTAT_WR_CMPLT);

                        Master_state = Master_SM_MSTR_HALT;    /* Expect restart */
                        Master_DisableInt();
                    }
                    else  /* Do normal Stop */
                    {
                        Master_ENABLE_INT_ON_STOP;    /* Enable interrupt on Stop, to catch it */
                        Master_GENERATE_STOP;
                    }
                }
                /* Last byte NAKed: end writing */
                else if(Master_CHECK_NO_STOP(Master_mstrControl))
                {
                    /* Set write complete, short transfer and master halted */
                    Master_mstrStatus |= (Master_MSTAT_ERR_XFER       |
                                                    Master_MSTAT_ERR_SHORT_XFER |
                                                    Master_MSTAT_XFER_HALT      |
                                                    Master_MSTAT_WR_CMPLT);

                    Master_state = Master_SM_MSTR_HALT;    /* Expect ReStart */
                    Master_DisableInt();
                }
                else  /* Do normal Stop */
                {
                    Master_ENABLE_INT_ON_STOP;    /* Enable interrupt on Stop, to catch it */
                    Master_GENERATE_STOP;

                    /* Set short transfer and error flag */
                    Master_mstrStatus |= (Master_MSTAT_ERR_SHORT_XFER |
                                                    Master_MSTAT_ERR_XFER);
                }

                break;

            case Master_SM_MSTR_RD_DATA:

                Master_mstrRdBufPtr[Master_mstrRdBufIndex] = Master_DATA_REG;
                Master_mstrRdBufIndex++;

                /* Check if end of buffer */
                if(Master_mstrRdBufIndex < Master_mstrRdBufSize)
                {
                    Master_ACK_AND_RECEIVE;       /* ACK and receive byte */
                }
                /* End of buffer: complete reading */
                else if(Master_CHECK_NO_STOP(Master_mstrControl))
                {
                    /* Set read complete and master halted */
                    Master_mstrStatus |= (Master_MSTAT_XFER_HALT |
                                                    Master_MSTAT_RD_CMPLT);

                    Master_state = Master_SM_MSTR_HALT;    /* Expect ReStart */
                    Master_DisableInt();
                }
                else
                {
                    Master_ENABLE_INT_ON_STOP;
                    Master_NAK_AND_RECEIVE;       /* NACK and TRY to generate Stop */
                }
                break;

            default: /* This is an invalid state and should not occur */

            #if(Master_TIMEOUT_ENABLED)
                /* Exit interrupt to take chance for timeout timer to handles this case */
                Master_DisableInt();
                Master_ClearPendingInt();
            #else
                /* Block execution flow: unexpected condition */
                CYASSERT(0u != 0u);
            #endif /* (Master_TIMEOUT_ENABLED) */

                break;
            }
        }

        /* Catches Stop: end of transaction */
        if(Master_CHECK_STOP_STS(tmpCsr))
        {
            Master_mstrStatus |= Master_GET_MSTAT_CMPLT;

            Master_DISABLE_INT_ON_STOP;
            Master_state = Master_SM_IDLE;
        }
    #endif /* (Master_MODE_MASTER_ENABLED) */
    }
    else if(Master_CHECK_SM_SLAVE)
    {
    #if(Master_MODE_SLAVE_ENABLED)

        if((Master_CHECK_STOP_STS(tmpCsr)) || /* Stop || Restart */
           (Master_CHECK_BYTE_COMPLETE(tmpCsr) && Master_CHECK_ADDRESS_STS(tmpCsr)))
        {
            /* Catch end of master write transaction: use interrupt on Stop */
            /* The Stop bit history on address phase does not have correct state */
            if(Master_SM_SL_WR_DATA == Master_state)
            {
                Master_DISABLE_INT_ON_STOP;

                Master_slStatus &= ((uint8) ~Master_SSTAT_WR_BUSY);
                Master_slStatus |= ((uint8)  Master_SSTAT_WR_CMPLT);

                Master_state = Master_SM_IDLE;
            }
        }

        if(Master_CHECK_BYTE_COMPLETE(tmpCsr))
        {
            /* The address only issued after Start or ReStart: so check the address
               to catch these events:
                FF : sets an address phase with a byte_complete interrupt trigger.
                UDB: sets an address phase immediately after Start or ReStart. */
            if(Master_CHECK_ADDRESS_STS(tmpCsr))
            {
            /* Check for software address detection */
            #if(Master_SW_ADRR_DECODE)
                tmp8 = Master_GET_SLAVE_ADDR(Master_DATA_REG);

                if(tmp8 == Master_slAddress)   /* Check for address match */
                {
                    if(0u != (Master_DATA_REG & Master_READ_FLAG))
                    {
                        /* Place code to prepare read buffer here                  */
                        /* `#START Master_SW_PREPARE_READ_BUF_interrupt` */

                        /* `#END` */

                    #ifdef Master_SW_PREPARE_READ_BUF_CALLBACK
                        Master_SwPrepareReadBuf_Callback();
                    #endif /* Master_SW_PREPARE_READ_BUF_CALLBACK */
                        
                        /* Prepare next operation to read, get data and place in data register */
                        if(Master_slRdBufIndex < Master_slRdBufSize)
                        {
                            /* Load first data byte from array */
                            Master_DATA_REG = Master_slRdBufPtr[Master_slRdBufIndex];
                            Master_ACK_AND_TRANSMIT;
                            Master_slRdBufIndex++;

                            Master_slStatus |= Master_SSTAT_RD_BUSY;
                        }
                        else    /* Overflow: provide 0xFF on bus */
                        {
                            Master_DATA_REG = Master_OVERFLOW_RETURN;
                            Master_ACK_AND_TRANSMIT;

                            Master_slStatus  |= (Master_SSTAT_RD_BUSY |
                                                           Master_SSTAT_RD_ERR_OVFL);
                        }

                        Master_state = Master_SM_SL_RD_DATA;
                    }
                    else  /* Write transaction: receive 1st byte */
                    {
                        Master_ACK_AND_RECEIVE;
                        Master_state = Master_SM_SL_WR_DATA;

                        Master_slStatus |= Master_SSTAT_WR_BUSY;
                        Master_ENABLE_INT_ON_STOP;
                    }
                }
                else
                {
                    /*     Place code to compare for additional address here    */
                    /* `#START Master_SW_ADDR_COMPARE_interruptStart` */

                    /* `#END` */

                #ifdef Master_SW_ADDR_COMPARE_ENTRY_CALLBACK
                    Master_SwAddrCompare_EntryCallback();
                #endif /* Master_SW_ADDR_COMPARE_ENTRY_CALLBACK */
                    
                    Master_NAK_AND_RECEIVE;   /* NACK address */

                    /* Place code to end of condition for NACK generation here */
                    /* `#START Master_SW_ADDR_COMPARE_interruptEnd`  */

                    /* `#END` */

                #ifdef Master_SW_ADDR_COMPARE_EXIT_CALLBACK
                    Master_SwAddrCompare_ExitCallback();
                #endif /* Master_SW_ADDR_COMPARE_EXIT_CALLBACK */
                }

            #else /* (Master_HW_ADRR_DECODE) */

                if(0u != (Master_DATA_REG & Master_READ_FLAG))
                {
                    /* Place code to prepare read buffer here                  */
                    /* `#START Master_HW_PREPARE_READ_BUF_interrupt` */

                    /* `#END` */
                    
                #ifdef Master_HW_PREPARE_READ_BUF_CALLBACK
                    Master_HwPrepareReadBuf_Callback();
                #endif /* Master_HW_PREPARE_READ_BUF_CALLBACK */

                    /* Prepare next operation to read, get data and place in data register */
                    if(Master_slRdBufIndex < Master_slRdBufSize)
                    {
                        /* Load first data byte from array */
                        Master_DATA_REG = Master_slRdBufPtr[Master_slRdBufIndex];
                        Master_ACK_AND_TRANSMIT;
                        Master_slRdBufIndex++;

                        Master_slStatus |= Master_SSTAT_RD_BUSY;
                    }
                    else    /* Overflow: provide 0xFF on bus */
                    {
                        Master_DATA_REG = Master_OVERFLOW_RETURN;
                        Master_ACK_AND_TRANSMIT;

                        Master_slStatus  |= (Master_SSTAT_RD_BUSY |
                                                       Master_SSTAT_RD_ERR_OVFL);
                    }

                    Master_state = Master_SM_SL_RD_DATA;
                }
                else  /* Write transaction: receive 1st byte */
                {
                    Master_ACK_AND_RECEIVE;
                    Master_state = Master_SM_SL_WR_DATA;

                    Master_slStatus |= Master_SSTAT_WR_BUSY;
                    Master_ENABLE_INT_ON_STOP;
                }

            #endif /* (Master_SW_ADRR_DECODE) */
            }
            /* Data states */
            /* Data master writes into slave */
            else if(Master_state == Master_SM_SL_WR_DATA)
            {
                if(Master_slWrBufIndex < Master_slWrBufSize)
                {
                    tmp8 = Master_DATA_REG;
                    Master_ACK_AND_RECEIVE;
                    Master_slWrBufPtr[Master_slWrBufIndex] = tmp8;
                    Master_slWrBufIndex++;
                }
                else  /* of array: complete write, send NACK */
                {
                    Master_NAK_AND_RECEIVE;

                    Master_slStatus |= Master_SSTAT_WR_ERR_OVFL;
                }
            }
            /* Data master reads from slave */
            else if(Master_state == Master_SM_SL_RD_DATA)
            {
                if(Master_CHECK_DATA_ACK(tmpCsr))
                {
                    if(Master_slRdBufIndex < Master_slRdBufSize)
                    {
                         /* Get data from array */
                        Master_DATA_REG = Master_slRdBufPtr[Master_slRdBufIndex];
                        Master_TRANSMIT_DATA;
                        Master_slRdBufIndex++;
                    }
                    else   /* Overflow: provide 0xFF on bus */
                    {
                        Master_DATA_REG = Master_OVERFLOW_RETURN;
                        Master_TRANSMIT_DATA;

                        Master_slStatus |= Master_SSTAT_RD_ERR_OVFL;
                    }
                }
                else  /* Last byte was NACKed: read complete */
                {
                    /* Only NACK appears on bus */
                    Master_DATA_REG = Master_OVERFLOW_RETURN;
                    Master_NAK_AND_TRANSMIT;

                    Master_slStatus &= ((uint8) ~Master_SSTAT_RD_BUSY);
                    Master_slStatus |= ((uint8)  Master_SSTAT_RD_CMPLT);

                    Master_state = Master_SM_IDLE;
                }
            }
            else
            {
            #if(Master_TIMEOUT_ENABLED)
                /* Exit interrupt to take chance for timeout timer to handle this case */
                Master_DisableInt();
                Master_ClearPendingInt();
            #else
                /* Block execution flow: unexpected condition */
                CYASSERT(0u != 0u);
            #endif /* (Master_TIMEOUT_ENABLED) */
            }
        }
    #endif /* (Master_MODE_SLAVE_ENABLED) */
    }
    else
    {
        /* The FSM skips master and slave processing: return to IDLE */
        Master_state = Master_SM_IDLE;
    }

#ifdef Master_ISR_EXIT_CALLBACK
    Master_ISR_ExitCallback();
#endif /* Master_ISR_EXIT_CALLBACK */    
}


#if ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED))
    /*******************************************************************************
    * Function Name: Master_WAKEUP_ISR
    ********************************************************************************
    *
    * Summary:
    *  The interrupt handler to trigger after a wakeup.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    CY_ISR(Master_WAKEUP_ISR)
    {
    #ifdef Master_WAKEUP_ISR_ENTRY_CALLBACK
        Master_WAKEUP_ISR_EntryCallback();
    #endif /* Master_WAKEUP_ISR_ENTRY_CALLBACK */
         
        /* Set flag to notify that matched address is received */
        Master_wakeupSource = 1u;

        /* SCL is stretched until the I2C_Wake() is called */

    #ifdef Master_WAKEUP_ISR_EXIT_CALLBACK
        Master_WAKEUP_ISR_ExitCallback();
    #endif /* Master_WAKEUP_ISR_EXIT_CALLBACK */
    }
#endif /* ((Master_FF_IMPLEMENTED) && (Master_WAKEUP_ENABLED)) */


/* [] END OF FILE */
