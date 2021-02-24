

#ifndef AES_CRC_LAYER_H_
#define AES_CRC_LAYER_H_


//-----------------Includes-----------------
#include "fsl_debug_console.h"
#include "aes.h"
#include "fsl_crc.h"

#include <math.h>


typedef struct crypt_msg_t{
	uint8_t* encrypted_msg;
	size_t padded_len;
}crypt_msg_t;


crypt_msg_t aes_encrypt_message(uint8_t message[]);
crypt_msg_t aes_decrypt_message(uint8_t message[]);
void InitCrc32(CRC_Type *base, uint32_t seed);
uint32_t calculate_crc32(crypt_msg_t msg);
void recvTask(crypt_msg_t msg);

#endif /* AES_CRC_LAYER_H_ */
