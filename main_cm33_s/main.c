/******************************************************************************
* File Name:   main.c
*
* Description: This is the main  core source code for the GPIO toggling
*              using the EPU constant_in functionality for ModusToolbox.
*
* Related Document: See README.md
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cycfg.h"
#include "cy_system_ppca_init.h"
#include "cybsp.h"
#include <stdio.h>
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START
#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE



/*******************************************************************************
* Global Variables
********************************************************************************/
/* For the Retarget-IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug UART HAL object */

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CPU.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize and Enable the UART for SERIAL logs */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize redirecting of low level IO */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget IO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable the PPCA_CNFG to connect the EPU to IO */
    Cy_PPCA_CNFG_Init(CNFG_PPCA_INOUT_HW, &CNFG_PPCA_INOUT_config);
    Cy_PPCA_Enable(CNFG_PPCA_INOUT_HW);

    /* EPU configuration and Enable */
    Cy_PPCA_EPU_EnableExclusiveAccess(EPU_BLK_HW, true);
    Cy_PPCA_EPU_Enable(EPU_BLK_HW);

    /* Configure Processing unit type2 (PUT_2) to generate the constant output */
    Cy_PPCA_EPU_PU_T2_Configure(PUT_IO_HW, PUT_IO_INDEX, &PUT_IO_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PUT_IO_HW, PUT_IO_INDEX, PUT_IO_ENABLE_MODE);

    /* Configure Combiner to connect the PUT_2 output to external GPIO (P4.0 LED6) */
    Cy_PPCA_EPU_Combo_Configure(COMBINER_IO_HW, COMBINER_IO_INDEX, &COMBINER_IO_combo_config);

    /* Configure Processing unit type2 (PUT_2) to generate the constant output */
    Cy_PPCA_EPU_PU_T2_Configure(PUT1_IO_HW, PUT1_IO_INDEX, &PUT1_IO_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PUT1_IO_HW, PUT1_IO_INDEX, PUT1_IO_ENABLE_MODE);

    /* Configure Combiner to connect the PUT_2 output to external GPIO (P1.5 LED1) */
    Cy_PPCA_EPU_Combo_Configure(COMBINER1_IO_HW, COMBINER1_IO_INDEX, &COMBINER1_IO_combo_config);

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf( "\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Direct GPIO control via PPCA cores\r\n");
    printf("************************************************************\r\n\n");

    /* enable interrupts */
    __enable_irq();

    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS,PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS,PPCA1_IMAGE_SIZE);

    printf("PPCA CM33 cores Boot called\n");

    for (;;)
    {
    }
}
