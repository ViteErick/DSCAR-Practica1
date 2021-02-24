//------------------Includes---------------------

#include "aes_crc_layer.h"

//------------------Variables--------------------

/* AES data */
uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
struct AES_ctx ctx;
size_t test_string_len,padded_len;
uint8_t padded_msg[512] = {0};


crypt_msg_t aes_encrypt_message(uint8_t message[])
{
    crypt_msg_t new_message;

    PRINTF("AES128\r\n");
	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

    /* To encrypt an array its length must be a multiple of 16 so we add zeros */
	test_string_len = strlen(message);
	padded_len = test_string_len + (16 - (test_string_len%16) );
	memcpy(padded_msg, message, test_string_len);

	/*Encrypt the buffer */
	AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

	new_message.encrypted_msg = padded_msg;
	new_message.padded_len =  padded_len;

	return new_message;
}

crypt_msg_t aes_decrypt_message(uint8_t message[])
{
	crypt_msg_t new_message;
	uint8_t message_len = 0;
	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	test_string_len = strlen(message);
	padded_len = test_string_len;
	memcpy(padded_msg, message, test_string_len);

	/*Decrypt the buffer */
	AES_CBC_decrypt_buffer(&ctx, padded_msg, padded_len);

	for(uint8_t i = 0; i < padded_len; i++ )
	{
		if(padded_msg[i] == 0)
		{
			message_len = i;
			break;
		}
	}

	new_message.encrypted_msg = padded_msg;
	new_message.padded_len = message_len;

	return new_message;
}

/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

uint32_t calculate_crc32(crypt_msg_t msg)
{
	/* CRC data */
	CRC_Type *base = CRC0;
	uint32_t checksum32;

	InitCrc32(base, 0xFFFFFFFFU);
	CRC_WriteData(base, msg.encrypted_msg, msg.padded_len);
	checksum32 = CRC_Get32bitResult(base);

	return checksum32;
}

void recvTask(crypt_msg_t msg)
{
	uint8_t msg_body_bytes[128];
	uint8_t msg_crc_bytes[4];
	uint32_t checksum_original =  0;
	uint32_t checksum_calc =  0;
	
	uint8_t counter = 0;

	crypt_msg_t split_msg_crc;
	crypt_msg_t split_msg_aes;

	//Get the message body bytes
	for(uint8_t i = 0; i < msg.padded_len - 4; i++)
	{
		msg_body_bytes[i] = msg.encrypted_msg[i];
	}

	//Get the CRC bytes
	for(uint8_t i = msg.padded_len - 4; i < msg.padded_len; i++)
	{
		msg_crc_bytes[counter] = msg.encrypted_msg[i];
		counter++;
	}

	//Convert the CRC bytes from the message to int
	for (uint8_t i = 0; i < 4; i++)
	{
		checksum_original = checksum_original + ( msg_crc_bytes[i] *  pow(256,i) );
	}


	split_msg_crc.encrypted_msg = msg_body_bytes;
	split_msg_crc.padded_len =  msg.padded_len - 4;;

	checksum_calc = calculate_crc32(split_msg_crc);

	if(checksum_original == checksum_calc)
	{
		PRINTF("Calculated Checksum: 	%u \r\n",	checksum_calc);
		PRINTF("Original checksum: 	%u \r\n",	checksum_original);
		PRINTF("Checksum match! Decrypting message..\r\n\n");

		split_msg_aes = aes_decrypt_message(msg_body_bytes);

		PRINTF("Decrypted Message: ");
		for(int i=0; i<split_msg_aes.padded_len; i++)
		{
			PRINTF("%c", split_msg_aes.encrypted_msg[i]);
		}
		PRINTF("\r\n\r\n");

	}
	else
	{
		PRINTF("...Check fail. Expected: 0x%x\r\n", checksum_original);
	}
}
