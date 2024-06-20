/*
 * embedded IPsec
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

 /** @file des_test.c
  *  @brief Test functions for DES/3DES CBC code
  *
  *  @author Christian Scheurer <http://www.christianscheurer.ch> <BR>
  *
  *  <B>OUTLINE:</B>
  *
  *  This file contains test functions used to verify the DES/3DES-CBC code.
  *
  *  <B>IMPLEMENTATION:</B>
  *
  *  There are no implementation hints to be mentioned.
  *
  *  <B>NOTES:</B>
  *
  * This document is part of <EM>embedded IPsec<BR>
  * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne<BR>
  * All rights reserved.</EM><HR>
  */

#include <string.h>

#include "ipsec/util.h"
#include "ipsec/des.h"
#include "ipsec/debug.h"
#include "structural_test.h"


  /**
   *  test DES key setter functions
	* @return int number of tests failed in this function
   */
int des_test_DES_set_key_checked(void)
{
	const unsigned char correct_key[] = { 0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67 };
	const unsigned char parityerror_key[] = { 0x01,0x23,0x45,0x67,0x03,0x23,0x45,0x67 };
	const unsigned char weak_key[] = { 0x1F,0x1F,0x1F,0x1F,0x0E,0x0E,0x0E,0x0E };
	const unsigned char semiweak_key[] = { 0x1F,0xE0,0x1F,0xE0,0x0E,0xF1,0x0E,0xF1 };

	int local_error_count = 0;
	int ret_val;
	DES_key_schedule ks;

	ret_val = DES_set_key_checked(&correct_key, &ks);
	if (ret_val != 0) {
		local_error_count++;
		printf("des_test_DES_set_key_checked(): error - DES_set_key_checked(&correct_key,&ks) rejected correct key - ret_val = %d\n", ret_val);
	}

	ret_val = DES_set_key_checked(&parityerror_key, &ks);
	if (ret_val != -1) {
		local_error_count++;
		printf("des_test_DES_set_key_checked(): error - DES_set_key_checked(&parityerror_key,&ks) accepted key with parity error - ret_val = %d\n", ret_val);
	}

	ret_val = DES_set_key_checked(&weak_key, &ks);
	if (ret_val != -2) {
		local_error_count++;
		printf("des_test_DES_set_key_checked(): error - DES_set_key_checked(&weak_key,&ks) did not reject weak key - ret_val = %d\n", ret_val);
	}

	ret_val = DES_set_key_checked(&semiweak_key, &ks);
	if (ret_val != -2) {
		local_error_count++;
		printf("des_test_DES_set_key_checked(): error - DES_set_key_checked(&semiweak_key,&ks) did not reject semi-weak key - ret_val = %d\n", ret_val);
	}

	return local_error_count;
}


/**
 * Tests the DES encryption
 * @return int number of tests failed in this function
 */
int des_test_des_ede3_cbc_encrypt(void)
{
	const unsigned char esp_encrypted_payload[] = {
		/* Encrypted Data */
		0x53, 0x83, 0xFB, 0x32, 0x7F, 0x7E, 0x9A, 0xE9, 0x01, 0xB3, 0x7F, 0x80, 0x06, 0xCF, 0x39, 0x26,
		0x6F, 0x5D, 0x25, 0x5B, 0x1E, 0x26, 0x28, 0xC4, 0xB3, 0x78, 0x04, 0xAC, 0x9F, 0x0B, 0xAE, 0x9D,
		0x57, 0xAC, 0x4E, 0x82, 0x0E, 0xB4, 0xBA, 0x6C, 0x27, 0x38, 0x0A, 0x71, 0x9B, 0xE1, 0xF7, 0xFD,
		0x14, 0xCC, 0x14, 0x0D, 0x71, 0xE5, 0xB8, 0xF7, 0x7C, 0xCB, 0xE0, 0x0B, 0x04, 0x78, 0xB1, 0x7F,
		0x2B, 0xEA, 0x85, 0x41, 0x95, 0x22, 0x18, 0xEE, 0x06, 0xFF, 0x79, 0x13, 0x26, 0xD3, 0x08, 0x10,
		0x75, 0x04, 0x53, 0x6C, 0x04, 0xFD, 0x91, 0xF3, 0x31, 0x93, 0xB6, 0x14, 0xE3, 0x53, 0xF6, 0xCE,
		0x6E, 0xC3, 0x74, 0xFE, 0xCE, 0x62, 0x01, 0xBD, 0x8D, 0x11, 0x26, 0x39, 0x0A, 0x74, 0x51, 0xCA,
		0x56, 0x42, 0x51, 0xD5, 0x7B, 0x8E, 0xA6, 0x47, 0xDC, 0x7F, 0x74, 0x09, 0x90, 0x8F, 0x9A, 0xF2,
		0x0C, 0x74, 0x81, 0x64, 0x10, 0x0D, 0x64, 0xCE, 0x54, 0xB5, 0x78, 0x81, 0xA5, 0x90, 0x36, 0xEB,
		0x82, 0xC9, 0xE7, 0x5C, 0xBD, 0x3F, 0x3B, 0x29, 0xD3, 0x2F, 0x22, 0x6E, 0x6F, 0x24, 0xDD, 0xFB,
		0xDD, 0x98, 0x00, 0x1D, 0x4B, 0x82, 0xCE, 0x12, 0xD1, 0x33, 0x29, 0x22, 0x4F, 0x92, 0xF9, 0x94,
		0xEB, 0x12, 0x57, 0x2F, 0xFA, 0x30, 0xC7, 0xF1, 0x9D, 0x72, 0xCA, 0x8D, 0x25, 0xEB, 0x2C, 0x3D,
		0xE3, 0xFB, 0xCC, 0xBC, 0x54, 0x11, 0x53, 0x25, 0xE2, 0xCE, 0x78, 0xF9, 0xDC, 0x6F, 0xE8, 0xE2,
		0xFB, 0x41, 0x3C, 0xED, 0x6F, 0x9A, 0xB1, 0xF3, 0x72, 0x78, 0x02, 0xCC, 0x91, 0x3D, 0x20, 0x7F,
		0xBE, 0xE9, 0x53, 0xD4, 0xFB, 0xFF, 0x6E, 0x42, 0xB1, 0xF6, 0x5E, 0x4D, 0x14, 0x03, 0xB4, 0xFB,
		0x2F, 0x53, 0x7A, 0xBC, 0x9E, 0xA7, 0xCE, 0x8E, 0xCA, 0x08, 0xC4, 0x7B, 0x19, 0xA1, 0x2B, 0x7F,
		0x6A, 0x79, 0x77, 0xCD, 0x77, 0xA4, 0xBA, 0x8C, 0xB8, 0xAC, 0x9C, 0x35, 0x9C, 0x30, 0xB1, 0x29,
		0x78, 0xAA, 0x4A, 0xFF, 0xB5, 0xAB, 0x5B, 0x03, 0x09, 0x43, 0xC9, 0x0F, 0xFC, 0x2D, 0x50, 0x65,
		0x62, 0xC2, 0xA4, 0xB2, 0xC5, 0x07, 0x4D, 0x41, 0x43, 0xEE, 0x9A, 0x23, 0xA3, 0x02, 0xA5, 0x9A,
		0x6E, 0xBD, 0x9E, 0x8C, 0x9F, 0xA2, 0x70, 0x52, 0x61, 0x33, 0x9A, 0x21, 0x26, 0xA6, 0xBE, 0xF6,
		0x82, 0x55, 0x0D, 0xB7, 0x03, 0x40, 0x56, 0x18, 0xAA, 0xAB, 0x09, 0xC1, 0x3B, 0x1F, 0xC0, 0x6D,
		0x62, 0x0C, 0x7D, 0xA3, 0x59, 0x96, 0x99, 0x94, 0xC3, 0x06, 0x98, 0x6B, 0x64, 0x82, 0xEB, 0xCC,
		0x9B, 0x3E, 0x56, 0xC3, 0xA7, 0xEB, 0x9C, 0xF7, 0x07, 0x6C, 0x07, 0x45, 0x8E, 0xBC, 0x85, 0x41,
		0x48, 0x69, 0x78, 0x23, 0xAB, 0xBC, 0xE7, 0x1F, 0x7B, 0xD2, 0x07, 0x81, 0xDA, 0x9D, 0x1B, 0x5F,
		0x0A, 0x4A, 0x37, 0x88, 0xF8, 0x1C, 0x4F, 0xB4, 0x84, 0xDC, 0x14, 0xD4, 0xD3, 0x3B, 0x29, 0x2B,
		0xA9, 0x42, 0x77, 0x2C, 0x91, 0x48, 0x05, 0xF5, 0x21, 0x54, 0xDD, 0xD3, 0xC8, 0xB5, 0xCE, 0xB7,
		0x10, 0x7E, 0x63, 0x5A, 0xF3, 0x9E, 0x2C, 0xC7, 0xBC, 0xC8, 0xC9, 0xC2, 0x2D, 0xF0, 0x62, 0x5E,
		0xB8, 0xA3, 0x84, 0x8A, 0x3D, 0x5D, 0xB5, 0xF0, 0x14, 0xB4, 0x77, 0xC3, 0x67, 0xD0, 0x5E, 0x77,
	};

	const unsigned char esp_orig_payload[] = {
		/* IP Header */
		0x45, 0x00, 0x01, 0xB9,
		0xDF, 0x12, 0x40, 0x00,
		0x40, 0x06, 0xD6, 0xB0,
		0xC0, 0xA8, 0x01, 0x03,
		0xC0, 0xA8, 0x01, 0x28,
		/* TCP Header */
		0x00, 0x50, 0x80, 0x1A,
		0x4B, 0x42, 0xCB, 0x6B,
		0x84, 0xB9, 0xC5, 0x67,
		0x80, 0x18, 0x7E, 0xA0,
		0x39, 0xA7, 0x00, 0x00,
		0x01, 0x01, 0x08, 0x0A,
		0x00, 0x6B, 0x14, 0x79,
		0x00, 0x0F, 0x22, 0x1C,
		/* HTTP Packet */
		0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x30, 0x20, 0x32, 0x30, 0x30,
		0x20, 0x4F, 0x4B, 0x0D, 0x0A, 0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79, 0x70,
		0x65, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x68, 0x74, 0x6D, 0x6C, 0x0D, 0x0A, 0x0D, 0x0A,
		0x3C, 0x68, 0x74, 0x6D, 0x6C, 0x3E, 0x0D, 0x0A, 0x3C, 0x68, 0x65, 0x61, 0x64, 0x3E, 0x0D, 0x0A,
		0x3C, 0x2F, 0x68, 0x65, 0x61, 0x64, 0x3E, 0x0D, 0x0A, 0x3C, 0x62, 0x6F, 0x64, 0x79, 0x20, 0x74,
		0x65, 0x78, 0x74, 0x3D, 0x22, 0x23, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x22, 0x20, 0x62, 0x67,
		0x63, 0x6F, 0x6C, 0x6F, 0x72, 0x3D, 0x22, 0x23, 0x46, 0x46, 0x46, 0x46, 0x39, 0x39, 0x22, 0x20,
		0x6C, 0x69, 0x6E, 0x6B, 0x3D, 0x22, 0x23, 0x30, 0x30, 0x30, 0x30, 0x45, 0x45, 0x22, 0x20, 0x76,
		0x6C, 0x69, 0x6E, 0x6B, 0x3D, 0x22, 0x23, 0x35, 0x35, 0x31, 0x41, 0x38, 0x42, 0x22, 0x20, 0x61,
		0x6C, 0x69, 0x6E, 0x6B, 0x3D, 0x22, 0x23, 0x46, 0x46, 0x30, 0x30, 0x30, 0x30, 0x22, 0x3E, 0x0D,
		0x0A, 0x26, 0x6E, 0x62, 0x73, 0x70, 0x3B, 0x0D, 0x0A, 0x3C, 0x63, 0x65, 0x6E, 0x74, 0x65, 0x72,
		0x3E, 0x3C, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x42, 0x4F, 0x52, 0x44, 0x45, 0x52, 0x3D, 0x30,
		0x20, 0x43, 0x4F, 0x4C, 0x53, 0x3D, 0x31, 0x20, 0x57, 0x49, 0x44, 0x54, 0x48, 0x3D, 0x22, 0x39,
		0x30, 0x25, 0x22, 0x20, 0x48, 0x45, 0x49, 0x47, 0x48, 0x54, 0x3D, 0x22, 0x38, 0x35, 0x25, 0x22,
		0x20, 0x3E, 0x0D, 0x0A, 0x3C, 0x74, 0x72, 0x20, 0x41, 0x4C, 0x49, 0x47, 0x4E, 0x3D, 0x4C, 0x45,
		0x46, 0x54, 0x20, 0x56, 0x41, 0x4C, 0x49, 0x47, 0x4E, 0x3D, 0x54, 0x4F, 0x50, 0x3E, 0x0D, 0x0A,
		0x3C, 0x74, 0x64, 0x3E, 0x3C, 0x62, 0x3E, 0x3C, 0x66, 0x6F, 0x6E, 0x74, 0x20, 0x66, 0x61, 0x63,
		0x65, 0x3D, 0x22, 0x56, 0x65, 0x72, 0x64, 0x61, 0x6E, 0x61, 0x22, 0x3E, 0x3C, 0x66, 0x6F, 0x6E,
		0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x2B, 0x32, 0x3E, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64,
		0x65, 0x64, 0x20, 0x57, 0x65, 0x62, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x54, 0x65,
		0x73, 0x74, 0x3C, 0x2F, 0x66, 0x6F, 0x6E, 0x74, 0x3E, 0x3C, 0x2F, 0x66, 0x6F, 0x6E, 0x74, 0x3E,
		0x3C, 0x2F, 0x62, 0x3E, 0x3C, 0x2F, 0x74, 0x64, 0x3E, 0x0D, 0x0A, 0x3C, 0x2F, 0x74, 0x72, 0x3E,
		0x0D, 0x0A, 0x3C, 0x2F, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x3E, 0x0D, 0x0A, 0x3C, 0x2F, 0x63, 0x65,
		0x6E, 0x74, 0x65, 0x72, 0x3E, 0x0D, 0x0A, 0x3C, 0x2F, 0x62, 0x6F, 0x64, 0x79, 0x3E, 0x0D, 0x0A,
		0x3C, 0x2F, 0x68, 0x74, 0x6D, 0x6C, 0x3E, 0x0D, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x05, 0x04,
	};
	unsigned char esp_decrypted_payload[sizeof(esp_encrypted_payload)];
	const unsigned char _3des_key[8 * 3] = { 0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67 };
	unsigned char iv_esp_payload[8] = { 0xD4,0xDB,0xAB,0x9A,0x9A,0xDB,0xD1,0x94 };
	int local_error_count = 0;
	int ret_val;

	/* decrypt ESP packet */
	cipher_3des_cbc((unsigned char *)&esp_encrypted_payload,
		sizeof(esp_encrypted_payload),
		(unsigned char *)&_3des_key,
		(unsigned char *)&iv_esp_payload,
		DES_DECRYPT,
		esp_decrypted_payload);

	ret_val = memcmp((unsigned char *)esp_orig_payload[52], (unsigned char *)esp_decrypted_payload[52], sizeof(esp_encrypted_payload) - 52);
	if (ret_val != 0)
	{
		local_error_count++;
		printf("des_test_des_ede3_cbc_encrypt(): error - des_ede3_cbc_encrypt() could not properly decrypt 3DES encrypted ESP payload - ret_val = %d\n", ret_val);
		IPSEC_DUMP_BUFFER("   input   : ", esp_encrypted_payload, 0, sizeof(esp_encrypted_payload));
		printf("\n");
		IPSEC_DUMP_BUFFER("   output  : ", esp_decrypted_payload, 0, sizeof(esp_decrypted_payload));
		printf("\n");
		IPSEC_DUMP_BUFFER("   expected: ", esp_orig_payload, 0, sizeof(esp_orig_payload));
		printf("\n\n");
	}

	return local_error_count;
}



/**
 * Main test function for the DES/3DES CBC tests.
 * It does nothing but calling the subtests one after the other.
 */
void des_test(test_result *global_results)
{
	test_result 	sub_results = {
						  5,
						  2,
						  0,
						  0,
	};

	int retcode;

	retcode = des_test_DES_set_key_checked();
	IPSEC_TESTING_EVALUATE(retcode, sub_results, "des_test_DES_set_key_checked()", ("ported from openssl.org"));

	retcode = des_test_des_ede3_cbc_encrypt();
	IPSEC_TESTING_EVALUATE(retcode, sub_results, "des_test_des_ede3_cbc_encrypt()", ("ported from openssl.org"));

	global_results->tests += sub_results.tests;
	global_results->functions += sub_results.functions;
	global_results->errors += sub_results.errors;
	global_results->notimplemented += sub_results.notimplemented;
}
