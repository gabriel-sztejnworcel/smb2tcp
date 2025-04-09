#pragma once

#include <Windows.h>
#include <bcrypt.h>

void bcrypt_create_aes_key(const BYTE* key, ULONG key_length, BCRYPT_KEY_HANDLE* key_handle, BCRYPT_ALG_HANDLE* alg_handle);
ULONG bcrypt_aes_encrypt_data(const BYTE* plaintext, ULONG plaintext_len, BCRYPT_KEY_HANDLE key_handle, BYTE* ciphertext, ULONG ciphertext_buffer_size);
ULONG bcrypt_aes_decrypt_data(const BYTE* ciphertext, ULONG ciphertext_len, BCRYPT_KEY_HANDLE key_handle, BYTE* plaintext, ULONG plaintext_buffer_size);
