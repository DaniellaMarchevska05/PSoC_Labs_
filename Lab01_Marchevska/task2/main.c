/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    for(;;)
    {
        if(Button_Read() == 1) //button off
        {
            // Violet (Red + Blue)
            LED_R_Write(0); // ON
            LED_G_Write(1); // OFF
            LED_B_Write(0); // ON
        }
        else //button on
        {
            // Yellow (Red + Green)
            LED_R_Write(0); // ON
            LED_G_Write(0); // ON
            LED_B_Write(1); // OFF
        }
    }
}

/* [] END OF FILE */
