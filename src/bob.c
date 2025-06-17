#include "shared.h"

int main() {
    int bob_sock, alice_sock;
    struct sockaddr_in bob_addr, alice_addr;
    socklen_t alice_len = sizeof(alice_addr);

    bob_sock = socket(AF_INET, SOCK_STREAM, 0);
    bob_addr.sin_family = AF_INET;
    bob_addr.sin_addr.s_addr = INADDR_ANY;
    bob_addr.sin_port = htons(BOB_PORT);

    if (bind(bob_sock, (struct sockaddr *)&bob_addr, sizeof(bob_addr)) < 0) {
        perror("Bob: Bind failed");
        exit(EXIT_FAILURE);
    }
    listen(bob_sock, 3);
    printf("Bob is running and waiting for Alice on port %d...\n", BOB_PORT);

    alice_sock = accept(bob_sock, (struct sockaddr *)&alice_addr, &alice_len);
    printf("Bob: Accepted connection from Alice.\n");

    Msg_Alice_Bob msg_from_alice;
    recv(alice_sock, &msg_from_alice, sizeof(msg_from_alice), 0);
    printf("Bob: Received message from Alice.\n");

    Ticket_To_Bob ticket;
    memcpy(&ticket, msg_from_alice.ticket_to_bob, sizeof(ticket));
    xor_cipher((char*)&ticket, sizeof(ticket), KEY_BOB_KDC);
    
    uint32_t session_key_AB = ticket.session_key_AB;
    printf("Bob: Decrypted ticket. Session Key K_AB: %x. Alice ID: %s\n", session_key_AB, ticket.alice_id);

    uint32_t nonce_A_prime;
    memcpy(&nonce_A_prime, msg_from_alice.encrypted_nonce_A_prime, sizeof(nonce_A_prime));
    xor_cipher((char*)&nonce_A_prime, sizeof(nonce_A_prime), session_key_AB);
    printf("Bob: Decrypted Alice's nonce: %u\n", nonce_A_prime);

    uint32_t modified_nonce = nonce_A_prime - 1;
    printf("Bob: Modifying nonce to %u for challenge-response.\n", modified_nonce);

    xor_cipher((char*)&modified_nonce, sizeof(modified_nonce), session_key_AB);
    printf("Bob: Encrypting and sending modified nonce back to Alice.\n");

    send(alice_sock, &modified_nonce, sizeof(modified_nonce), 0);

    printf("Bob: Authentication with Alice is complete. Session is secure.\n");
    printf("Bob: We can now communicate securely using K_AB = %x\n", session_key_AB);

    close(alice_sock);
    close(bob_sock);
    return 0;
}
