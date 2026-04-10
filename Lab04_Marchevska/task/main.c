#include "project.h"

static uint8_t LED_NUM[] = {
    0xC0, //0
    0xF9, //1
    0xA4, //2
    0xB0, //3
    0x99, //4
    0x92, //5
    0x82, //6
    0xF8, //7
    0x80, //8
    0x90, //9
    0xBF, //-
    0x88, //A
    0x83, //b
    0xC6, //C
    0xA1, //d
    0x86, //E
    0x8E, //F
    0x7F  //.
};

static void FourDigit74HC595_sendData(uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        if(data & (0x80 >> i))
            Pin_DO_Write(1);
        else
            Pin_DO_Write(0);

        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot)
{
    if(position >= 8)
    {
        FourDigit74HC595_sendData(0xFF);
        FourDigit74HC595_sendData(0xFF);
    }

    FourDigit74HC595_sendData(0xFF & ~(1 << position));

    if(dot)
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F);
    else
        FourDigit74HC595_sendData(LED_NUM[digit]);

    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

static uint8_t keys[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 0, 11}, // * 0 #
};

static uint8_t getPressedButton(void);
static void displayValue(uint8_t btn, uint8_t mode);
static void setColor(uint8_t btn);

static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8_t value) = {
    COLUMN_0_Write,
    COLUMN_1_Write,
    COLUMN_2_Write
};

static uint8 (*ROW_x_Read[4])() = {
    ROW_0_Read,
    ROW_1_Read,
    ROW_2_Read,
    ROW_3_Read
};

static void initMatrix()
{
    for(int i = 0; i < 3; i++)
        COLUMN_x_SetDriveMode[i](COLUMN_0_DM_DIG_HIZ);
}

static void readMatrix()
{
    for(int col = 0; col < 3; col++)
    {
        COLUMN_x_SetDriveMode[col](COLUMN_0_DM_STRONG);
        COLUMN_x_Write[col](0);   

        for(int row = 0; row < 4; row++)
        {
            keys[row][col] = ROW_x_Read[row]();
        }

        COLUMN_x_SetDriveMode[col](COLUMN_0_DM_DIG_HIZ);
    }
}

static uint8_t getPressedButton()
{
    for(int row = 0; row < 4; row++)
    {
        for(int col = 0; col < 3; col++)
        {
            if(keys[row][col] == 0)
            {
                if(row == 3 && col == 0) return 10; // *
                if(row == 3 && col == 1) return 0;  // 0
                if(row == 3 && col == 2) return 11; // #
                return row * 3 + col + 1;
            }
        }
    }
    return 255;
}

static void displayValue(uint8_t btn, uint8_t mode)
{
    if(mode == 0) 
    {
        if(btn <= 9)
        {
            FourDigit74HC595_sendOneDigit(0, btn, 0);

            /* Вивід у UART що зараз показано */
            SW_Tx_UART_PutString("Display: ");
            SW_Tx_UART_PutChar('0' + btn);
            SW_Tx_UART_PutString("\r\n");
        }
    }
    else // букви
    {
        switch(btn)
        {
            case 1:
                FourDigit74HC595_sendOneDigit(0, 11, 0); // A
                SW_Tx_UART_PutString("Display: A\r\n");
                break;

            case 2:
                FourDigit74HC595_sendOneDigit(0, 12, 0); // b
                SW_Tx_UART_PutString("Display: b\r\n");
                break;

            case 3:
                FourDigit74HC595_sendOneDigit(0, 13, 0); // C
                SW_Tx_UART_PutString("Display: C\r\n");
                break;

            case 4:
                FourDigit74HC595_sendOneDigit(0, 14, 0); // d
                SW_Tx_UART_PutString("Display: d\r\n");
                break;

            case 5:
                FourDigit74HC595_sendOneDigit(0, 15, 0); // E
                SW_Tx_UART_PutString("Display: E\r\n");
                break;

            case 6:
                FourDigit74HC595_sendOneDigit(0, 16, 0); // F
                SW_Tx_UART_PutString("Display: F\r\n");
                break;

            default:
                break;
        }
    }
}

static void setColor(uint8_t btn)
{
    switch(btn)
    {
        case 1: LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); break;
        case 2: LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); break;
        case 3: LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); break;
        case 4: LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); break;//жовтий
        case 5: LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); break;//ціановий
        case 6: LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); break;//фіолетовий
        case 7: LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); break;//жовтий
        case 8: LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); break;//ціановий
        case 9: LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); break;//фіолетовий
        case 0: LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); break;//білий
        case 10: LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); break; // *
        default:
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(1);
    }
}


int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    initMatrix();

    uint8_t last = 255;
    uint8_t mode = 0; // 0 цифри, 1 букви

    for(;;)
    {
        readMatrix();
        uint8_t btn = getPressedButton();

        if(btn != 255 && btn != last)
        {
            last = btn;

            if(btn == 11) // #
            {
                mode = !mode;
                SW_Tx_UART_PutString(mode ? "LETTERS\r\n" : "DIGITS\r\n");
            }
            else
            {
                displayValue(btn, mode);
                setColor(btn);
            }
        }

        if(btn == 255)
            last = 255;

        CyDelay(50);
    }
}
