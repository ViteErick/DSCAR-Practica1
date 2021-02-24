ssize_t server_create(const char *server_ip_address)
{
    ssize_t sock, new_sock;
    struct sockaddr_in Serveraddr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    
    Serveraddr.sin_family = AF_INET;
    Serveraddr.sin_port = htons(SERVER_PORT);
    inet_aton(server_ip_address, &Serveraddr.sin_addr.s_addr);
    bind(sock, (struct sockaddr*) &Serveraddr, sizeof(Serveraddr));
    PRINTF("myssn INFO: Create server on %s port %i\r\n",server_ip_address,SERVER_PORT);
    return sock;
}