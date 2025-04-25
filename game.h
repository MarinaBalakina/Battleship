#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <utility>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <algorithm> 

// Структура для хеширования пары
struct pair_hash {
    std::size_t operator()(const std::pair<uint64_t, uint64_t>& p) const {
        return std::hash<uint64_t>()(p.first) ^ (std::hash<uint64_t>()(p.second) << 1);
    }
};

struct ship_info {
    uint64_t type; 
    uint64_t hits; 
    std::vector<std::pair<uint64_t, uint64_t>> cells; 
};

class Game {
public:
    Game();
    bool is_game_started();
    void create_player(std::string &role);
    void generate_master_params();
    bool can_place_all_ships();
    void arrange_ships_master();
    bool is_area_free(uint64_t x, uint64_t y, char orientation, uint64_t size);
    void mark_forbidden_cells(ship_info &ship);
    ship_info* find_ship_at(uint64_t x, uint64_t y);
    void arrange_ships_slave();
    void set_width(uint64_t w);
    void get_width();
    void set_height(uint64_t h);
    void get_height();
    void set_count(uint64_t type, uint64_t count);
    void get_count(uint64_t type);
    void set_strategy(std::string &strategy);
    void start();
    void stop();
    void shot_user(uint64_t x, uint64_t y);
    void shot_bot();
    void set_result(std::string &res);
    void finished();
    void win();
    void lose();
    void dump(std::string &path);
    void load(std::string &path);

private:
    std::string mode;
    bool game_started;
    bool game_finished;
    uint64_t field_width;
    uint64_t field_height;
    uint64_t ships_count[4];
    std::vector<std::vector<uint64_t>> field;
    std::vector<ship_info> ships;
    std::string strategy;
    std::string finish_state;
    std::string win_state;
    std::string lose_state;
    uint64_t ordered_x;
    uint64_t ordered_y;
    uint64_t custom_x;
    uint64_t custom_y;
    bool params_set_by_user;
    std::unordered_set<std::pair<uint64_t, uint64_t>, pair_hash> bot_shots_fired;
};
