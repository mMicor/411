/*******************************************************************************
* File Name: Poll_intr.h
* Version 1.71
*
*  Description:
*   Provides the function definitions for the Interrupt Controller.
*
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_ISR_Poll_intr_H)
#define CY_ISR_Poll_intr_H


#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void Poll_intr_Start(void);
void Poll_intr_StartEx(cyisraddress address);
void Poll_intr_Stop(void);

CY_ISR_PROTO(Poll_intr_Interrupt);

void Poll_intr_SetVector(cyisraddress address);
cyisraddress Poll_intr_GetVector(void);

void Poll_intr_SetPriority(uint8 priority);
uint8 Poll_intr_GetPriority(void);

void Poll_intr_Enable(void);
uint8 Poll_intr_GetState(void);
void Poll_intr_Disable(void);

void Poll_intr_SetPending(void);
void Poll_intr_ClearPending(void);


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the Poll_intr ISR. */
#define Poll_intr_INTC_VECTOR            ((reg32 *) Poll_intr__INTC_VECT)

/* Address of the Poll_intr ISR priority. */
#define Poll_intr_INTC_PRIOR             ((reg8 *) Poll_intr__INTC_PRIOR_REG)

/* Priority of the Poll_intr interrupt. */
#define Poll_intr_INTC_PRIOR_NUMBER      Poll_intr__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable Poll_intr interrupt. */
#define Poll_intr_INTC_SET_EN            ((reg32 *) Poll_intr__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the Poll_intr interrupt. */
#define Poll_intr_INTC_CLR_EN            ((reg32 *) Poll_intr__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the Poll_intr interrupt state to pending. */
#define Poll_intr_INTC_SET_PD            ((reg32 *) Poll_intr__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the Poll_intr interrupt. */
#define Poll_intr_INTC_CLR_PD            ((reg32 *) Poll_intr__INTC_CLR_PD_REG)


#endif /* CY_ISR_Poll_intr_H */


/* [] END OF FILE */
