/* 
 * File:   lcd.h
 * Author: Patrick
 *
 * Created on 23. Januar 2021, 12:04
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

// ASCII Characters 33..125 are represented by their 8-bit int value
  
#include <stdint.h>
   
#define BOOT_DELAY_US           50000
#define LEVEL_DELAY_US          20
#define CMD_DELAY_US            10000
#define DATA_HOLD_DELAY_US      500
#define DATA_OUTPUT_DELAY_US    500

#define LCD_MODE_TRUNCATE       0x00
#define LCD_MODE_WRAP           0x01
    
// Line addresses
#define LCD_LINE0_ADDR          0x00
#define LCD_LINE1_ADDR          0x40
#define LCD_LINE2_ADDR          0x14
#define LCD_LINE3_ADDR          0x54
    
// LCD Commands
#define LCD_CLEAR_DISPLAY       0x01    // Clear display and set DD-RAM Address to 0
#define LCD_RETURN_HOME         0x02    // DD_RAM Address to 0 and return to original position
#define LCD_ENTRY_MODE_SET      0x04    // Define cursor moving direction and shift during writing
#define LCD_DISPLAY_CONTROL     0x08    // Control display (switch on/off, cursor, blink)
#define LCD_CURSOR_SHIFT        0x10    // Move cursor and shift display without changing DD-RAM    
#define LCD_FUNCTION_SET        0x20    // Command to initialize LCD function
#define LCD_SET_CGRAM_ADDR      0x40    // Sets CGRAM Addr to Address Counter, can be read or written after
#define LCD_SET_DDRAM_ADDR      0x80    // Sets DDRAM Addr to Address Counter, can be read or written after
    
// LCD Entry Mode bits
#define LCD_SHIFT               0x01                      
#define LCD_NO_SHIFT            0x00
#define LCD_INCREMENT           0x02
#define LCD_DECREMENT           0x00
    
// LCD Display Control bits
#define LCD_BLINK_ON            0x01
#define LCD_BLINK_OFF           0x00
#define LCD_CURSOR_ON           0x02
#define LCD_CURSOR_OFF          0x00
#define LCD_DISPLAY_ON          0x04
#define LCD_DISPLAY_OFF         0x00
    
// LCD Cursor Shift bits
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00
    
// LCD Function Bits
#define LCD_8BIT                0x10        // Flag to set 8-bit mode
#define LCD_4BIT                0x00        // Flag to set 4-bit mode
#define LCD_1_LINE              0x00        // Flag to set one line mode
#define LCD_2_LINE              0x08        // Flag to set two line mode
#define LCD_5F8                 0x00        // Flag to set 5x8 dots character font
#define LCD_5F10                0x04        // Flag to set 5x10 character font
    
#define MAX_ROWS_SUPPORTED  2
    
// Enumerator for 4-bit or 8-bit data bus
typedef enum {LCD_BUS_WIDTH_8, LCD_BUS_WIDTH_4} lcd_bit_e;
// Enumerator for font size
typedef enum {LCD_FONT_5x8, LCD_FONT_5x10} lcd_font_e;
  
// Structure for commanded 8-bit data lines, 4-bit control lines
typedef struct{
    uint8_t data;  // 8-bit parallel data lines
    uint8_t rw;    // Read mode (1), write mode (0)
    uint8_t rs;    // Data signal (1), instruction signal(0)
    uint8_t e;     // Clock latch
    uint8_t ledk;  // LED Back light cathode
}lcd_cmd_s;
    
typedef struct{
    uint8_t address :7;
    uint8_t busy    :1;
}lcd_status_s;

typedef struct{
    uint8_t row;
    uint8_t col;
}lcd_pos_s;

// Structure for an LCD configuration
typedef struct{
    lcd_bit_e bus_width;    // Configured bus width
    lcd_font_e font;
    uint8_t rows;    // Number of rows
    uint8_t cols;    // Number of columns
    uint8_t state_display_control;
    uint8_t mode;
}lcd_config_s;

// Function pointer to write callback
typedef bool (*IF_Write_Fcn)(void *interface_config, lcd_cmd_s lcd_cmd);
typedef uint8_t (*IF_Read_Fcn)(void *interface_config);

typedef struct{
    void *config;           // Generic pointer to configuration used by the interface
    IF_Write_Fcn write_fun; // Function pointer to write callback function of interface     
    IF_Read_Fcn read_fun;   // Function pointer to read callback function of interface
}interface_s;

// Initialize the LCD
int lcd_configure(lcd_config_s *config, lcd_bit_e bus_width, lcd_font_e font, uint8_t rows, uint8_t cols, uint8_t mode);
void lcd_init(lcd_config_s *config, interface_s * interface);

/* High level functions for user */

void wait_busy(const lcd_config_s *config, interface_s *interface);
void lcd_clear(const lcd_config_s *config, interface_s *interface);
void lcd_home(const lcd_config_s *config, interface_s *interface);

// Power
void lcd_power_on(interface_s *interface);
void lcd_power_off(interface_s *interface);

// Display
void lcd_display_on(lcd_config_s *config, interface_s *interface);
void lcd_display_off(lcd_config_s *config, interface_s *interface);
void lcd_blink_on(lcd_config_s *config, interface_s *interface);
void lcd_blink_off(lcd_config_s *config, interface_s *interface);

/* DDRAM Addresses are moved when whole display is moved */
void lcd_mv_right(const lcd_config_s *config, interface_s *interface);
void lcd_mv_left(const lcd_config_s *config, interface_s *interface);

// Cursor
void lcd_cursor_on(lcd_config_s *config, interface_s *interface);
void lcd_cursor_off(lcd_config_s *config, interface_s *interface);
void lcd_mv_cursor_right(const lcd_config_s *config, interface_s *interface);
void lcd_mv_cursor_left(const lcd_config_s *config, interface_s *interface);
void lcd_mv_cursor(const lcd_config_s *config, interface_s *interface, uint8_t row, uint8_t col);
lcd_pos_s lcd_get_cursor(const lcd_config_s *config, interface_s *interface);

// Print functions
void lcd_putc(const lcd_config_s *config, interface_s *interface, char c);
void lcd_printf(const lcd_config_s *config, interface_s *interface, char *s);
void lcd_printf_at(const lcd_config_s *config, interface_s *interface, char *s, uint8_t row, uint8_t col);
char lcd_getc(const lcd_config_s *config, interface_s *interface);

// LCD status
lcd_status_s lcd_get_status(const lcd_config_s *config, interface_s *interface);

void lcd_create_custom(const lcd_config_s *config, interface_s *interface, uint8_t addr, uint8_t *character);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

