#define F_CPU 8000000UL

#include <stdbool.h>

#include "lcd.h"
#include "delay.h"
#include "definitions.h"

/* Low-level functions */

static void lcd_write(const lcd_config_s *config, interface_s *interface, uint8_t cmd, uint8_t is_data){
    // Empty LCD command
    lcd_cmd_s lcd_cmd;
    lcd_cmd.data = 0x00;
    lcd_cmd.ledk = 1;
    lcd_cmd.rs = is_data;
    lcd_cmd.rw = 0;
    lcd_cmd.e = 1;
    
    if (config->bus_width == LCD_BUS_WIDTH_4){
        // Set control lines
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        lcd_cmd.e = 1;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        
        // Send upper nibble
        lcd_cmd.data = cmd & 0xf0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(LEVEL_DELAY_US);
        
        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
        
        // Send lower nibble
        lcd_cmd.e = 1;
        lcd_cmd.data = (cmd & 0x0f) << 4;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(LEVEL_DELAY_US);
        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
    }
    else{
        // Send command word at once
        lcd_cmd.data = cmd;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(LEVEL_DELAY_US);
        
        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
    }
    
    delay_usec(CMD_DELAY_US);
}

static uint8_t lcd_read(const lcd_config_s *config, interface_s *interface, uint8_t is_data){
    // Empty LCD command
    lcd_cmd_s lcd_cmd;
    lcd_cmd.data = 0xff;
    lcd_cmd.ledk = 1;
    lcd_cmd.rs = is_data;
    lcd_cmd.rw = 1;
    lcd_cmd.e = 1;
    uint8_t read_value;

    if (config->bus_width == LCD_BUS_WIDTH_4){
        // Set control lines
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        lcd_cmd.e = 1;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        
        // Read upper nibble
        read_value = interface->read_fun(interface->config) & 0xF0;
        
        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
        
        // Set control lines
        lcd_cmd.e = 1;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        
        // Read lower nibble
        read_value |= (interface->read_fun(interface->config) & 0xF0) >> 4;

        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
    }
    else{
        // Set control lines
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_OUTPUT_DELAY_US);
        
        // Read register at once
        read_value = interface->read_fun(interface->config);
        
        // High-low transition on Enable bit
        lcd_cmd.e = 0;
        interface->write_fun(interface->config, lcd_cmd);
        delay_usec(DATA_HOLD_DELAY_US);
    }
    return read_value;
}

/* Misc. functions */

void lcd_clear(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_CLEAR_DISPLAY, 0);
    wait_busy(config, interface);
}

void lcd_home(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_RETURN_HOME, 0);
    wait_busy(config, interface);
}

void wait_busy(const lcd_config_s *config, interface_s *interface){
    lcd_status_s status;  
    do{
        status = lcd_get_status(config, interface);
    } while(status.busy);
}

/* Setup functions */

int lcd_configure(lcd_config_s *config, lcd_bit_e bus_width, lcd_font_e font, uint8_t rows, uint8_t cols, uint8_t mode){
    // Assign correct bus type
    switch(bus_width){
        // Intentional fall-through for all valid cases
        case LCD_BUS_WIDTH_8:
        case LCD_BUS_WIDTH_4:
            config->bus_width = bus_width;
            break;
        default:
            // Unknown bit type, error code
            return -1;
    }
    
    // Assign correct font type
    switch(font){
        // Intentional fall-through for all valid cases
        case LCD_FONT_5x8:
        case LCD_FONT_5x10:
            config->font = font;
            break;
        default:
            // Unknown font type
            return -1;
    }
    
    // Assign rows and columns
    if (rows > 0 && rows <= MAX_ROWS_SUPPORTED && cols > 0){
        config->rows = rows;
        config->cols = cols;
    }  
    else 
        return -1;
    
    // Assign default states
    config->state_display_control = LCD_DISPLAY_ON;
    config->mode = mode;
    
    // All good
    return 0;
}

void lcd_init(lcd_config_s *config, interface_s * interface){  
    uint8_t config_cmd = LCD_FUNCTION_SET;
    lcd_bit_e original_bus_width = config->bus_width;
    
    // Send 3x 0x30 with delays in between in 8-bit mode first
    // Delays are necessary because busy flag is still unavailable
    config->bus_width = LCD_BUS_WIDTH_8;
    delay_usec(BOOT_DELAY_US);
    lcd_write(config, interface, LCD_FUNCTION_SET | LCD_8BIT, 0);
    delay_usec(BOOT_DELAY_US/5);
    lcd_write(config, interface, LCD_FUNCTION_SET | LCD_8BIT, 0);
    delay_usec(BOOT_DELAY_US/10);
    lcd_write(config, interface, LCD_FUNCTION_SET | LCD_8BIT, 0);
    
    // Set bus width
    if (original_bus_width == LCD_BUS_WIDTH_8)
        config_cmd |= LCD_8BIT;
    else
        config_cmd |= LCD_4BIT;
          
    // Set the mode
    wait_busy(config, interface);
    lcd_write(config, interface, config_cmd, 0);
    
    // Reset bus to original bus width
    config->bus_width = original_bus_width;
    
    // Set row configuration
    if (config->rows == 1)
        config_cmd |= LCD_1_LINE;
    else
        config_cmd |= LCD_2_LINE;

    // Set font configuration
    if (config->font == LCD_FONT_5x8)
        config_cmd |= LCD_5F8;
    else
        config_cmd |= LCD_5F10;
   
    // Send the configuration command
    wait_busy(config, interface);
    lcd_write(config, interface, config_cmd, 0);

    // Display on, no cursor, no blink
    wait_busy(config, interface);
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | LCD_CURSOR_OFF | LCD_BLINK_OFF | LCD_DISPLAY_ON, 0);
    
    // Clear display
    wait_busy(config, interface);
    lcd_write(config, interface, LCD_CLEAR_DISPLAY, 0);
    
    // Entry mode set
    wait_busy(config, interface);
    lcd_write(config, interface, LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NO_SHIFT, 0);
    
    // Home LCD
    wait_busy(config, interface);
    lcd_write(config, interface, LCD_RETURN_HOME, 0);
    wait_busy(config, interface);
}

/* High level commands for user */

// Power switching
void lcd_power_on(interface_s *interface){
    // Empty LCD command, no E pulse, only cathode
    lcd_cmd_s lcd_cmd;
    lcd_cmd.data = 0x00;
    lcd_cmd.ledk = 1;
    lcd_cmd.rs = 0;
    lcd_cmd.rw = 0;
    lcd_cmd.e = 0;
    interface->write_fun(interface->config, lcd_cmd);
}

void lcd_power_off(interface_s *interface){
    // Empty LCD command, no E pulse, only cathode
    lcd_cmd_s lcd_cmd;
    lcd_cmd.data = 0x00;
    lcd_cmd.ledk = 0;
    lcd_cmd.rs = 0;
    lcd_cmd.rw = 0;
    lcd_cmd.e = 0;
    interface->write_fun(interface->config, lcd_cmd);
}

// Display functions
void lcd_display_on(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control |= LCD_DISPLAY_ON;
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_display_off(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control &= (~LCD_DISPLAY_ON);
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_blink_on(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control |= LCD_BLINK_ON;
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_blink_off(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control &= (~LCD_BLINK_ON);
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_mv_right(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_CURSOR_SHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, 0);
}

void lcd_mv_left(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_CURSOR_SHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, 0);
}

// Cursor functions
void lcd_cursor_on(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control |= LCD_CURSOR_ON;
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_cursor_off(lcd_config_s *config, interface_s *interface){
    // Update state
    config->state_display_control &= (~LCD_CURSOR_ON);
    // Use state
    lcd_write(config, interface, LCD_DISPLAY_CONTROL | config->state_display_control, 0);
}

void lcd_mv_cursor_right(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_CURSOR_SHIFT | LCD_MOVERIGHT, 0);
}

void lcd_mv_cursor_left(const lcd_config_s *config, interface_s *interface){
    lcd_write(config, interface, LCD_CURSOR_SHIFT | LCD_MOVELEFT, 0);
}

void lcd_mv_cursor(const lcd_config_s *config, interface_s *interface, uint8_t row, uint8_t col){
    // No write possible if target is outside of specified LCD area
    if (col >= config->cols || row >= config->rows)
        return;
    
    // Calculate target address (7-bit, 8th bit is always set to indicate command)
    uint8_t addr = LCD_SET_DDRAM_ADDR;
    
    // Switch for line start addresses since they are not continuous in memory
    switch(row){
        case 0:
            addr |= (LCD_LINE0_ADDR + col);
            break;
        case 1:
            addr |= (LCD_LINE1_ADDR + col);
            break;
        case 2:
            addr |= (LCD_LINE2_ADDR + col);
            break;
        case 3:
            addr |= (LCD_LINE3_ADDR + col);
            break;
        default:
            return;
    }
    
    // Set display address
    lcd_write(config, interface, addr, 0);
}

lcd_pos_s lcd_get_cursor(const lcd_config_s *config, interface_s *interface){
    lcd_pos_s curr_pos;
    
    // Get value of address counter
    lcd_status_s lcd_status = lcd_get_status(config, interface);
    
    // Derive rows and columns from address
    switch(config->rows){
        case 2:
            curr_pos.row = lcd_status.address >= LCD_LINE1_ADDR;
            curr_pos.col = lcd_status.address - curr_pos.row * LCD_LINE1_ADDR;
            break;
        case 3:
            // Intentional fall-through
        case 4:
            if (config->rows > 3 && lcd_status.address >= LCD_LINE3_ADDR){
                curr_pos.row = 3;
                curr_pos.col = lcd_status.address - LCD_LINE3_ADDR;
            }
            else if (lcd_status.address >= LCD_LINE2_ADDR){
                curr_pos.row = 2;
                curr_pos.col = lcd_status.address - LCD_LINE2_ADDR;
            }
            else if (lcd_status.address >= LCD_LINE1_ADDR){
                curr_pos.row = 1;
                curr_pos.col = lcd_status.address - LCD_LINE1_ADDR;
            }
            else{
                curr_pos.row = 0;
                curr_pos.col = lcd_status.address;
            }
            break;
        case 1:
            // Intentional fall-through
        default:
            // Row is always one
            curr_pos.row = 0;
            curr_pos.col = lcd_status.address;
    }    
    return curr_pos;
}

// Print functions
void lcd_putc(const lcd_config_s *config, interface_s *interface, char c){
    lcd_write(config, interface, (uint8_t) c, 1);
}

void lcd_printf(const lcd_config_s *config, interface_s *interface, char *s){
    // Maximum string length is rows * columns of the display
    uint16_t size = config->rows * config->cols;
    lcd_pos_s init_pos = lcd_get_cursor(config, interface);
    uint8_t counter = 0;
    
    // Print characters one by one
    // Stop after rows*cols characters or at null terminator
    for(char *c = s; *c != '\0' && counter < size; c++, counter++){        
        // Wrap cursor to next line
        if (config->mode == LCD_MODE_WRAP && (init_pos.col + counter) >= config->cols){
            // Stop if last row is reached, no further wrap possible
            init_pos.row++;
            if (init_pos.row >= config->rows)
                return;
            
            // Reset counter for next row and go to start of next row
            counter = 0;
            lcd_mv_cursor(config, interface, init_pos.row, 0);
        }
        
        // Print next character at current cursor position
        // Cursor is automatically incremented
        lcd_putc(config, interface, *c);
    }
}

void lcd_printf_at(const lcd_config_s *config, interface_s *interface, char *s, uint8_t row, uint8_t col){
    lcd_mv_cursor(config, interface, row, col);
    lcd_printf(config, interface, s);
}

char lcd_getc(const lcd_config_s *config, interface_s *interface){
    return (char) lcd_read(config, interface, 1);
}

// LCD Status
lcd_status_s lcd_get_status(const lcd_config_s *config, interface_s *interface){
    uint8_t temp;
    lcd_status_s status;
    temp = lcd_read(config, interface, 0);
    status.address = temp & ~(1 << 7);
    status.busy = temp & (1 << 7);
    return status;    
}

// Special characters
void lcd_create_custom(const lcd_config_s *config, interface_s *interface, uint8_t addr, uint8_t *character){
    int max_row;
       
    // Get number of rows to write to CGRAM
    switch (config->font){
        case LCD_FONT_5x10:
            // Only four characters possible
            if (addr >= 0x04)
                return;
            addr <<= 4;
            max_row = 10;
            break;
        case LCD_FONT_5x8:
            // Intentional fall-through
        default:
            // Only eight characters possible
            if (addr >= 0x08)
                return;
            addr <<= 3;
            max_row = 8;
    }
    
    // Set initial CGRAM address
    lcd_write(config, interface, LCD_SET_CGRAM_ADDR | addr, 0);
    
    // Write data to CGRAM address
    // CGRAM address is incremented automatically
    for (int row=0; row < max_row; row++, character++){
        lcd_write(config, interface, *character, 1);
    }
}
