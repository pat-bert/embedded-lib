/*******************************************************************************
  Main Source File

  Example for driving LCD via PCF8574 on Atmel SAMD10, exchange I2C-functions to use

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#define DEBUG
#include <stddef.h>                     // Defines NULL
#include "definitions.h"                // SYS function prototypes
#include "lcd.h"
#include "PCF8574.h"

#define PCF8574_I2C_ADDR    0x27

bool I2C_Write(uint16_t address, uint8_t* wrData, uint32_t wrLength){
    bool ret;
    ret = SERCOM1_I2C_Write(address, wrData, wrLength);
    while(SERCOM1_I2C_IsBusy()){;}
    return ret;
}

bool I2C_Read(uint16_t address, uint8_t* rdData, uint32_t rdLength){
    bool ret;
    ret = SERCOM1_I2C_Read(address, rdData, rdLength);
    while(SERCOM1_I2C_IsBusy()){;}
    return ret;
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    // Configuration variables
    lcd_config_s lcd_config;
    pcf8574_config_s expander_config;
    interface_s lcd_interface;
    int err;
    
    // Initialize the LCD configuration
    err = lcd_configure(&lcd_config, LCD_BUS_WIDTH_4, LCD_FONT_5x8, 2, 16, LCD_MODE_WRAP);
    if (err == -1)
        return 1;
    
    // Initialize the GPIO-Expander configuration
    pcf8574_configure(&expander_config, PCF8574_I2C_ADDR, &I2C_Write, &I2C_Read);
        
    // Configure LCD interface
    lcd_interface.config = &expander_config;
    lcd_interface.write_fun = &pcf8574_lcd_if_write;
    lcd_interface.read_fun = &pcf8574_lcd_if_read;
    
    // Initialize LCD with selected configuration
    lcd_init(&lcd_config, &lcd_interface);
    lcd_blink_on(&lcd_config, &lcd_interface);
    lcd_cursor_on(&lcd_config, &lcd_interface);
    
    lcd_printf_at(&lcd_config, &lcd_interface, "Text is continued below...", 0, 0);
    
    while ( 1 )
    {
    
    }
    
    return 1;
}
