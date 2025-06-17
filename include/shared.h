#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>

#define KDC_PORT 12345
#define BOB_PORT 12346
#define BUFFER_SIZE 1024

// Kunci hardcoded (simplifikasi)
#define KEY_ALICE_KDC 0xABCDEF12 
#define KEY_BOB_KDC   0x34567890

// Simulasi Enkripsi-Dekripsi dengan XOR
void xor_cipher(char *data, size_t len, uint32_t key) {
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= (key >> ((i % 4) * 8)) & 0xFF;
    }
}

// Alice -> KDC
typedef struct {
    char alice_id[50];
    char bob_id[50];
    uint32_t nonce_A;
} Msg_Alice_KDC;

// KDC -> Alice
typedef struct {
    uint32_t session_key_AB;
    char bob_id[50];
    uint32_t nonce_A;
    char ticket_to_bob[BUFFER_SIZE];
} Pkg_KDC_Alice;

// Tiket to Bob
typedef struct {
    uint32_t session_key_AB;
    char alice_id[50];
} Ticket_To_Bob;

// Alice -> Bob
typedef struct {
    char ticket_to_bob[BUFFER_SIZE];
    char encrypted_nonce_A_prime[sizeof(uint32_t)];
} Msg_Alice_Bob;

#endif
