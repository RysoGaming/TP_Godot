#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <chrono>
#include <cmath>

#include <snl.h>

#pragma pack(push, 1)
struct SpawnPacket {
    uint32_t packet_type;
    uint32_t type_id;
    uint32_t network_id;
};
#pragma pack(pop)

static uint16_t read_u16_le(const uint8_t* data) {
    return (uint16_t)data[0] |
        ((uint16_t)data[1] << 8);
}

static uint32_t read_u32_le(const uint8_t* data) {
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);
}

struct PlayerState {
    uint32_t network_id = 0;
    float x = 500.f;
    float y = 500.f;
    float vx = 0.f;
    float vy = 0.f;
    uint16_t input_mask = 0;
    int8_t move_x = 0;
    int8_t move_y = 0;
};

int main() {
    const char* bind_addr = "0.0.0.0:4242";

    std::cout << "[Server] SNL version: " << net_get_version() << "\n";

    GameSocket* sock = net_socket_create(bind_addr);
    if (!sock) {
        std::cerr << "[Server] ERROR: net_socket_create failed\n";
        return 1;
    }

    std::cout << "[Server] Listening on " << bind_addr << "\n";

    uint32_t next_network_id = 100;
    std::unordered_map<std::string, PlayerState> players;
    std::vector<SpawnPacket> all_spawns;

    uint8_t in_data[2048];
    char sender[256];

    auto last_tick = std::chrono::steady_clock::now();

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

        if (received > 0) {
            std::string sender_addr(sender);

            if (received == 5 && std::memcmp(in_data, "hello", 5) == 0) {
                if (players.find(sender_addr) == players.end()) {
                    std::cout << "[Server] New client: " << sender_addr << "\n";

                    PlayerState p;
                    p.network_id = next_network_id++;
                    players[sender_addr] = p;

                    SpawnPacket spawn{};
                    spawn.packet_type = 1;
                    spawn.type_id = 1;
                    spawn.network_id = p.network_id;

                    all_spawns.push_back(spawn);

                    std::cout << "[Server] Spawn entity ID: " << spawn.network_id << "\n";

                    for (const auto& [addr, _] : players) {
                        net_socket_send(
                            sock,
                            addr.c_str(),
                            reinterpret_cast<const uint8_t*>(&spawn),
                            (uintptr_t)sizeof(spawn)
                        );
                    }
                }
            }
            else if (received >= 15 && in_data[0] == 1) {
                if (players.find(sender_addr) == players.end()) {
                    continue;
                }

                uint32_t sequence = read_u32_le(&in_data[1]);
                uint16_t input_mask = read_u16_le(&in_data[9]);
                int8_t move_x = (int8_t)in_data[13];
                int8_t move_y = (int8_t)in_data[14];

                auto& p = players[sender_addr];
                p.input_mask = input_mask;
                p.move_x = move_x;
                p.move_y = move_y;

                std::cout << "[Input] seq=" << sequence
                          << " mask=0x" << std::hex << input_mask << std::dec
                          << " move=(" << (int)move_x << "," << (int)move_y << ")\n";
            }
        }

        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - last_tick).count();

        if (elapsed >= 0.05f) {
            last_tick = now;

            for (auto& [addr, p] : players) {
                float speed = 200.0f;

                if (p.input_mask & 0x80) {
                    speed *= 1.5f;
                }

                float dir_x = p.move_x / 127.0f;
                float dir_y = p.move_y / 127.0f;

                if (dir_x == 0.0f && dir_y == 0.0f) {
                    if (p.input_mask & 0x08) dir_x = 1.0f;
                    if (p.input_mask & 0x04) dir_x = -1.0f;
                    if (p.input_mask & 0x01) dir_y = -1.0f;
                    if (p.input_mask & 0x02) dir_y = 1.0f;
                }

                float mag = std::sqrt(dir_x * dir_x + dir_y * dir_y);
                if (mag > 1.0f) {
                    dir_x /= mag;
                    dir_y /= mag;
                }

                p.vx = dir_x * speed;
                p.vy = dir_y * speed;

                p.x += p.vx * 0.05f;
                p.y += p.vy * 0.05f;

                if (p.x < 0.f) p.x = 0.f;
                if (p.x > 1000.f) p.x = 1000.f;
                if (p.y < 0.f) p.y = 0.f;
                if (p.y > 1000.f) p.y = 1000.f;

                std::cout << "[Tick] " << addr
                          << " pos=" << p.x << ", " << p.y << "\n";
            }
        }
    }

    net_socket_destroy(sock);
    return 0;
}