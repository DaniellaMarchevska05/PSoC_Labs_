#include "project.h"
#include <string.h>

/* =========================================================
   POINTERS TO PIN FUNCTIONS
   ========================================================= */

/* Column control */
static void (*Column_SetDriveMode[3])(uint8_t mode) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*Column_Write[3])(uint8_t value) = {
    COLUMN_0_Write,
    COLUMN_1_Write,
    COLUMN_2_Write
};

/* Row read */
static uint8 (*Row_Read[4])() = {
    ROW_0_Read,
    ROW_1_Read,
    ROW_2_Read,
    ROW_3_Read
};

/* =========================================================
   KEYBOARD DATA
   ========================================================= */

/* Logical mapping of keypad */
static uint8_t keyMap[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 0, 11} // * 0 #
};

/* Current state of matrix */
static uint8_t keys[4][3];

/* =========================================================
   LED MACROS (ACTIVE LOW)
   ========================================================= */

#define LED_WHITE()   do { LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); } while(0)
#define LED_BLACK()   do { LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); } while(0)
#define LED_RED()     do { LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); } while(0)
#define LED_GREEN()   do { LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); } while(0)
#define LED_BLUE()    do { LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); } while(0)
#define LED_YELLOW()  do { LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); } while(0)
#define LED_PURPLE()  do { LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); } while(0)
#define LED_CYAN()    do { LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); } while(0)

/* =========================================================
   PASSWORD SETTINGS
   ========================================================= */

#define PASSWORD_LENGTH 4

static uint8_t password[PASSWORD_LENGTH] = {1,2,3,4};
static uint8_t inputBuffer[PASSWORD_LENGTH];
static uint8_t inputIndex = 0;

/* =========================================================
   MATRIX INITIALIZATION
   ========================================================= */

static void initMatrix(void)
{
    /* Set all columns to high impedance */
    for (int column = 0; column < 3; column++)
    {
        Column_SetDriveMode[column](COLUMN_0_DM_DIG_HIZ);
    }
}

/* =========================================================
   MATRIX SCAN FUNCTION
   ========================================================= */

static void readMatrix(void)
{
    for (int column = 0; column < 3; column++)
    {
        /* Activate current column */
        Column_SetDriveMode[column](COLUMN_0_DM_STRONG);
        Column_Write[column](0);

        /* Read all rows */
        for (int row = 0; row < 4; row++)
        {
            keys[row][column] = Row_Read[row]();
        }

        /* Disable column */
        Column_SetDriveMode[column](COLUMN_0_DM_DIG_HIZ);
    }
}

/* =========================================================
   SET LED COLOR USING SWITCH-CASE
   ========================================================= */

static void setColorByKey(uint8_t key)
{
    switch (key)
    {
        case 1:
        case 7:
            LED_RED();
            break;

        case 2:
        case 8:
            LED_GREEN();
            break;

        case 3:
        case 9:
            LED_BLUE();
            break;

        case 4:
        case 10: // *
            LED_YELLOW();
            break;

        case 5:
        case 0:
            LED_PURPLE();
            break;

        case 6:
        case 11: // #
            LED_CYAN();
            break;

        default:
            break;
    }
}

/* =========================================================
   MAIN PROGRAM
   ========================================================= */

int main(void)
{
    CyGlobalIntEnable;
    SW_Tx_UART_Start();

    initMatrix();

    LED_WHITE(); // initial state

    uint8_t last_key = 255;

    for (;;)
    {
        readMatrix();

        uint8_t anyPressed = 0;

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                if (keys[row][column] == 0) // button pressed
                {
                    uint8_t key = keyMap[row][column];
                    anyPressed = 1;

                    /* Detect ONLY new press (no repeat while holding) */
                    if (key != last_key)
                    {
                        last_key = key;

                        /* UART debug */
                        SW_Tx_UART_PutString("Pressed key: ");
                        SW_Tx_UART_PutHexInt(key);
                        SW_Tx_UART_PutCRLF();

                        /* Change LED color */
                        setColorByKey(key);

                        /* Save to password buffer */
                        inputBuffer[inputIndex++] = key;

                        /* Check password */
                        if (inputIndex >= PASSWORD_LENGTH)
                        {
                            if (memcmp(inputBuffer, password, PASSWORD_LENGTH) == 0)
                                SW_Tx_UART_PutString("Access allowed\r\n");
                            else
                                SW_Tx_UART_PutString("Access denied\r\n");

                            inputIndex = 0; // reset buffer
                        }
                    }
                }
            }
        }

        /* If ALL buttons released */
        if (!anyPressed)
        {
            LED_BLACK();     // turn off LED
            last_key = 255;  // reset state
        }

        CyDelay(20); // debounce
    }
}
