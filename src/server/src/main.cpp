#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#include <snl.h>


#pragma pack(push, 1)
struct SpawnPacket {
    uint32_t packet_type; 
    uint32_t type_id;     
    uint32_t network_id;  
};
#pragma pack(pop)

static bool addr_exists(const std::vector<std::string>& clients, const std::string& addr) {
    for (const auto& c : clients) {
        if (c == addr) return true;
    }
    return false;
}

int main() {
    const char* bind_addr = "0.0.0.0:4242";

    std::cout << "[Server] SNL version: " << net_get_version() << "\n";

    GameSocket* sock = net_socket_create(bind_addr);
    if (!sock) {
        std::cerr << "[Server] ERROR: net_socket_create failed for " << bind_addr << "\n";
        return 1;
    }

    std::cout << "[Server] Listening on " << bind_addr << "\n";

    uint32_t next_network_id = 100;
    std::vector<std::string> clients;

   
    uint8_t in_data[2048];
    char sender[256];

    while (true) {
        std::memset(in_data, 0, sizeof(in_data));
        std::memset(sender, 0, sizeof(sender));

        
        int32_t received = net_socket_poll(
            sock,
            in_data,
            (uintptr_t)sizeof(in_data),
            sender,
            (uintptr_t)sizeof(sender)
        );

        if (received <= 0) {
           
            continue;
        }

        std::string sender_addr(sender);

        
        if (!addr_exists(clients, sender_addr)) {
            clients.push_back(sender_addr);
            std::cout << "[Server] New client: " << sender_addr << "\n";

           
            SpawnPacket spawn{};
            spawn.packet_type = 1;
            spawn.type_id = 1;
            spawn.network_id = next_network_id++;

            std::cout << "[Server] Spawn entity with NetworkID: " << spawn.network_id << "\n";

           
            for (const auto& c : clients) {
                int32_t sent = net_socket_send(
                    sock,
                    c.c_str(),
                    reinterpret_cast<const uint8_t*>(&spawn),
                    (uintptr_t)sizeof(spawn)
                );

                if (sent <= 0) {
                    std::cout << "[Server] WARN: send failed to " << c << "\n";
                }
            }
        } else {
            
        }
    }

<   
    net_socket_destroy(sock);
    return 0;
}