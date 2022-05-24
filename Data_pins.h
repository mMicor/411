/*******************************************************************************
* File Name: Data_pins.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Data_pins_H) /* Pins Data_pins_H */
#define CY_PINS_Data_pins_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Data_pins_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Data_pins__PORT == 15 && ((Data_pins__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Data_pins_Write(uint8 value);
void    Data_pins_SetDriveMode(uint8 mode);
uint8   Data_pins_ReadDataReg(void);
uint8   Data_pins_Read(void);
void    Data_pins_SetInterruptMode(uint16 position, uint16 mode);
uint8   Data_pins_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Data_pins_SetDriveMode() function.
     *  @{
     */
        #define Data_pins_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Data_pins_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Data_pins_DM_RES_UP          PIN_DM_RES_UP
        #define Data_pins_DM_RES_DWN         PIN_DM_RES_DWN
        #define Data_pins_DM_OD_LO           PIN_DM_OD_LO
        #define Data_pins_DM_OD_HI           PIN_DM_OD_HI
        #define Data_pins_DM_STRONG          PIN_DM_STRONG
        #define Data_pins_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Data_pins_MASK               Data_pins__MASK
#define Data_pins_SHIFT              Data_pins__SHIFT
#define Data_pins_WIDTH              2u

/* Interrupt constants */
#if defined(Data_pins__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Data_pins_SetInterruptMode() function.
     *  @{
     */
        #define Data_pins_INTR_NONE      (uint16)(0x0000u)
        #define Data_pins_INTR_RISING    (uint16)(0x0001u)
        #define Data_pins_INTR_FALLING   (uint16)(0x0002u)
        #define Data_pins_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Data_pins_INTR_MASK      (0x01u) 
#endif /* (Data_pins__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Data_pins_PS                     (* (reg8 *) Data_pins__PS)
/* Data Register */
#define Data_pins_DR                     (* (reg8 *) Data_pins__DR)
/* Port Number */
#define Data_pins_PRT_NUM                (* (reg8 *) Data_pins__PRT) 
/* Connect to Analog Globals */                                                  
#define Data_pins_AG                     (* (reg8 *) Data_pins__AG)                       
/* Analog MUX bux enable */
#define Data_pins_AMUX                   (* (reg8 *) Data_pins__AMUX) 
/* Bidirectional Enable */                                                        
#define Data_pins_BIE                    (* (reg8 *) Data_pins__BIE)
/* Bit-mask for Aliased Register Access */
#define Data_pins_BIT_MASK               (* (reg8 *) Data_pins__BIT_MASK)
/* Bypass Enable */
#define Data_pins_BYP                    (* (reg8 *) Data_pins__BYP)
/* Port wide control signals */                                                   
#define Data_pins_CTL                    (* (reg8 *) Data_pins__CTL)
/* Drive Modes */
#define Data_pins_DM0                    (* (reg8 *) Data_pins__DM0) 
#define Data_pins_DM1                    (* (reg8 *) Data_pins__DM1)
#define Data_pins_DM2                    (* (reg8 *) Data_pins__DM2) 
/* Input Buffer Disable Override */
#define Data_pins_INP_DIS                (* (reg8 *) Data_pins__INP_DIS)
/* LCD Common or Segment Drive */
#define Data_pins_LCD_COM_SEG            (* (reg8 *) Data_pins__LCD_COM_SEG)
/* Enable Segment LCD */
#define Data_pins_LCD_EN                 (* (reg8 *) Data_pins__LCD_EN)
/* Slew Rate Control */
#define Data_pins_SLW                    (* (reg8 *) Data_pins__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Data_pins_PRTDSI__CAPS_SEL       (* (reg8 *) Data_pins__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Data_pins_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Data_pins__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Data_pins_PRTDSI__OE_SEL0        (* (reg8 *) Data_pins__PRTDSI__OE_SEL0) 
#define Data_pins_PRTDSI__OE_SEL1        (* (reg8 *) Data_pins__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Data_pins_PRTDSI__OUT_SEL0       (* (reg8 *) Data_pins__PRTDSI__OUT_SEL0) 
#define Data_pins_PRTDSI__OUT_SEL1       (* (reg8 *) Data_pins__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Data_pins_PRTDSI__SYNC_OUT       (* (reg8 *) Data_pins__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Data_pins__SIO_CFG)
    #define Data_pins_SIO_HYST_EN        (* (reg8 *) Data_pins__SIO_HYST_EN)
    #define Data_pins_SIO_REG_HIFREQ     (* (reg8 *) Data_pins__SIO_REG_HIFREQ)
    #define Data_pins_SIO_CFG            (* (reg8 *) Data_pins__SIO_CFG)
    #define Data_pins_SIO_DIFF           (* (reg8 *) Data_pins__SIO_DIFF)
#endif /* (Data_pins__SIO_CFG) */

/* Interrupt Registers */
#if defined(Data_pins__INTSTAT)
    #define Data_pins_INTSTAT            (* (reg8 *) Data_pins__INTSTAT)
    #define Data_pins_SNAP               (* (reg8 *) Data_pins__SNAP)
    
	#define Data_pins_0_INTTYPE_REG 		(* (reg8 *) Data_pins__0__INTTYPE)
	#define Data_pins_1_INTTYPE_REG 		(* (reg8 *) Data_pins__1__INTTYPE)
#endif /* (Data_pins__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Data_pins_H */


/* [] END OF FILE */
