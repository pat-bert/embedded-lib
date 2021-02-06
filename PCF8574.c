#include <stdint.h>
#include <stdio.h>
#include "PCF8574.h"
#include "definitions.h"

void pcf8574_configure(pcf8574_config_s *config, uint16_t i2c_addr, I2C_Fcn write_fun, I2C_Fcn read_fun){
    // Save the I2C address of the device
    config->i2c_addr = i2c_addr;
    // Save a function pointer to the write function for the corresponding I2C bus
    config->i2c_write_fun = write_fun;
    config->i2c_read_fun = read_fun;
    config->rd_buffer = 0x00;
    config->wr_buffer = 0x00;
}

bool pcf8574_write(pcf8574_config_s *config, uint8_t data){
    // Save data to buffer
    config->wr_buffer = data;
    // Write the data via the configured I2C function
    return config->i2c_write_fun(config->i2c_addr, &(config->wr_buffer), 1);
}

bool pcf8574_read(pcf8574_config_s *config, uint8_t mask){
    bool ret;
    // Assert pins with mask
    // Pins are open-collector and need to be switched to high to be able to read
    // Maintain pins that are high anyway by OR-combination with the mask
    
    if ((config->wr_buffer | mask) != config->wr_buffer){
        ret = pcf8574_write(config, config->wr_buffer | mask);
        if (!ret)
            return ret;
    }

    // Data is received via I2C and saved to buffer
    return config->i2c_read_fun(config->i2c_addr, &(config->rd_buffer), 1);
}

bool pcf8574_lcd_if_write(void *interface_config, lcd_cmd_s lcd_cmd){
    // Cast generic interface configuration to PCF configuration
    pcf8574_config_s *config = (pcf8574_config_s *) interface_config;
    
    // Map LCD command lines to PC8574 GPIOs
    uint8_t output =    (lcd_cmd.rs << RS_PIN) | 
                        (lcd_cmd.rw << RW_PIN) |
                        (lcd_cmd.e << E_PIN) |
                        (lcd_cmd.ledk << LEDK_PIN) |
                        // Bitmask the i-th bit and shift it right by i, then map
                        ((lcd_cmd.data & (1 << 4)) >> 4 << DB4_PIN) |
                        ((lcd_cmd.data & (1 << 5)) >> 5 << DB5_PIN) |
                        ((lcd_cmd.data & (1 << 6)) >> 6 << DB6_PIN) |
                        ((lcd_cmd.data & (1 << 7)) >> 7 << DB7_PIN);
   
    // Write the data from the buffer to the device
    return pcf8574_write(config, output);
}

uint8_t pcf8574_lcd_if_read(void *interface_config){
    // Cast generic interface configuration to PCF configuration
    pcf8574_config_s *config = (pcf8574_config_s *) interface_config;
    
    // Create mask for data pins
    uint8_t mask =  (1 << DB4_PIN) | (1 << DB5_PIN) | 
                    (1 << DB6_PIN) | (1 << DB7_PIN);
    
    // Read data pins to buffer inside configuration
    pcf8574_read(config, mask);
            
    // Translate value from buffer to data value
    // Bitmask the data pins and shift them to their bit position
    uint8_t data =  ((config->rd_buffer) & (1 << DB4_PIN)) >> DB4_PIN << 4 |
                    ((config->rd_buffer) & (1 << DB5_PIN)) >> DB5_PIN << 5 |
                    ((config->rd_buffer) & (1 << DB6_PIN)) >> DB6_PIN << 6 |
                    ((config->rd_buffer) & (1 << DB7_PIN)) >> DB7_PIN << 7;
    
    // Reset buffer
    config->rd_buffer = 0x00;
    return data;
}