/* 
 * File:   PCF8574.h
 * Author: Patrick
 *
 * Created on 23. Januar 2021, 12:39
 */

#ifndef PCF8574_H
#define	PCF8574_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
    
// Mapping of LCD pins to I/O Expander pins, 4-bit parallel interface is used
#define RS_PIN      0       
#define RW_PIN      1
#define E_PIN       2
#define LEDK_PIN    3
#define DB4_PIN     4
#define DB5_PIN     5
#define DB6_PIN     6
#define DB7_PIN     7   
    
// I2C Bus Function Signature
typedef bool (*I2C_Fcn)(uint16_t, uint8_t*, uint32_t);
    
// Configuration structure for the PCF8574 GPIO Expander
typedef struct{
    uint16_t i2c_addr;         // I2C Address of the device
    I2C_Fcn i2c_write_fun;     // I2C Bus Write Function to be used
    I2C_Fcn i2c_read_fun;      // I2C Bus Read Function to be used
    uint8_t rd_buffer;         // Buffer to store received data
    uint8_t wr_buffer;         // Buffer to store data to be sent

}pcf8574_config_s;

// Edit the configuration data
void pcf8574_configure(pcf8574_config_s *config, uint16_t i2c_addr, I2C_Fcn write_fun, I2C_Fcn read_fun);

/* Standalone functions */
// Write a byte to the device output
bool pcf8574_write(pcf8574_config_s *config, uint8_t data);
// Read a byte from the device input
bool pcf8574_read(pcf8574_config_s *config, uint8_t mask);

/* Interface functions for usage as LCD io */
// Convert a 12-bit parallel interface command for an LCD for a hooked up expander
bool pcf8574_lcd_if_write(void *interface_config, lcd_cmd_s lcd_cmd);
// Read the 8-bit data lines
uint8_t pcf8574_lcd_if_read(void *interface_config);

#ifdef	__cplusplus
}
#endif

#endif	/* PCF8574_H */

