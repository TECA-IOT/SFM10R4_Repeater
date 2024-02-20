/**
 * @file decode_hex.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __DECODE_H
#define __DECODE_H

/**
 * @brief comentar esta definicion si no se va usar el SERIAL
 */
#define __DEBUG_SESRIAL

#ifdef __cplusplus
 extern "C" {
#endif
/*Includes ----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*defines -----------------------------------------------*/
#define DEF(type, name, val)        type name = val
#define GET_UINT8( buffer, pos)     (uint8_t)buffer[pos]
#define GET_UINT16(buffer, pos)     (uint16_t)(buffer[pos]<<8 | buffer[pos + 1])   
#define GET_UINT32(buffer, pos)     (uint32_t)((uint32_t)buffer[pos]<<24 | (uint32_t)buffer[pos + 1]<<16 | (uint32_t)buffer[pos+2]<<8 | (uint32_t)buffer[pos+3])
#define GET_FLOAT(buffer, pos)      get_float_value(buffer, pos)
/*Enums -------------------------------------------------*/
typedef enum{
    BYTE0 = 0,
    BYTE1,
    BYTE2,
    BYTE3,
    BYTE4,
    BYTE5,
    BYTE6,
    BYTE7,
    BYTE8,
    BYTE9,
    BYTE10,
    BYTE11
}Byte_pos_t;
/*Typedef -----------------------------------------------*/

//para poder obtener bits individuales
typedef struct 
{
    uint8_t b0 : 1;
    uint8_t b1 : 1;
    uint8_t b2 : 1;
    uint8_t b3 : 1;
    uint8_t b4 : 1;
    uint8_t b5 : 1;
    uint8_t b6 : 1;
    uint8_t b7 : 1;
}Bits_t;

/*Function prototype ------------------------------------*/
/**
 * @brief convierte una cadena de caracteres en 12bytes hex
 * 
 * @param ptrHEX : buffer donde se va almacer la conversion
 * @param str : buffer que almacena la cadena de caracteres
 * @param length_hex: cantidad de hexadecimales
 */
void convertStringToHEX(uint8_t *ptrHEX,uint8_t length_hex, char *str);
/**
 * @brief funcion que convierte un buffer de hex en string
 * 
 * @param ptrStr : buffer donde se almacena la conversion
 * @param ptrHEX : buffer que contiene los hexs
 * @param length_hex : cantidad de elementos del hexs
 */
void convertHEXToString(char *ptrStr, uint8_t *ptrHEX, uint8_t length_hex);

/**
 * @brief Get the float value object
 * 
 * @param hex 
 * @param byper_pos 
 * @return float 
 */
float get_float_value(uint8_t *hex, uint8_t byte_pos);
/**
 * @brief obtener una mapa de bits
 * 
 * @param xByte : mapa de bits que contendrá los bits individuales
 * @param byte : byte a pasar a mapa de bits
 */
void bits_value(Bits_t *xByte, uint8_t data);

/**
 * @brief limpia el string recibido por la wisol y la libreria UFOX
 * 
 * @param result : Donde se va almacenar 
 * @param buffer : buffer recibido desde la wisol
 */
void clean_str_ufox(char *result, const char * buffer);


#ifdef __cplusplus
}
#endif

#endif
