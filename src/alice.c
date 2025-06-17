#include "shared.h"

int main() {
    int sock_to_kdc, sock_to_bob;
    struct sockaddr_in kdc_addr, bob_addr;
    
    char alice_id[] = "Alice";
    char bob_id[] = "Bob";
    
    srand(time(NULL));

    printf("--- ALICE <-> KDC ---\n");

    sock_to_kdc = socket(AF_INET, SOCK_STREAM, 0);
    kdc_addr.sin_family = AF_INET;
    kdc_addr.sin_port = htons(KDC_PORT);
    inet_pton(AF_INET, "127.0.0.1", &kdc_addr.sin_addr);

    if (connect(sock_to_kdc, (struct sockaddr *)&kdc_addr, sizeof(kdc_addr)) < 0) {
        perror("Alice: Connection to KDC failed");
        exit(EXIT_FAILURE);
    }
    printf("Alice: Connected to KDC.\n");

    Msg_Alice_KDC request_to_kdc;
    strcpy(request_to_kdc.alice_id, alice_id);
    strcpy(request_to_kdc.bob_id, bob_id);
    request_to_kdc.nonce_A = rand() % 1000;
    
    send(sock_to_kdc, &request_to_kdc, sizeof(request_to_kdc), 0);
    printf("Alice: Sent request to KDC (My ID: %s, Bob's ID: %s, Nonce: %u)\n", 
           alice_id, bob_id, request_to_kdc.nonce_A);

    Pkg_KDC_Alice encrypted_pkg_from_kdc;
    recv(sock_to_kdc, &encrypted_pkg_from_kdc, sizeof(encrypted_pkg_from_kdc), 0);
    printf("Alice: Received encrypted package from KDC.\n");
    
    xor_cipher((char*)&encrypted_pkg_from_kdc, sizeof(encrypted_pkg_from_kdc), KEY_ALICE_KDC);
    printf("Alice: Decrypted the package.\n");

    if (encrypted_pkg_from_kdc.nonce_A != request_to_kdc.nonce_A) {
        printf("Alice: Nonce mismatch! Aborting.\n");
        close(sock_to_kdc);
        exit(EXIT_FAILURE);
    }
    printf("Alice: Nonce verified!\n");
    
    uint32_t session_key_AB = encrypted_pkg_from_kdc.session_key_AB;
    printf("Alice: Extracted Session Key K_AB: %x\n", session_key_AB);
    
    close(sock_to_kdc);

    printf("\n--- ALICE <-> BOB ---\n");
    
    sock_to_bob = socket(AF_INET, SOCK_STREAM, 0);
    bob_addr.sin_family = AF_INET;
    bob_addr.sin_port = htons(BOB_PORT);
    inet_pton(AF_INET, "127.0.0.1", &bob_addr.sin_addr);
    
    sleep(1); 
    if (connect(sock_to_bob, (struct sockaddr *)&bob_addr, sizeof(bob_addr)) < 0) {
        perror("Alice: Connection to Bob failed");
        exit(EXIT_FAILURE);
    }
    printf("Alice: Connected to Bob.\n");

    Msg_Alice_Bob msg_to_bob;
    memcpy(msg_to_bob.ticket_to_bob, encrypted_pkg_from_kdc.ticket_to_bob, sizeof(msg_to_bob.ticket_to_bob));
    
    uint32_t nonce_A_prime = rand() % 2000;
    printf("Alice: Generated a new nonce for Bob: %u\n", nonce_A_prime);
    
    uint32_t encrypted_nonce_A_prime = nonce_A_prime;
    xor_cipher((char*)&encrypted_nonce_A_prime, sizeof(encrypted_nonce_A_prime), session_key_AB);
    memcpy(msg_to_bob.encrypted_nonce_A_prime, &encrypted_nonce_A_prime, sizeof(encrypted_nonce_A_prime));

    send(sock_to_bob, &msg_to_bob, sizeof(msg_to_bob), 0);
    printf("Alice: Sent ticket and encrypted nonce to Bob.\n");
    
    uint32_t response_from_bob;
    recv(sock_to_bob, &response_from_bob, sizeof(response_from_bob), 0);
    printf("Alice: Received challenge-response from Bob.\n");

    xor_cipher((char*)&response_from_bob, sizeof(response_from_bob), session_key_AB);
    printf("Alice: Decrypted Bob's response: %u\n", response_from_bob);
    
    if (response_from_bob == (nonce_A_prime - 1)) {
        printf("Alice: Bob's response is correct! Authentication successful.\n");
        printf("Alice: We can now communicate securely using K_AB = %x\n", session_key_AB);
    } else {
        printf("Alice: Bob's response is incorrect! Authentication failed.\n");
    }

    close(sock_to_bob);
    return 0;
}
