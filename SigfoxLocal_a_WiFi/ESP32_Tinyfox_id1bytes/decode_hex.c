/**
 * @file decode_hex.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "decode_hex.h"
/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
// uint32_t hex2int(char *hex) {
//     uint32_t val = 0;
//     while (*hex) {
//         // get current character then increment
//         uint8_t byte = *hex++; 
//         // transform hex character to the 4bit equivalent number, using the ascii table indexes
//         if (byte >= '0' && byte <= '9') byte = byte - '0';
//         else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
//         else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
//         // shift 4 to make space for new digit, and add the 4 bits of the new digit 
//         val = (val << 4) | (byte & 0xF);
//     }
//     return val;
// }
/**
 * @brief convierte una cadena de caracteres en 12bytes hex
 * 
 * @param ptrHEX : buffer donde se va almacer la conversion
 * @param str : buffer que almacena la cadena de caracteres
 * @param length_hex: cantidad de hexadecimales
 */
void convertStringToHEX(uint8_t *ptrHEX,uint8_t length_hex, char *str){
    char temp[5] ;
    strcpy(temp, "0x");
    uint8_t i, j,k;           //iteradores
    for(i = 0; i<length_hex; i++){
        k = 2;
        //se obtiene la cadena que representa el byte
        for(j = 2 * i; j<(2*i+2);j++){
             temp[k] = str[j];
             k++;
        }
        temp[k] = '\0';
        //se realiza la conversion
        sscanf(temp,"%x",&ptrHEX[i]);
    }
    return;
}

/**
 * @brief funcion que convierte un buffer de hex en string
 * 
 * @param ptrStr : buffer donde se almacena la conversion
 * @param ptrHEX : buffer que contiene los hexs
 * @param length_hex : cantidad de elementos del hexs
 */
void convertHEXToString(char *ptrStr, uint8_t *ptrHEX, uint8_t length_hex){
    uint8_t i;
    char temp[2];
    memset(ptrStr, '\0', 2 * length_hex);

    for(i = 0; i<length_hex; i++){
        sprintf(temp,"%02X",ptrHEX[i]);
        strcat(ptrStr, temp);
    }
    return;
}

/**
 * @brief Get the float value object
 * 
 * @param hex 
 * @param byper_pos 
 * @return float 
 */
float get_float_value(uint8_t *hex, uint8_t byte_pos){
    float value; 
    static uint32_t temp = 0x0000;
    temp = (uint32_t)(GET_UINT32(hex, byte_pos));
    //uint32_t temp = 0x4141eb85;
    value  = (float)(*((float*)&temp));

    return value;
}
/**
 * @brief obtener una mapa de bits
 * 
 * @param xByte : mapa de bits que contendrá los bits individuales
 * @param byte : byte a pasar a mapa de bits
 */
void bits_value(Bits_t *xByte, uint8_t data){
    Bits_t *temp;
    temp = (Bits_t *)&data;
    memcpy(xByte, temp, sizeof(Bits_t) );
    return;
}

void clean_str_ufox(char *result, const char * buffer){
  uint8_t i, j;
  j = 0;
  for(i =3 ; i<strlen(buffer); i++){
    
    if(buffer[i] != ' '){
      result[j] = buffer[i];
      j++;
    }
    
  }
  result[j] = '\0';
}
