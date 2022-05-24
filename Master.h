/*******************************************************************************
* File Name: Master.h
* Version 3.50
*
* Description:
*  This file provides constants and parameter values for the I2C component.

*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_I2C_Master_H)
#define CY_I2C_Master_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h"

/* Check if required defines such as CY_PSOC5LP are available in cy_boot */
#if !defined (CY_PSOC5LP)
    #error Component I2C_v3_50 requires cy_boot v3.10 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define Master_IMPLEMENTATION     (1u)
#define Master_MODE               (2u)
#define Master_ENABLE_WAKEUP      (0u)
#define Master_ADDR_DECODE        (1u)
#define Master_UDB_INTRN_CLOCK    (0u)


/* I2C implementation enum */
#define Master_UDB    (0x00u)
#define Master_FF     (0x01u)

#define Master_FF_IMPLEMENTED     (Master_FF  == Master_IMPLEMENTATION)
#define Master_UDB_IMPLEMENTED    (Master_UDB == Master_IMPLEMENTATION)

#define Master_UDB_INTRN_CLOCK_ENABLED    (Master_UDB_IMPLEMENTED && \
                                                     (0u != Master_UDB_INTRN_CLOCK))
/* I2C modes enum */
#define Master_MODE_SLAVE                 (0x01u)
#define Master_MODE_MASTER                (0x02u)
#define Master_MODE_MULTI_MASTER          (0x06u)
#define Master_MODE_MULTI_MASTER_SLAVE    (0x07u)
#define Master_MODE_MULTI_MASTER_MASK     (0x04u)

#define Master_MODE_SLAVE_ENABLED         (0u != (Master_MODE_SLAVE  & Master_MODE))
#define Master_MODE_MASTER_ENABLED        (0u != (Master_MODE_MASTER & Master_MODE))
#define Master_MODE_MULTI_MASTER_ENABLED  (0u != (Master_MODE_MULTI_MASTER_MASK & \
                                                            Master_MODE))
#define Master_MODE_MULTI_MASTER_SLAVE_ENABLED    (Master_MODE_MULTI_MASTER_SLAVE == \
                                                             Master_MODE)

/* Address detection enum */
#define Master_SW_DECODE      (0x00u)
#define Master_HW_DECODE      (0x01u)

#define Master_SW_ADRR_DECODE             (Master_SW_DECODE == Master_ADDR_DECODE)
#define Master_HW_ADRR_DECODE             (Master_HW_DECODE == Master_ADDR_DECODE)

/* Wakeup enabled */
#define Master_WAKEUP_ENABLED             (0u != Master_ENABLE_WAKEUP)

/* Adds bootloader APIs to component */
#define Master_BOOTLOADER_INTERFACE_ENABLED   (Master_MODE_SLAVE_ENABLED && \
                                                            ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Master) || \
                                                             (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface)))

/* Timeout functionality */
#define Master_TIMEOUT_ENABLE             (0u)
#define Master_TIMEOUT_SCL_TMOUT_ENABLE   (0u)
#define Master_TIMEOUT_SDA_TMOUT_ENABLE   (0u)
#define Master_TIMEOUT_PRESCALER_ENABLE   (0u)
#define Master_TIMEOUT_IMPLEMENTATION     (0u)

/* Convert to boolean */
#define Master_TIMEOUT_ENABLED            (0u != Master_TIMEOUT_ENABLE)
#define Master_TIMEOUT_SCL_TMOUT_ENABLED  (0u != Master_TIMEOUT_SCL_TMOUT_ENABLE)
#define Master_TIMEOUT_SDA_TMOUT_ENABLED  (0u != Master_TIMEOUT_SDA_TMOUT_ENABLE)
#define Master_TIMEOUT_PRESCALER_ENABLED  (0u != Master_TIMEOUT_PRESCALER_ENABLE)

/* Timeout implementation enum. */
#define Master_TIMEOUT_UDB    (0x00u)
#define Master_TIMEOUT_FF     (0x01u)

#define Master_TIMEOUT_FF_IMPLEMENTED     (Master_TIMEOUT_FF  == \
                                                        Master_TIMEOUT_IMPLEMENTATION)
#define Master_TIMEOUT_UDB_IMPLEMENTED    (Master_TIMEOUT_UDB == \
                                                        Master_TIMEOUT_IMPLEMENTATION)

#define Master_TIMEOUT_FF_ENABLED         (Master_TIMEOUT_ENABLED && \
                                                     Master_TIMEOUT_FF_IMPLEMENTED)

#define Master_TIMEOUT_UDB_ENABLED        (Master_TIMEOUT_ENABLED && \
                                                     Master_TIMEOUT_UDB_IMPLEMENTED)

#define Master_EXTERN_I2C_INTR_HANDLER    (0u)
#define Master_EXTERN_TMOUT_INTR_HANDLER  (0u)

#define Master_INTERN_I2C_INTR_HANDLER    (0u == Master_EXTERN_I2C_INTR_HANDLER)
#define Master_INTERN_TMOUT_INTR_HANDLER  (0u == Master_EXTERN_TMOUT_INTR_HANDLER)


/***************************************
*       Type defines
***************************************/

/* Structure to save registers before go to sleep */
typedef struct
{
    uint8 enableState;

#if (Master_FF_IMPLEMENTED)
    uint8 xcfg;
    uint8 cfg;
    uint8 addr;
    uint8 clkDiv1;
    uint8 clkDiv2;
#else
    uint8 control;
#endif /* (Master_FF_IMPLEMENTED) */

#if (Master_TIMEOUT_ENABLED)
    uint16 tmoutCfg;
    uint8  tmoutIntr;
#endif /* (Master_TIMEOUT_ENABLED) */

} Master_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void Master_Init(void)                            ;
void Master_Enable(void)                          ;

void Master_Start(void)                           ;
void Master_Stop(void)                            ;

#define Master_EnableInt()        CyIntEnable      (Master_ISR_NUMBER)
#define Master_DisableInt()       CyIntDisable     (Master_ISR_NUMBER)
#define Master_ClearPendingInt()  CyIntClearPending(Master_ISR_NUMBER)
#define Master_SetPendingInt()    CyIntSetPending  (Master_ISR_NUMBER)

void Master_SaveConfig(void)                      ;
void Master_Sleep(void)                           ;
void Master_RestoreConfig(void)                   ;
void Master_Wakeup(void)                          ;

/* I2C Master functions prototypes */
#if (Master_MODE_MASTER_ENABLED)
    /* Read and Clear status functions */
    uint8 Master_MasterStatus(void)                ;
    uint8 Master_MasterClearStatus(void)           ;

    /* Interrupt based operation functions */
    uint8 Master_MasterWriteBuf(uint8 slaveAddress, uint8 * wrData, uint8 cnt, uint8 mode) \
                                                            ;
    uint8 Master_MasterReadBuf(uint8 slaveAddress, uint8 * rdData, uint8 cnt, uint8 mode) \
                                                            ;
    uint8 Master_MasterGetReadBufSize(void)       ;
    uint8 Master_MasterGetWriteBufSize(void)      ;
    void  Master_MasterClearReadBuf(void)         ;
    void  Master_MasterClearWriteBuf(void)        ;

    /* Manual operation functions */
    uint8 Master_MasterSendStart(uint8 slaveAddress, uint8 R_nW) \
                                                            ;
    uint8 Master_MasterSendRestart(uint8 slaveAddress, uint8 R_nW) \
                                                            ;
    uint8 Master_MasterSendStop(void)             ;
    uint8 Master_MasterWriteByte(uint8 theByte)   ;
    uint8 Master_MasterReadByte(uint8 acknNak)    ;

#endif /* (Master_MODE_MASTER_ENABLED) */

/* I2C Slave functions prototypes */
#if (Master_MODE_SLAVE_ENABLED)
    /* Read and Clear status functions */
    uint8 Master_SlaveStatus(void)                ;
    uint8 Master_SlaveClearReadStatus(void)       ;
    uint8 Master_SlaveClearWriteStatus(void)      ;

    void  Master_SlaveSetAddress(uint8 address)   ;

    /* Interrupt based operation functions */
    void  Master_SlaveInitReadBuf(uint8 * rdBuf, uint8 bufSize) \
                                                            ;
    void  Master_SlaveInitWriteBuf(uint8 * wrBuf, uint8 bufSize) \
                                                            ;
    uint8 Master_SlaveGetReadBufSize(void)        ;
    uint8 Master_SlaveGetWriteBufSize(void)       ;
    void  Master_SlaveClearReadBuf(void)          ;
    void  Master_SlaveClearWriteBuf(void)         ;

    /* Communication bootloader I2C Slave APIs */
    #if defined(CYDEV_BOOTLOADER_IO_COMP) && (Master_BOOTLOADER_INTERFACE_ENABLED)
        /* Physical layer functions */
        void     Master_CyBtldrCommStart(void) CYSMALL \
                                                            ;
        void     Master_CyBtldrCommStop(void)  CYSMALL \
                                                            ;
        void     Master_CyBtldrCommReset(void) CYSMALL \
                                                            ;
        cystatus Master_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) \
                                                        CYSMALL ;
        cystatus Master_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)  CYSMALL \
                                                            ;

        #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Master)
            #define CyBtldrCommStart    Master_CyBtldrCommStart
            #define CyBtldrCommStop     Master_CyBtldrCommStop
            #define CyBtldrCommReset    Master_CyBtldrCommReset
            #define CyBtldrCommWrite    Master_CyBtldrCommWrite
            #define CyBtldrCommRead     Master_CyBtldrCommRead
        #endif /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Master) */

        /* Size of Read/Write buffers for I2C bootloader  */
        #define Master_BTLDR_SIZEOF_READ_BUFFER   (0x80u)
        #define Master_BTLDR_SIZEOF_WRITE_BUFFER  (0x80u)
        #define Master_MIN_UNT16(a, b)            ( ((uint16)(a) < (b)) ? ((uint16) (a)) : ((uint16) (b)) )
        #define Master_WAIT_1_MS                  (1u)

    #endif /* defined(CYDEV_BOOTLOADER_IO_COMP) && (Master_BOOTLOADER_INTERFACE_ENABLED) */

#endif /* (Master_MODE_SLAVE_ENABLED) */

/* Component interrupt handlers */
CY_ISR_PROTO(Master_ISR);
#if ((Master_FF_IMPLEMENTED) || (Master_WAKEUP_ENABLED))
    CY_ISR_PROTO(Master_WAKEUP_ISR);
#endif /* ((Master_FF_IMPLEMENTED) || (Master_WAKEUP_ENABLED)) */


/**********************************
*   Variable with external linkage
**********************************/

extern uint8 Master_initVar;


/***************************************
*   Initial Parameter Constants
***************************************/

#define Master_DATA_RATE          (400u)
#define Master_DEFAULT_ADDR       (8u)
#define Master_I2C_PAIR_SELECTED  (0u)

/* I2C pair enum */
#define Master_I2C_PAIR_ANY   (0x01u) /* Any pins for I2C */
#define Master_I2C_PAIR0      (0x01u) /* I2C0: (SCL = P12[4]) && (SCL = P12[5]) */
#define Master_I2C_PAIR1      (0x02u) /* I2C1: (SCL = P12[0]) && (SDA = P12[1]) */

#define Master_I2C1_SIO_PAIR  (Master_I2C_PAIR1 == Master_I2C_PAIR_SELECTED)
#define Master_I2C0_SIO_PAIR  (Master_I2C_PAIR0 == Master_I2C_PAIR_SELECTED)


/***************************************
*            API Constants
***************************************/

/* Master/Slave control constants */
#define Master_READ_XFER_MODE     (0x01u) /* Read */
#define Master_WRITE_XFER_MODE    (0x00u) /* Write */
#define Master_ACK_DATA           (0x01u) /* Send ACK */
#define Master_NAK_DATA           (0x00u) /* Send NAK */
#define Master_OVERFLOW_RETURN    (0xFFu) /* Send on bus in case of overflow */

#if (Master_MODE_MASTER_ENABLED)
    /* "Mode" constants for MasterWriteBuf() or MasterReadBuf() function */
    #define Master_MODE_COMPLETE_XFER     (0x00u) /* Full transfer with Start and Stop */
    #define Master_MODE_REPEAT_START      (0x01u) /* Begin with a ReStart instead of a Start */
    #define Master_MODE_NO_STOP           (0x02u) /* Complete the transfer without a Stop */

    /* Master status */
    #define Master_MSTAT_CLEAR            (0x00u) /* Clear (initialize) status value */

    #define Master_MSTAT_RD_CMPLT         (0x01u) /* Read complete */
    #define Master_MSTAT_WR_CMPLT         (0x02u) /* Write complete */
    #define Master_MSTAT_XFER_INP         (0x04u) /* Master transfer in progress */
    #define Master_MSTAT_XFER_HALT        (0x08u) /* Transfer is halted */

    #define Master_MSTAT_ERR_MASK         (0xF0u) /* Mask for all errors */
    #define Master_MSTAT_ERR_SHORT_XFER   (0x10u) /* Master NAKed before end of packet */
    #define Master_MSTAT_ERR_ADDR_NAK     (0x20u) /* Slave did not ACK */
    #define Master_MSTAT_ERR_ARB_LOST     (0x40u) /* Master lost arbitration during communication */
    #define Master_MSTAT_ERR_XFER         (0x80u) /* Error during transfer */

    /* Master API returns */
    #define Master_MSTR_NO_ERROR          (0x00u) /* Function complete without error */
    #define Master_MSTR_BUS_BUSY          (0x01u) /* Bus is busy, process not started */
    #define Master_MSTR_NOT_READY         (0x02u) /* Master not Master on the bus or */
                                                            /*  Slave operation in progress */
    #define Master_MSTR_ERR_LB_NAK        (0x03u) /* Last Byte Naked */
    #define Master_MSTR_ERR_ARB_LOST      (0x04u) /* Master lost arbitration during communication */
    #define Master_MSTR_ERR_ABORT_START_GEN  (0x05u) /* Master did not generate Start, the Slave */
                                                               /* was addressed before */

#endif /* (Master_MODE_MASTER_ENABLED) */

#if (Master_MODE_SLAVE_ENABLED)
    /* Slave Status Constants */
    #define Master_SSTAT_RD_CMPLT     (0x01u) /* Read transfer complete */
    #define Master_SSTAT_RD_BUSY      (0x02u) /* Read transfer in progress */
    #define Master_SSTAT_RD_ERR_OVFL  (0x04u) /* Read overflow Error */
    #define Master_SSTAT_RD_MASK      (0x0Fu) /* Read Status Mask */
    #define Master_SSTAT_RD_NO_ERR    (0x00u) /* Read no Error */

    #define Master_SSTAT_WR_CMPLT     (0x10u) /* Write transfer complete */
    #define Master_SSTAT_WR_BUSY      (0x20u) /* Write transfer in progress */
    #define Master_SSTAT_WR_ERR_OVFL  (0x40u) /* Write overflow Error */
    #define Master_SSTAT_WR_MASK      (0xF0u) /* Write Status Mask  */
    #define Master_SSTAT_WR_NO_ERR    (0x00u) /* Write no Error */

    #define Master_SSTAT_RD_CLEAR     (0x0Du) /* Read Status clear */
    #define Master_SSTAT_WR_CLEAR     (0xD0u) /* Write Status Clear */

#endif /* (Master_MODE_SLAVE_ENABLED) */


/***************************************
*       I2C state machine constants
***************************************/

/* Default slave address states */
#define  Master_SM_IDLE           (0x10u) /* Default state - IDLE */
#define  Master_SM_EXIT_IDLE      (0x00u) /* Pass master and slave processing and go to IDLE */

/* Slave mode states */
#define  Master_SM_SLAVE          (Master_SM_IDLE) /* Any Slave state */
#define  Master_SM_SL_WR_DATA     (0x11u) /* Master writes data to slave  */
#define  Master_SM_SL_RD_DATA     (0x12u) /* Master reads data from slave */

/* Master mode states */
#define  Master_SM_MASTER         (0x40u) /* Any master state */

#define  Master_SM_MSTR_RD        (0x08u) /* Any master read state          */
#define  Master_SM_MSTR_RD_ADDR   (0x49u) /* Master sends address with read */
#define  Master_SM_MSTR_RD_DATA   (0x4Au) /* Master reads data              */

#define  Master_SM_MSTR_WR        (0x04u) /* Any master read state           */
#define  Master_SM_MSTR_WR_ADDR   (0x45u) /* Master sends address with write */
#define  Master_SM_MSTR_WR_DATA   (0x46u) /* Master writes data              */

#define  Master_SM_MSTR_HALT      (0x60u) /* Master waits for ReStart */

#define Master_CHECK_SM_MASTER    (0u != (Master_SM_MASTER & Master_state))
#define Master_CHECK_SM_SLAVE     (0u != (Master_SM_SLAVE  & Master_state))


/***************************************
*              Registers
***************************************/

#if (Master_FF_IMPLEMENTED)
    /* Fixed Function registers */
    #define Master_XCFG_REG           (*(reg8 *) Master_I2C_FF__XCFG)
    #define Master_XCFG_PTR           ( (reg8 *) Master_I2C_FF__XCFG)

    #define Master_ADDR_REG           (*(reg8 *) Master_I2C_FF__ADR)
    #define Master_ADDR_PTR           ( (reg8 *) Master_I2C_FF__ADR)

    #define Master_CFG_REG            (*(reg8 *) Master_I2C_FF__CFG)
    #define Master_CFG_PTR            ( (reg8 *) Master_I2C_FF__CFG)

    #define Master_CSR_REG            (*(reg8 *) Master_I2C_FF__CSR)
    #define Master_CSR_PTR            ( (reg8 *) Master_I2C_FF__CSR)

    #define Master_DATA_REG           (*(reg8 *) Master_I2C_FF__D)
    #define Master_DATA_PTR           ( (reg8 *) Master_I2C_FF__D)

    #define Master_MCSR_REG           (*(reg8 *) Master_I2C_FF__MCSR)
    #define Master_MCSR_PTR           ( (reg8 *) Master_I2C_FF__MCSR)

    #define Master_ACT_PWRMGR_REG     (*(reg8 *) Master_I2C_FF__PM_ACT_CFG)
    #define Master_ACT_PWRMGR_PTR     ( (reg8 *) Master_I2C_FF__PM_ACT_CFG)
    #define Master_ACT_PWR_EN         ( (uint8)  Master_I2C_FF__PM_ACT_MSK)

    #define Master_STBY_PWRMGR_REG    (*(reg8 *) Master_I2C_FF__PM_STBY_CFG)
    #define Master_STBY_PWRMGR_PTR    ( (reg8 *) Master_I2C_FF__PM_STBY_CFG)
    #define Master_STBY_PWR_EN        ( (uint8)  Master_I2C_FF__PM_STBY_MSK)

    #define Master_PWRSYS_CR1_REG     (*(reg8 *) CYREG_PWRSYS_CR1)
    #define Master_PWRSYS_CR1_PTR     ( (reg8 *) CYREG_PWRSYS_CR1)

    #define Master_CLKDIV1_REG    (*(reg8 *) Master_I2C_FF__CLK_DIV1)
    #define Master_CLKDIV1_PTR    ( (reg8 *) Master_I2C_FF__CLK_DIV1)

    #define Master_CLKDIV2_REG    (*(reg8 *) Master_I2C_FF__CLK_DIV2)
    #define Master_CLKDIV2_PTR    ( (reg8 *) Master_I2C_FF__CLK_DIV2)

#else
    /* UDB implementation registers */
    #define Master_CFG_REG \
            (*(reg8 *) Master_bI2C_UDB_SyncCtl_CtrlReg__CONTROL_REG)
    #define Master_CFG_PTR \
            ( (reg8 *) Master_bI2C_UDB_SyncCtl_CtrlReg__CONTROL_REG)

    #define Master_CSR_REG        (*(reg8 *) Master_bI2C_UDB_StsReg__STATUS_REG)
    #define Master_CSR_PTR        ( (reg8 *) Master_bI2C_UDB_StsReg__STATUS_REG)

    #define Master_INT_MASK_REG   (*(reg8 *) Master_bI2C_UDB_StsReg__MASK_REG)
    #define Master_INT_MASK_PTR   ( (reg8 *) Master_bI2C_UDB_StsReg__MASK_REG)

    #define Master_INT_ENABLE_REG (*(reg8 *) Master_bI2C_UDB_StsReg__STATUS_AUX_CTL_REG)
    #define Master_INT_ENABLE_PTR ( (reg8 *) Master_bI2C_UDB_StsReg__STATUS_AUX_CTL_REG)

    #define Master_DATA_REG       (*(reg8 *) Master_bI2C_UDB_Shifter_u0__A0_REG)
    #define Master_DATA_PTR       ( (reg8 *) Master_bI2C_UDB_Shifter_u0__A0_REG)

    #define Master_GO_REG         (*(reg8 *) Master_bI2C_UDB_Shifter_u0__F1_REG)
    #define Master_GO_PTR         ( (reg8 *) Master_bI2C_UDB_Shifter_u0__F1_REG)

    #define Master_GO_DONE_REG    (*(reg8 *) Master_bI2C_UDB_Shifter_u0__A1_REG)
    #define Master_GO_DONE_PTR    ( (reg8 *) Master_bI2C_UDB_Shifter_u0__A1_REG)

    #define Master_MCLK_PRD_REG   (*(reg8 *) Master_bI2C_UDB_Master_ClkGen_u0__D0_REG)
    #define Master_MCLK_PRD_PTR   ( (reg8 *) Master_bI2C_UDB_Master_ClkGen_u0__D0_REG)

    #define Master_MCLK_CMP_REG   (*(reg8 *) Master_bI2C_UDB_Master_ClkGen_u0__D1_REG)
    #define Master_MCLK_CMP_PTR   ( (reg8 *) Master_bI2C_UDB_Master_ClkGen_u0__D1_REG)

    #if (Master_MODE_SLAVE_ENABLED)
        #define Master_ADDR_REG       (*(reg8 *) Master_bI2C_UDB_Shifter_u0__D0_REG)
        #define Master_ADDR_PTR       ( (reg8 *) Master_bI2C_UDB_Shifter_u0__D0_REG)

        #define Master_PERIOD_REG     (*(reg8 *) Master_bI2C_UDB_Slave_BitCounter__PERIOD_REG)
        #define Master_PERIOD_PTR     ( (reg8 *) Master_bI2C_UDB_Slave_BitCounter__PERIOD_REG)

        #define Master_COUNTER_REG    (*(reg8 *) Master_bI2C_UDB_Slave_BitCounter__COUNT_REG)
        #define Master_COUNTER_PTR    ( (reg8 *) Master_bI2C_UDB_Slave_BitCounter__COUNT_REG)

        #define Master_COUNTER_AUX_CTL_REG \
                                    (*(reg8 *) Master_bI2C_UDB_Slave_BitCounter__CONTROL_AUX_CTL_REG)
        #define Master_COUNTER_AUX_CTL_PTR \
                                    ( (reg8 *) Master_bI2C_UDB_Slave_BitCounter__CONTROL_AUX_CTL_REG)

    #endif /* (Master_MODE_SLAVE_ENABLED) */

#endif /* (Master_FF_IMPLEMENTED) */


/***************************************
*        Registers Constants
***************************************/

/* Master_I2C_IRQ */
#define Master_ISR_NUMBER     ((uint8) Master_I2C_IRQ__INTC_NUMBER)
#define Master_ISR_PRIORITY   ((uint8) Master_I2C_IRQ__INTC_PRIOR_NUM)

/* I2C Slave Data Register */
#define Master_SLAVE_ADDR_MASK    (0x7Fu)
#define Master_SLAVE_ADDR_SHIFT   (0x01u)
#define Master_DATA_MASK          (0xFFu)
#define Master_READ_FLAG          (0x01u)

/* Block reset constants */
#define Master_CLEAR_REG          (0x00u)
#define Master_BLOCK_RESET_DELAY  (2u)
#define Master_FF_RESET_DELAY     (Master_BLOCK_RESET_DELAY)
#define Master_RESTORE_TIMEOUT    (255u)

#if (Master_FF_IMPLEMENTED)
    /* XCFG I2C Extended Configuration Register */
    #define Master_XCFG_CLK_EN        (0x80u) /* Enable gated clock to block */
    #define Master_XCFG_I2C_ON        (0x40u) /* Enable I2C as wake up source*/
    #define Master_XCFG_RDY_TO_SLEEP  (0x20u) /* I2C ready go to sleep */
    #define Master_XCFG_FORCE_NACK    (0x10u) /* Force NACK all incoming transactions */
    #define Master_XCFG_NO_BC_INT     (0x08u) /* No interrupt on byte complete */
    #define Master_XCFG_BUF_MODE      (0x02u) /* Enable buffer mode */
    #define Master_XCFG_HDWR_ADDR_EN  (0x01u) /* Enable Hardware address match */

    /* CFG I2C Configuration Register */
    #define Master_CFG_SIO_SELECT     (0x80u) /* Pin Select for SCL/SDA lines */
    #define Master_CFG_PSELECT        (0x40u) /* Pin Select */
    #define Master_CFG_BUS_ERR_IE     (0x20u) /* Bus Error Interrupt Enable */
    #define Master_CFG_STOP_IE        (0x10u) /* Enable Interrupt on STOP condition */
    #define Master_CFG_CLK_RATE_MSK   (0x0Cu) /* Clock rate select */
    #define Master_CFG_CLK_RATE_100   (0x00u) /* Clock rate select 100K */
    #define Master_CFG_CLK_RATE_400   (0x04u) /* Clock rate select 400K */
    #define Master_CFG_CLK_RATE_050   (0x08u) /* Clock rate select 50K  */
    #define Master_CFG_CLK_RATE_RSVD  (0x0Cu) /* Clock rate select Invalid */
    #define Master_CFG_EN_MSTR        (0x02u) /* Enable Master operation */
    #define Master_CFG_EN_SLAVE       (0x01u) /* Enable Slave operation */

    #define Master_CFG_CLK_RATE_LESS_EQUAL_50 (0x04u) /* Clock rate select <= 50kHz */
    #define Master_CFG_CLK_RATE_GRATER_50     (0x00u) /* Clock rate select > 50kHz */

    /* CSR I2C Control and Status Register */
    #define Master_CSR_BUS_ERROR      (0x80u) /* Active high when bus error has occurred */
    #define Master_CSR_LOST_ARB       (0x40u) /* Set to 1 if lost arbitration in host mode */
    #define Master_CSR_STOP_STATUS    (0x20u) /* Set if Stop has been detected */
    #define Master_CSR_ACK            (0x10u) /* ACK response */
    #define Master_CSR_NAK            (0x00u) /* NAK response */
    #define Master_CSR_ADDRESS        (0x08u) /* Set in firmware 0 = status bit, 1 Address is slave */
    #define Master_CSR_TRANSMIT       (0x04u) /* Set in firmware 1 = transmit, 0 = receive */
    #define Master_CSR_LRB            (0x02u) /* Last received bit */
    #define Master_CSR_LRB_ACK        (0x00u) /* Last received bit was an ACK */
    #define Master_CSR_LRB_NAK        (0x02u) /* Last received bit was an NAK */
    #define Master_CSR_BYTE_COMPLETE  (0x01u) /* Informs that last byte has been sent */
    #define Master_CSR_STOP_GEN       (0x00u) /* Generate a stop condition */
    #define Master_CSR_RDY_TO_RD      (0x00u) /* Set to receive mode */

    /* MCSR I2C Master Control and Status Register */
    #define Master_MCSR_STOP_GEN      (0x10u) /* Firmware sets this bit to initiate a Stop condition */
    #define Master_MCSR_BUS_BUSY      (0x08u) /* Status bit, Set at Start and cleared at Stop condition */
    #define Master_MCSR_MSTR_MODE     (0x04u) /* Status bit, Set at Start and cleared at Stop condition */
    #define Master_MCSR_RESTART_GEN   (0x02u) /* Firmware sets this bit to initiate a ReStart condition */
    #define Master_MCSR_START_GEN     (0x01u) /* Firmware sets this bit to initiate a Start condition */

    /* PWRSYS_CR1 to handle Sleep */
    #define Master_PWRSYS_CR1_I2C_REG_BACKUP  (0x04u) /* Enables, power to I2C regs while sleep */

#else
    /* CONTROL REG bits location */
    #define Master_CTRL_START_SHIFT           (7u)
    #define Master_CTRL_STOP_SHIFT            (6u)
    #define Master_CTRL_RESTART_SHIFT         (5u)
    #define Master_CTRL_NACK_SHIFT            (4u)
    #define Master_CTRL_ANY_ADDRESS_SHIFT     (3u)
    #define Master_CTRL_TRANSMIT_SHIFT        (2u)
    #define Master_CTRL_ENABLE_MASTER_SHIFT   (1u)
    #define Master_CTRL_ENABLE_SLAVE_SHIFT    (0u)
    #define Master_CTRL_START_MASK            ((uint8) (0x01u << Master_CTRL_START_SHIFT))
    #define Master_CTRL_STOP_MASK             ((uint8) (0x01u << Master_CTRL_STOP_SHIFT))
    #define Master_CTRL_RESTART_MASK          ((uint8) (0x01u << Master_CTRL_RESTART_SHIFT))
    #define Master_CTRL_NACK_MASK             ((uint8) (0x01u << Master_CTRL_NACK_SHIFT))
    #define Master_CTRL_ANY_ADDRESS_MASK      ((uint8) (0x01u << Master_CTRL_ANY_ADDRESS_SHIFT))
    #define Master_CTRL_TRANSMIT_MASK         ((uint8) (0x01u << Master_CTRL_TRANSMIT_SHIFT))
    #define Master_CTRL_ENABLE_MASTER_MASK    ((uint8) (0x01u << Master_CTRL_ENABLE_MASTER_SHIFT))
    #define Master_CTRL_ENABLE_SLAVE_MASK     ((uint8) (0x01u << Master_CTRL_ENABLE_SLAVE_SHIFT))

    /* STATUS REG bits location */
    #define Master_STS_LOST_ARB_SHIFT         (6u)
    #define Master_STS_STOP_SHIFT             (5u)
    #define Master_STS_BUSY_SHIFT             (4u)
    #define Master_STS_ADDR_SHIFT             (3u)
    #define Master_STS_MASTER_MODE_SHIFT      (2u)
    #define Master_STS_LRB_SHIFT              (1u)
    #define Master_STS_BYTE_COMPLETE_SHIFT    (0u)
    #define Master_STS_LOST_ARB_MASK          ((uint8) (0x01u << Master_STS_LOST_ARB_SHIFT))
    #define Master_STS_STOP_MASK              ((uint8) (0x01u << Master_STS_STOP_SHIFT))
    #define Master_STS_BUSY_MASK              ((uint8) (0x01u << Master_STS_BUSY_SHIFT))
    #define Master_STS_ADDR_MASK              ((uint8) (0x01u << Master_STS_ADDR_SHIFT))
    #define Master_STS_MASTER_MODE_MASK       ((uint8) (0x01u << Master_STS_MASTER_MODE_SHIFT))
    #define Master_STS_LRB_MASK               ((uint8) (0x01u << Master_STS_LRB_SHIFT))
    #define Master_STS_BYTE_COMPLETE_MASK     ((uint8) (0x01u << Master_STS_BYTE_COMPLETE_SHIFT))

    /* AUX_CTL bits definition */
    #define Master_COUNTER_ENABLE_MASK        (0x20u) /* Enable 7-bit counter */
    #define Master_INT_ENABLE_MASK            (0x10u) /* Enable interrupt from status register */
    #define Master_CNT7_ENABLE                (Master_COUNTER_ENABLE_MASK)
    #define Master_INTR_ENABLE                (Master_INT_ENABLE_MASK)

#endif /* (Master_FF_IMPLEMENTED) */


/***************************************
*        Marco
***************************************/

/* ACK and NACK for data and address checks */
#define Master_CHECK_ADDR_ACK(csr)    ((Master_CSR_LRB_ACK | Master_CSR_ADDRESS) == \
                                                 ((Master_CSR_LRB    | Master_CSR_ADDRESS) &  \
                                                  (csr)))


#define Master_CHECK_ADDR_NAK(csr)    ((Master_CSR_LRB_NAK | Master_CSR_ADDRESS) == \
                                                 ((Master_CSR_LRB    | Master_CSR_ADDRESS) &  \
                                                  (csr)))

#define Master_CHECK_DATA_ACK(csr)    (0u == ((csr) & Master_CSR_LRB_NAK))

/* MCSR conditions check */
#define Master_CHECK_BUS_FREE(mcsr)       (0u == ((mcsr) & Master_MCSR_BUS_BUSY))
#define Master_CHECK_MASTER_MODE(mcsr)    (0u != ((mcsr) & Master_MCSR_MSTR_MODE))

/* CSR conditions check */
#define Master_WAIT_BYTE_COMPLETE(csr)    (0u == ((csr) & Master_CSR_BYTE_COMPLETE))
#define Master_WAIT_STOP_COMPLETE(csr)    (0u == ((csr) & (Master_CSR_BYTE_COMPLETE | \
                                                                     Master_CSR_STOP_STATUS)))
#define Master_CHECK_BYTE_COMPLETE(csr)   (0u != ((csr) & Master_CSR_BYTE_COMPLETE))
#define Master_CHECK_STOP_STS(csr)        (0u != ((csr) & Master_CSR_STOP_STATUS))
#define Master_CHECK_LOST_ARB(csr)        (0u != ((csr) & Master_CSR_LOST_ARB))
#define Master_CHECK_ADDRESS_STS(csr)     (0u != ((csr) & Master_CSR_ADDRESS))

/* Software start and end of transaction check */
#define Master_CHECK_RESTART(mstrCtrl)    (0u != ((mstrCtrl) & Master_MODE_REPEAT_START))
#define Master_CHECK_NO_STOP(mstrCtrl)    (0u != ((mstrCtrl) & Master_MODE_NO_STOP))

/* Send read or write completion depends on state */
#define Master_GET_MSTAT_CMPLT ((0u != (Master_state & Master_SM_MSTR_RD)) ? \
                                                 (Master_MSTAT_RD_CMPLT) : (Master_MSTAT_WR_CMPLT))

/* Returns 7-bit slave address */
#define Master_GET_SLAVE_ADDR(dataReg)   (((dataReg) >> Master_SLAVE_ADDR_SHIFT) & \
                                                                  Master_SLAVE_ADDR_MASK)

#if (Master_FF_IMPLEMENTED)
    /* Check enable of module */
    #define Master_I2C_ENABLE_REG     (Master_ACT_PWRMGR_REG)
    #define Master_IS_I2C_ENABLE(reg) (0u != ((reg) & Master_ACT_PWR_EN))
    #define Master_IS_ENABLED         (0u != (Master_ACT_PWRMGR_REG & Master_ACT_PWR_EN))

    #define Master_CHECK_PWRSYS_I2C_BACKUP    (0u != (Master_PWRSYS_CR1_I2C_REG_BACKUP & \
                                                                Master_PWRSYS_CR1_REG))

    /* Check start condition generation */
    #define Master_CHECK_START_GEN(mcsr)  ((0u != ((mcsr) & Master_MCSR_START_GEN)) && \
                                                     (0u == ((mcsr) & Master_MCSR_MSTR_MODE)))

    #define Master_CLEAR_START_GEN        do{ \
                                                        Master_MCSR_REG &=                                   \
                                                                           ((uint8) ~Master_MCSR_START_GEN); \
                                                    }while(0)

    /* Stop interrupt */
    #define Master_ENABLE_INT_ON_STOP     do{ \
                                                        Master_CFG_REG |= Master_CFG_STOP_IE; \
                                                    }while(0)

    #define Master_DISABLE_INT_ON_STOP    do{ \
                                                        Master_CFG_REG &=                                 \
                                                                           ((uint8) ~Master_CFG_STOP_IE); \
                                                    }while(0)

    /* Transmit data */
    #define Master_TRANSMIT_DATA          do{ \
                                                        Master_CSR_REG = Master_CSR_TRANSMIT; \
                                                    }while(0)

    #define Master_ACK_AND_TRANSMIT       do{ \
                                                        Master_CSR_REG = (Master_CSR_ACK |      \
                                                                                    Master_CSR_TRANSMIT); \
                                                    }while(0)

    #define Master_NAK_AND_TRANSMIT       do{ \
                                                        Master_CSR_REG = Master_CSR_NAK; \
                                                    }while(0)

    /* Special case: udb needs to ack, ff needs to nak */
    #define Master_ACKNAK_AND_TRANSMIT    do{ \
                                                        Master_CSR_REG  = (Master_CSR_NAK |      \
                                                                                     Master_CSR_TRANSMIT); \
                                                    }while(0)
    /* Receive data */
    #define Master_ACK_AND_RECEIVE        do{ \
                                                        Master_CSR_REG = Master_CSR_ACK; \
                                                    }while(0)

    #define Master_NAK_AND_RECEIVE        do{ \
                                                        Master_CSR_REG = Master_CSR_NAK; \
                                                    }while(0)

    #define Master_READY_TO_READ          do{ \
                                                        Master_CSR_REG = Master_CSR_RDY_TO_RD; \
                                                    }while(0)

    /* Release bus after lost arbitration */
    #define Master_BUS_RELEASE    Master_READY_TO_READ

    /* Master Start/ReStart/Stop conditions generation */
    #define Master_GENERATE_START         do{ \
                                                        Master_MCSR_REG = Master_MCSR_START_GEN; \
                                                    }while(0)

    #define Master_GENERATE_RESTART \
                    do{                       \
                        Master_MCSR_REG = (Master_MCSR_RESTART_GEN | \
                                                     Master_MCSR_STOP_GEN);    \
                        Master_CSR_REG  = Master_CSR_TRANSMIT;       \
                    }while(0)

    #define Master_GENERATE_STOP \
                    do{                    \
                        Master_MCSR_REG = Master_MCSR_STOP_GEN; \
                        Master_CSR_REG  = Master_CSR_TRANSMIT;  \
                    }while(0)

    /* Master manual APIs compatible defines */
    #define Master_GENERATE_START_MANUAL      Master_GENERATE_START
    #define Master_GENERATE_RESTART_MANUAL    Master_GENERATE_RESTART
    #define Master_GENERATE_STOP_MANUAL       Master_GENERATE_STOP
    #define Master_TRANSMIT_DATA_MANUAL       Master_TRANSMIT_DATA
    #define Master_READY_TO_READ_MANUAL       Master_READY_TO_READ
    #define Master_ACK_AND_RECEIVE_MANUAL     Master_ACK_AND_RECEIVE
    #define Master_BUS_RELEASE_MANUAL         Master_BUS_RELEASE

#else

    /* Masks to enable interrupts from Status register */
    #define Master_STOP_IE_MASK           (Master_STS_STOP_MASK)
    #define Master_BYTE_COMPLETE_IE_MASK  (Master_STS_BYTE_COMPLETE_MASK)

    /* FF compatibility: CSR register bit-fields */
    #define Master_CSR_LOST_ARB       (Master_STS_LOST_ARB_MASK)
    #define Master_CSR_STOP_STATUS    (Master_STS_STOP_MASK)
    #define Master_CSR_BUS_ERROR      (0x00u)
    #define Master_CSR_ADDRESS        (Master_STS_ADDR_MASK)
    #define Master_CSR_TRANSMIT       (Master_CTRL_TRANSMIT_MASK)
    #define Master_CSR_LRB            (Master_STS_LRB_MASK)
    #define Master_CSR_LRB_NAK        (Master_STS_LRB_MASK)
    #define Master_CSR_LRB_ACK        (0x00u)
    #define Master_CSR_BYTE_COMPLETE  (Master_STS_BYTE_COMPLETE_MASK)

    /* FF compatibility: MCSR registers bit-fields */
    #define Master_MCSR_REG           (Master_CSR_REG)  /* UDB incorporates master and slave regs */
    #define Master_MCSR_BUS_BUSY      (Master_STS_BUSY_MASK)      /* Is bus is busy               */
    #define Master_MCSR_START_GEN     (Master_CTRL_START_MASK)    /* Generate Start condition     */
    #define Master_MCSR_RESTART_GEN   (Master_CTRL_RESTART_MASK)  /* Generates RESTART condition  */
    #define Master_MCSR_MSTR_MODE     (Master_STS_MASTER_MODE_MASK)/* Define if active Master     */

    /* Data to write into TX FIFO to release FSM */
    #define Master_PREPARE_TO_RELEASE (0xFFu)
    #define Master_RELEASE_FSM        (0x00u)

    /* Define release command done: history of byte complete status is cleared */
    #define Master_WAIT_RELEASE_CMD_DONE  (Master_RELEASE_FSM != Master_GO_DONE_REG)

    /* Check enable of module */
    #define Master_I2C_ENABLE_REG     (Master_CFG_REG)
    #define Master_IS_I2C_ENABLE(reg) ((0u != ((reg) & Master_ENABLE_MASTER)) || \
                                                 (0u != ((reg) & Master_ENABLE_SLAVE)))

    #define Master_IS_ENABLED         (0u != (Master_CFG_REG & Master_ENABLE_MS))

    /* Check start condition generation */
    #define Master_CHECK_START_GEN(mcsr)  ((0u != (Master_CFG_REG &        \
                                                             Master_MCSR_START_GEN)) \
                                                    &&                                         \
                                                    (0u == ((mcsr) & Master_MCSR_MSTR_MODE)))

    #define Master_CLEAR_START_GEN        do{ \
                                                        Master_CFG_REG &=                 \
                                                        ((uint8) ~Master_MCSR_START_GEN); \
                                                    }while(0)

    /* Stop interrupt */
    #define Master_ENABLE_INT_ON_STOP     do{ \
                                                       Master_INT_MASK_REG |= Master_STOP_IE_MASK; \
                                                    }while(0)

    #define Master_DISABLE_INT_ON_STOP    do{ \
                                                        Master_INT_MASK_REG &=                               \
                                                                             ((uint8) ~Master_STOP_IE_MASK); \
                                                    }while(0)

    /* Transmit data */
    #define Master_TRANSMIT_DATA \
                                    do{    \
                                        Master_CFG_REG     = (Master_CTRL_TRANSMIT_MASK | \
                                                                       Master_CTRL_DEFAULT);        \
                                        Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE;   \
                                        Master_GO_REG      = Master_RELEASE_FSM;          \
                                    }while(0)

    #define Master_ACK_AND_TRANSMIT   Master_TRANSMIT_DATA

    #define Master_NAK_AND_TRANSMIT \
                                        do{   \
                                            Master_CFG_REG     = (Master_CTRL_NACK_MASK     | \
                                                                            Master_CTRL_TRANSMIT_MASK | \
                                                                            Master_CTRL_DEFAULT);       \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE;   \
                                            Master_GO_REG      = Master_RELEASE_FSM;          \
                                        }while(0)

    /* Receive data */
    #define Master_READY_TO_READ  \
                                        do{ \
                                            Master_CFG_REG     = Master_CTRL_DEFAULT;       \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE; \
                                            Master_GO_REG      = Master_RELEASE_FSM;       \
                                        }while(0)

    #define Master_ACK_AND_RECEIVE    Master_READY_TO_READ

    /* Release bus after arbitration is lost */
    #define Master_BUS_RELEASE    Master_READY_TO_READ

    #define Master_NAK_AND_RECEIVE \
                                        do{  \
                                            Master_CFG_REG     = (Master_CTRL_NACK_MASK |   \
                                                                            Master_CTRL_DEFAULT);     \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE; \
                                            Master_GO_REG      = Master_RELEASE_FSM;       \
                                        }while(0)

    /* Master condition generation */
    #define Master_GENERATE_START \
                                        do{ \
                                            Master_CFG_REG     = (Master_CTRL_START_MASK |  \
                                                                            Master_CTRL_DEFAULT);     \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE; \
                                            Master_GO_REG      = Master_RELEASE_FSM;       \
                                        }while(0)

    #define Master_GENERATE_RESTART \
                                        do{   \
                                            Master_CFG_REG     = (Master_CTRL_RESTART_MASK | \
                                                                            Master_CTRL_NACK_MASK    | \
                                                                            Master_CTRL_DEFAULT);      \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE;  \
                                            Master_GO_REG  =     Master_RELEASE_FSM;         \
                                        }while(0)

    #define Master_GENERATE_STOP  \
                                        do{ \
                                            Master_CFG_REG    = (Master_CTRL_NACK_MASK |    \
                                                                           Master_CTRL_STOP_MASK |    \
                                                                           Master_CTRL_DEFAULT);      \
                                            Master_GO_DONE_REG = Master_PREPARE_TO_RELEASE; \
                                            Master_GO_REG      = Master_RELEASE_FSM;        \
                                        }while(0)

    /* Master manual APIs compatible macros */
    /* These macros wait until byte complete history is cleared after command issued */
    #define Master_GENERATE_START_MANUAL \
                                        do{ \
                                            Master_GENERATE_START;                  \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)
                                        
    #define Master_GENERATE_RESTART_MANUAL \
                                        do{          \
                                            Master_GENERATE_RESTART;                \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)

    #define Master_GENERATE_STOP_MANUAL \
                                        do{       \
                                            Master_GENERATE_STOP;                   \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)

    #define Master_TRANSMIT_DATA_MANUAL \
                                        do{       \
                                            Master_TRANSMIT_DATA;                   \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)

    #define Master_READY_TO_READ_MANUAL \
                                        do{       \
                                            Master_READY_TO_READ;                   \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)

    #define Master_ACK_AND_RECEIVE_MANUAL \
                                        do{         \
                                            Master_ACK_AND_RECEIVE;                 \
                                            /* Wait until byte complete history is cleared */ \
                                            while(Master_WAIT_RELEASE_CMD_DONE)     \
                                            {                                                 \
                                            }                                                 \
                                        }while(0)

    #define Master_BUS_RELEASE_MANUAL Master_READY_TO_READ_MANUAL

#endif /* (Master_FF_IMPLEMENTED) */


/***************************************
*     Default register init constants
***************************************/

#define Master_DISABLE    (0u)
#define Master_ENABLE     (1u)

#if (Master_FF_IMPLEMENTED)
    /* Master_XCFG_REG: bits definition */
    #define Master_DEFAULT_XCFG_HW_ADDR_EN ((Master_HW_ADRR_DECODE) ? \
                                                        (Master_XCFG_HDWR_ADDR_EN) : (0u))

    #define Master_DEFAULT_XCFG_I2C_ON    ((Master_WAKEUP_ENABLED) ? \
                                                        (Master_XCFG_I2C_ON) : (0u))


    #define Master_DEFAULT_CFG_SIO_SELECT ((Master_I2C1_SIO_PAIR) ? \
                                                        (Master_CFG_SIO_SELECT) : (0u))


    /* Master_CFG_REG: bits definition */
    #define Master_DEFAULT_CFG_PSELECT    ((Master_WAKEUP_ENABLED) ? \
                                                        (Master_CFG_PSELECT) : (0u))

    #define Master_DEFAULT_CLK_RATE0  ((Master_DATA_RATE <= 50u) ?        \
                                                    (Master_CFG_CLK_RATE_050) :     \
                                                    ((Master_DATA_RATE <= 100u) ?   \
                                                        (Master_CFG_CLK_RATE_100) : \
                                                        (Master_CFG_CLK_RATE_400)))

    #define Master_DEFAULT_CLK_RATE1  ((Master_DATA_RATE <= 50u) ?           \
                                                 (Master_CFG_CLK_RATE_LESS_EQUAL_50) : \
                                                 (Master_CFG_CLK_RATE_GRATER_50))

    #define Master_DEFAULT_CLK_RATE   (Master_DEFAULT_CLK_RATE1)


    #define Master_ENABLE_MASTER      ((Master_MODE_MASTER_ENABLED) ? \
                                                 (Master_CFG_EN_MSTR) : (0u))

    #define Master_ENABLE_SLAVE       ((Master_MODE_SLAVE_ENABLED) ? \
                                                 (Master_CFG_EN_SLAVE) : (0u))

    #define Master_ENABLE_MS      (Master_ENABLE_MASTER | Master_ENABLE_SLAVE)


    /* Master_DEFAULT_XCFG_REG */
    #define Master_DEFAULT_XCFG   (Master_XCFG_CLK_EN         | \
                                             Master_DEFAULT_XCFG_I2C_ON | \
                                             Master_DEFAULT_XCFG_HW_ADDR_EN)

    /* Master_DEFAULT_CFG_REG */
    #define Master_DEFAULT_CFG    (Master_DEFAULT_CFG_SIO_SELECT | \
                                             Master_DEFAULT_CFG_PSELECT    | \
                                             Master_DEFAULT_CLK_RATE       | \
                                             Master_ENABLE_MASTER          | \
                                             Master_ENABLE_SLAVE)

    /*Master_DEFAULT_DIVIDE_FACTOR_REG */
    #define Master_DEFAULT_DIVIDE_FACTOR  ((uint16) 1u)

#else
    /* Master_CFG_REG: bits definition  */
    #define Master_ENABLE_MASTER  ((Master_MODE_MASTER_ENABLED) ? \
                                             (Master_CTRL_ENABLE_MASTER_MASK) : (0u))

    #define Master_ENABLE_SLAVE   ((Master_MODE_SLAVE_ENABLED) ? \
                                             (Master_CTRL_ENABLE_SLAVE_MASK) : (0u))

    #define Master_ENABLE_MS      (Master_ENABLE_MASTER | Master_ENABLE_SLAVE)


    #define Master_DEFAULT_CTRL_ANY_ADDR   ((Master_HW_ADRR_DECODE) ? \
                                                      (0u) : (Master_CTRL_ANY_ADDRESS_MASK))

    /* Master_DEFAULT_CFG_REG */
    #define Master_DEFAULT_CFG    (Master_DEFAULT_CTRL_ANY_ADDR)

    /* All CTRL default bits to be used in macro */
    #define Master_CTRL_DEFAULT   (Master_DEFAULT_CTRL_ANY_ADDR | Master_ENABLE_MS)

    /* Master clock generator: d0 and d1 */
    #define Master_MCLK_PERIOD_VALUE  (0x0Fu)
    #define Master_MCLK_COMPARE_VALUE (0x08u)

    /* Slave bit-counter: control period */
    #define Master_PERIOD_VALUE       (0x07u)

    /* Master_DEFAULT_INT_MASK */
    #define Master_DEFAULT_INT_MASK   (Master_BYTE_COMPLETE_IE_MASK)

    /* Master_DEFAULT_MCLK_PRD_REG */
    #define Master_DEFAULT_MCLK_PRD   (Master_MCLK_PERIOD_VALUE)

    /* Master_DEFAULT_MCLK_CMP_REG */
    #define Master_DEFAULT_MCLK_CMP   (Master_MCLK_COMPARE_VALUE)

    /* Master_DEFAULT_PERIOD_REG */
    #define Master_DEFAULT_PERIOD     (Master_PERIOD_VALUE)

#endif /* (Master_FF_IMPLEMENTED) */


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

#define Master_SSTAT_RD_ERR       (0x08u)
#define Master_SSTAT_WR_ERR       (0x80u)
#define Master_MSTR_SLAVE_BUSY    (Master_MSTR_NOT_READY)
#define Master_MSTAT_ERR_BUF_OVFL (0x80u)
#define Master_SSTAT_RD_CMPT      (Master_SSTAT_RD_CMPLT)
#define Master_SSTAT_WR_CMPT      (Master_SSTAT_WR_CMPLT)
#define Master_MODE_MULTI_MASTER_ENABLE    (Master_MODE_MULTI_MASTER_MASK)
#define Master_DATA_RATE_50       (50u)
#define Master_DATA_RATE_100      (100u)
#define Master_DEV_MASK           (0xF0u)
#define Master_SM_SL_STOP         (0x14u)
#define Master_SM_MASTER_IDLE     (0x40u)
#define Master_HDWR_DECODE        (0x01u)

#endif /* CY_I2C_Master_H */


/* [] END OF FILE */
