#include "shared.h"

int main() {
    int kdc_sock, client_sock;
    struct sockaddr_in kdc_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    srand(time(NULL));

    kdc_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (kdc_sock == -1) {
        perror("KDC: Socket creation failed");
        exit(EXIT_FAILURE);
    }

    kdc_addr.sin_family = AF_INET;
    kdc_addr.sin_addr.s_addr = INADDR_ANY;
    kdc_addr.sin_port = htons(KDC_PORT);

    if (bind(kdc_sock, (struct sockaddr *)&kdc_addr, sizeof(kdc_addr)) < 0) {
        perror("KDC: Bind failed");
        close(kdc_sock);
        exit(EXIT_FAILURE);
    }

    listen(kdc_sock, 3);
    printf("KDC Server is running and listening on port %d...\n", KDC_PORT);

    while (1) {
        client_sock = accept(kdc_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("KDC: Accept failed");
            continue;
        }
        printf("KDC: Accepted connection from an entity.\n");

        Msg_Alice_KDC msg_from_alice;
        if (recv(client_sock, &msg_from_alice, sizeof(msg_from_alice), 0) <= 0) {
            perror("KDC: Receive failed");
            close(client_sock);
            continue;
        }
        printf("KDC: Received request from '%s' to talk to '%s'. Nonce: %u\n", 
               msg_from_alice.alice_id, msg_from_alice.bob_id, msg_from_alice.nonce_A);

        uint32_t session_key_AB = rand();
        printf("KDC: Generated Session Key K_AB: %x\n", session_key_AB);

        Ticket_To_Bob ticket_for_bob;
        ticket_for_bob.session_key_AB = session_key_AB;
        strcpy(ticket_for_bob.alice_id, msg_from_alice.alice_id);
        
        xor_cipher((char*)&ticket_for_bob, sizeof(ticket_for_bob), KEY_BOB_KDC);
        printf("KDC: Created and encrypted ticket for Bob.\n");

        Pkg_KDC_Alice package_for_alice;
        package_for_alice.session_key_AB = session_key_AB;
        strcpy(package_for_alice.bob_id, msg_from_alice.bob_id);
        package_for_alice.nonce_A = msg_from_alice.nonce_A;
        memcpy(package_for_alice.ticket_to_bob, &ticket_for_bob, sizeof(ticket_for_bob));

        xor_cipher((char*)&package_for_alice, sizeof(package_for_alice), KEY_ALICE_KDC);
        printf("KDC: Created and encrypted package for Alice.\n");
        
        send(client_sock, &package_for_alice, sizeof(package_for_alice), 0);
        printf("KDC: Sent encrypted package to Alice. Closing connection.\n\n");
        
        close(client_sock);
    }

    close(kdc_sock);
    return 0;
}
