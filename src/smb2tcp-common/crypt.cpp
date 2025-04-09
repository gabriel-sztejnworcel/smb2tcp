#include "crypt.h"
#include <wil/resource.h>
#include <stdexcept>
#include <string>

void bcrypt_create_aes_key(const BYTE* key, ULONG key_length, BCRYPT_KEY_HANDLE* key_handle, BCRYPT_ALG_HANDLE* alg_handle)
{
    NTSTATUS status = BCryptOpenAlgorithmProvider(alg_handle, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (status < 0)
    {
        std::string msg = "BCryptOpenAlgorithmProvider failed: " + std::to_string(status);
        throw std::runtime_error(msg);
    }

    status = BCryptGenerateSymmetricKey(*alg_handle, key_handle, NULL, 0, (BYTE*)key, key_length, 0);
    if (status < 0)
    {
        std::string msg = "BCryptGenerateSymmetricKey failed: " + std::to_string(status);
        throw std::runtime_error(msg);
    }
}

ULONG bcrypt_aes_encrypt_data(const BYTE* plaintext, ULONG plaintext_len, BCRYPT_KEY_HANDLE key_handle, BYTE* ciphertext, ULONG ciphertext_buffer_size)
{
    ULONG ciphertext_size = 0;
    NTSTATUS status = BCryptEncrypt(
        key_handle,
        (BYTE*)plaintext, plaintext_len,
        NULL,
        NULL,
        0,
        ciphertext, ciphertext_buffer_size, &ciphertext_size,
        BCRYPT_BLOCK_PADDING
    );

    if (status < 0)
    {
        std::string msg = "BCryptEncrypt failed: " + std::to_string(status);
        throw std::runtime_error(msg);
    }

    return ciphertext_size;
}

ULONG bcrypt_aes_decrypt_data(const BYTE* ciphertext, ULONG ciphertext_len, BCRYPT_KEY_HANDLE key_handle, BYTE* plaintext, ULONG plaintext_buffer_size)
{
    ULONG plaintext_size = 0;
    NTSTATUS status = BCryptDecrypt(
        key_handle,
        (BYTE*)ciphertext, ciphertext_len,
        NULL,
        NULL,
        0,
        plaintext, plaintext_buffer_size, &plaintext_size,
        BCRYPT_BLOCK_PADDING
    );

    if (status < 0)
    {
        std::string msg = "BCryptDecrypt failed: " + std::to_string(status);
        throw std::runtime_error(msg);
    }

    return plaintext_size;
}
