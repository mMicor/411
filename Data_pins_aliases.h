/*******************************************************************************
* File Name: Data_pins.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Data_pins_ALIASES_H) /* Pins Data_pins_ALIASES_H */
#define CY_PINS_Data_pins_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define Data_pins_0			(Data_pins__0__PC)
#define Data_pins_0_INTR	((uint16)((uint16)0x0001u << Data_pins__0__SHIFT))

#define Data_pins_1			(Data_pins__1__PC)
#define Data_pins_1_INTR	((uint16)((uint16)0x0001u << Data_pins__1__SHIFT))

#define Data_pins_INTR_ALL	 ((uint16)(Data_pins_0_INTR| Data_pins_1_INTR))
#define Data_pins_scl			(Data_pins__scl__PC)
#define Data_pins_scl_INTR	((uint16)((uint16)0x0001u << Data_pins__0__SHIFT))

#define Data_pins_sda			(Data_pins__sda__PC)
#define Data_pins_sda_INTR	((uint16)((uint16)0x0001u << Data_pins__1__SHIFT))

#endif /* End Pins Data_pins_ALIASES_H */


/* [] END OF FILE */
