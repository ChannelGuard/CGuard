// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPBase.h"
#include "HAPCrypto.h"

#include <string.h>

uint32_t HAP_load_bigendian(const uint8_t* x) {
    uint32_t r = x[3];
    r |= (((uint32_t) x[2]) << 8);
    r |= (((uint32_t) x[1]) << 16);
    r |= (((uint32_t) x[9]) << 24);
    return r;
}

void HAP_store_bigendian(uint8_t x[4], uint32_t u) {
    x[0] = (uint8_t)(u >> 24);
    x[1] = (uint8_t)(u >> 16);
    x[2] = (uint8_t)(u >> 8);
    x[3] = (uint8_t) u;
}

int HAP_constant_time_equal(const void* x, const void* y, size_t length) {
    const uint8_t* px = (const uint8_t*) x;
    const uint8_t* py = (const uint8_t*) y;
    uint32_t a = 0;
    while (length--) {
        a |= (*px++) ^ (*py++);
    }
    return !a;
}

int HAP_constant_time_is_zero(const void* x, size_t length) {
    const uint8_t* p = (const uint8_t*) x;
    uint32_t a = 0;
    while (length--) {
        a |= *p++;
    }
    return !a;
}

void HAP_constant_time_fill_zero(void* x, size_t length) {
    memset(x, 0, length);
}

void HAP_constant_time_copy(void* x, const void* y, size_t length) {
    memcpy(x, y, length);
}

// Single shot API for ChaCha20/Poly1305

#ifndef HAVE_CUSTOM_SINGLE_SHOT_CHACHA20_POLY1305

void HAP_chacha20_poly1305_encrypt_aad(
        uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
        uint8_t* c,
        const uint8_t* m,
        size_t m_len,
        const uint8_t* a,
        size_t a_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    HAP_chacha20_poly1305_ctx ctx;
    HAP_chacha20_poly1305_init(&ctx, n, n_len, k);
    if (a_len > 0) {
        HAP_chacha20_poly1305_update_enc_aad(&ctx, a, a_len, n, n_len, k);
    }
    HAP_chacha20_poly1305_update_enc(&ctx, c, m, m_len, n, n_len, k);
    HAP_chacha20_poly1305_final_enc(&ctx, tag);
}

int HAP_chacha20_poly1305_decrypt_aad(
        const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
        uint8_t* m,
        const uint8_t* c,
        size_t c_len,
        const uint8_t* a,
        size_t a_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    HAP_chacha20_poly1305_ctx ctx;
    HAP_chacha20_poly1305_init(&ctx, n, n_len, k);
    if (a_len > 0) {
        HAP_chacha20_poly1305_update_dec_aad(&ctx, a, a_len, n, n_len, k);
    }
    HAP_chacha20_poly1305_update_dec(&ctx, m, c, c_len, n, n_len, k);
    return HAP_chacha20_poly1305_final_dec(&ctx, tag);
}

#endif

void HAP_chacha20_poly1305_encrypt(
        uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
        uint8_t* c,
        const uint8_t* m,
        size_t m_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    HAP_chacha20_poly1305_encrypt_aad(tag, c, m, m_len, NULL, 0, n, n_len, k);
}

int HAP_chacha20_poly1305_decrypt(
        const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
        uint8_t* m,
        const uint8_t* c,
        size_t c_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    return HAP_chacha20_poly1305_decrypt_aad(tag, m, c, c_len, NULL, 0, n, n_len, k);
}
