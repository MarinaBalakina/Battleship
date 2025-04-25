#include "game.h"


// Вспомогательная функция для обрезки пробелов
static void trim_string(std::string &str) {
    while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front()))) str.erase(str.begin());
    while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back()))) str.pop_back();
}

bool num_error(std::string &num) {
    size_t i = 0;
    while (i < num.size()) {
        if (!std::isdigit(static_cast<unsigned char>(num[i]))) {
            return false;
        }
        i++;
    }
    return true;
}

Game::Game() {
    mode = "";
    game_started = false;
    game_finished = false;
    field_width = 0;
    field_height = 0;
    for(int i = 0; i < 4; i++) {
        ships_count[i] = 0;
    }
    strategy = "custom";
    finish_state = "no";
    win_state = "no";
    lose_state = "no";
    ordered_x = 0;
    ordered_y = 0;
    custom_x = 0;
    custom_y = 0;
    params_set_by_user = false;
    srand(static_cast<unsigned int>(time(NULL)));
}

bool Game::is_game_started () {
    return game_started;
}

void Game::create_player(std::string &word) {
    std::string role = word;
    trim_string(role);
    if (role == "master") {
        mode = "master";
        generate_master_params();
        std::cout << "ok\n";
    }
    else if (role == "slave") {
        mode = "slave";
        std::cout << "ok\n";
    }
    else {
        std::cout << "failed\n";
    }
}

void Game::generate_master_params() {
    field_width = (rand() % 100000) + 10;
    field_height = (rand() % 100000) + 10;

    bool ok = false;
    while(!ok) {
        for(int i = 0; i < 4; i++) {
            ships_count[i] = (rand() % (std::min(field_height, field_width) / 4)) + 1;
        }
        ok = can_place_all_ships();
    }
    field.assign(field_height, std::vector<uint64_t>(field_width, 0));
    arrange_ships_master();
}

bool Game::can_place_all_ships() {
    uint64_t total_size = ships_count[0]*1 + ships_count[1]*2 + ships_count[2]*3 + ships_count[3]*4;
    uint64_t area = field_width * field_height;
    return total_size <= area;
}

void Game::arrange_ships_master() {
    ships.clear();
    for(int t = 0; t < 4; t++) { 
        uint64_t size = t + 1;
        for(uint64_t c = 0; c < ships_count[t]; c++) {
            bool placed = false;
            char orientation;

            if (rand() % 2 == 0) {
                orientation = 'h';
            } else {
                orientation = 'v';
            }

            for(int step = 0; step < 1000 && !placed; step++) {
                uint64_t x = (rand() % field_width);
                uint64_t y = (rand() % field_height);

                bool fits = false;
                if(orientation == 'h' && x + size <= field_width)
                    fits = true;
                if(orientation == 'v' && y + size <= field_height)
                    fits = true;
                if(!fits)
                    continue;

                if(is_area_free(x, y, orientation, size)) {
                    ship_info ship;
                    ship.type = size;
                    ship.hits = 0;
                    for(uint64_t i = 0; i < size; i++) {
                        if(orientation == 'h') {
                            field[y][x + i] = size;
                            std::pair<uint64_t, uint64_t> cell = std::make_pair(x + i, y);
                            ship.cells.push_back(cell);
                        }
                        else {
                            field[y + i][x] = size;
                            std::pair<uint64_t, uint64_t> cell = std::make_pair(x, y + i);
                            ship.cells.push_back(cell);
                        }
                    }
                    mark_forbidden_cells(ship);     // Маркировка окружения корабля
                    ships.push_back(ship);
                    placed = true;
                }
            }

            if(!placed) {
                std::cout << "failed\n";
                return;
            }
        }
    }
}

bool Game::is_area_free(uint64_t x, uint64_t y, char orientation, uint64_t size) {
    for(uint64_t i = 0; i < size; i++) {
        uint64_t current_x;
        if (orientation == 'h') {
            current_x = x + i;
        } else {
            current_x = x;
        }

        uint64_t current_y;
        if (orientation == 'h') {
            current_y = y;
        } else {
            current_y = y + i;
        }

        if(field[current_y][current_x] != 0)
            return false;

        for(int around_x = -1; around_x <=1; around_x++) {
            for(int around_y = -1; around_y <=1; around_y++) {
                int neighbor_x = static_cast<int>(current_x) + around_x;
                int neighbor_y = static_cast<int>(current_y) + around_y;
                if(neighbor_x >= 0 && neighbor_x < static_cast<int>(field_width) &&
                   neighbor_y >= 0 && neighbor_y < static_cast<int>(field_height)) {
                    if(field[neighbor_y][neighbor_x] != 0)
                        return false;
                }
            }
        }
    }
    return true;
}

void Game::mark_forbidden_cells(ship_info &ship) {
    size_t ship_size = ship.cells.size();
    for(size_t i = 0; i < ship_size; i++) {
        uint64_t x = ship.cells[i].first;
        uint64_t y = ship.cells[i].second;

        for(int around_x = -1; around_x <=1; around_x++) {
            for(int around_y = -1; around_y <=1; around_y++) {
                int neighbor_x = static_cast<int>(x) + around_x;
                int neighbor_y = static_cast<int>(y) + around_y;
                if(neighbor_x >= 0 && neighbor_x < static_cast<int>(field_width) &&
                   neighbor_y >= 0 && neighbor_y < static_cast<int>(field_height)) {
                    if(field[neighbor_y][neighbor_x] == 0)
                        field[neighbor_y][neighbor_x] = 5;
                }
            }
        }
    }
}

ship_info* Game::find_ship_at(uint64_t x, uint64_t y) {
    size_t i = 0;
    size_t j = 0;
    for(i = 0; i < ships.size(); i++) {
        for(j = 0; j < ships[i].cells.size(); j++) {
            if(ships[i].cells[j].first == x && ships[i].cells[j].second == y) {
                return &ships[i];
            }
        }
    }
    return NULL;
}

void Game::arrange_ships_slave() {
    ships.clear();
    if(!params_set_by_user) {
        std::cout << "failed\n";
        return;
    }

    field.assign(field_height, std::vector<uint64_t>(field_width, 0));

    for(int t = 0; t < 4; t++) {
        uint64_t size = t + 1;
        for(uint64_t count = 0; count < ships_count[t]; count++) { 
            bool placed = false;
            char orientation;
            if (rand() % 2 == 0) {
                orientation = 'h';
            } else {
                orientation = 'v';
            }

            for(int step = 0; step < 1000 && !placed; step++) {
                uint64_t x = (rand() % field_width);
                uint64_t y = (rand() % field_height);

                bool fits = false;
                if(orientation == 'h' && x + size <= field_width)
                    fits = true;
                if(orientation == 'v' && y + size <= field_height)
                    fits = true;
                if(!fits)
                    continue;

                if(is_area_free(x, y, orientation, size)) {
                    ship_info ship;
                    ship.type = size;
                    ship.hits = 0;
                    for(uint64_t i = 0; i < size; i++) {
                        if(orientation == 'h') {
                            field[y][x + i] = size;
                            std::pair<uint64_t, uint64_t> cell = std::make_pair(x + i, y);
                            ship.cells.push_back(cell);
                        }
                        else {
                            field[y + i][x] = size;
                            std::pair<uint64_t, uint64_t> cell = std::make_pair(x, y + i);
                            ship.cells.push_back(cell);
                        }
                    }
                    mark_forbidden_cells(ship);
                    ships.push_back(ship);
                    placed = true;
                }
            }
             if(!placed) {
                std::cout << "failed\n";
                return;
            }
        }
    }
}

void Game::set_width(uint64_t w) {
    if(w == 0) {
        std::cout << "failed\n";
        return;
    }
    field_width = w;
    params_set_by_user = true;
    std::cout << "ok\n";
}

void Game::get_width() {
    std::cout << field_width << "\n";
}

void Game::set_height(uint64_t h) {
    if(h == 0) {
        std::cout << "failed\n";
        return;
    }
    field_height = h;
    params_set_by_user = true;
    std::cout << "ok\n";
}

void Game::get_height() {
    std::cout << field_height << "\n";
}

void Game::set_count(uint64_t type, uint64_t count) {
    if(type < 1 || type > 4) {
        std::cout << "failed\n";
        return;
    }
    ships_count[type-1] = count;
    params_set_by_user = true;
    std::cout << "ok\n";
}

void Game::get_count(uint64_t type) {
    if(type < 1 || type > 4) {
        std::cout << "0\n";
        return;
    }
    std::cout << ships_count[type-1] << "\n";
}

void Game::set_strategy(std::string &strat) {
    if(strat == "ordered" || strat == "custom") {
        strategy = strat;
        std::cout << "ok\n";
    }
    else {
        std::cout << "failed\n";
    }
}

void Game::start() {
    bool ships_set = false;
    for(int i = 0; i < 4; i++) {
        if(ships_count[i] > 0) {
            ships_set = true;
            break;
        }
    }
    if(field_width > 0 && field_height > 0 && ships_set) {
        game_started = true;
        game_finished = false;
        finish_state = "no";
        win_state = "no";
        lose_state = "no";
        if(mode == "slave") {
            arrange_ships_slave();
        }
        std::cout << "ok\n";
    }
    else {
        std::cout << "failed\n";
    }
}

void Game::stop() {
    game_started = false;
    game_finished = false;
    field_width = 0;
    field_height = 0;
    for(int i = 0; i < 4; i++) {
        ships_count[i] = 0;
    }
    ships.clear();
    field.clear();
    bot_shots_fired.clear();
    finish_state = "no";
    win_state = "no";
    lose_state = "no";
    ordered_x = 0;
    ordered_y = 0;
    custom_x = 0;
    custom_y = 0;
    params_set_by_user = false;
    std::cout << "ok\n";
}

void Game::shot_user(uint64_t x, uint64_t y) {
    if(!game_started) {
        std::cout << "failed\n";
        return;
    }

    // Проверка границ
    if(x < 1 || y < 1 || x > field_width || y > field_height) {
        std::cout << "miss\n";
        shot_bot();
        return;
    }

    uint64_t field_x = x - 1;
    uint64_t field_y = y - 1;

    uint64_t cell = field[field_y][field_x];
    if(cell == 0 || cell == 5) {
        std::cout << "miss\n";
        shot_bot();
    }
    else if(cell >=1 && cell <=4) {
        ship_info* ship = find_ship_at(field_x, field_y);
        if(ship) {
            ship->hits += 1;
            if(ship->hits >= ship->type) {
                std::cout << "kill\n";
                bool all_sunk = true;
                size_t i = 0;
                for(i = 0; i < ships.size(); i++) {
                    if(ships[i].hits < ships[i].type) {
                        all_sunk = false;
                        break;
                    }
                }
                if(all_sunk) {
                    game_finished = true;
                    finish_state = "yes";
                    win_state = "yes";
                    lose_state = "no";
                }
                
            }
            else {
                std::cout << "hit\n";
            }
        }
        else {
            std::cout << "miss\n";
            shot_bot();
        }
    }
    else {
        std::cout << "miss\n";
        shot_bot();
    }
}

void Game::shot_bot() {
    if(!game_started) {
        std::cout << "failed\n";
        return;
    }

    uint64_t x_shot = 0;
    uint64_t y_shot = 0;

    if(strategy == "ordered") {
        if(ordered_y >= field_height) {
            std::cout << "0 0\n"; 
            return;
        }
        x_shot = ordered_x + 1;
        y_shot = ordered_y + 1;
        ordered_x++;
        if(ordered_x >= field_width) {
            ordered_x = 0;
            ordered_y++;
        }
    }
    else {
        if(custom_x >= field_width || custom_y >= field_height) {
            custom_x = 0;
            custom_y = 0;
        }
        x_shot = custom_x + 1;
        y_shot = custom_y + 1;
        custom_x++;
        custom_y++;
    }

    std::pair<uint64_t, uint64_t> shot = std::make_pair(x_shot, y_shot);
    while(bot_shots_fired.find(shot) != bot_shots_fired.end()) {
        x_shot = (rand() % field_width) + 1;
        y_shot = (rand() % field_height) + 1;
        shot = std::make_pair(x_shot, y_shot);
    }
    bot_shots_fired.insert(shot);

    // Обработка выстрела
    if(x_shot < 1 || y_shot < 1 || x_shot > field_width || y_shot > field_height) {
        std::cout << "miss\n";
        return;
    }

    uint64_t field_x = x_shot - 1;
    uint64_t field_y = y_shot - 1;

    uint64_t cell = field[field_y][field_x];
    if(cell == 0 || cell == 5) {
        std::cout << "miss\n";
    }
    else if(cell >=1 && cell <=4) {
        ship_info* ship = find_ship_at(field_x, field_y);
        if(ship) {
            ship->hits += 1;
            if(ship->hits >= ship->type) {
                std::cout << "kill\n";
                bool all_sunk = true;
                size_t i = 0;
                for(i = 0; i < ships.size(); i++) {
                    if(ships[i].hits < ships[i].type) {
                        all_sunk = false;
                        break;
                    }
                }
                if(all_sunk) {
                    game_finished = true;
                    finish_state = "yes";
                    win_state = "no";
                    lose_state = "yes";
                }
                else {
                    shot_bot();
                }
            }
            else {
                std::cout << "hit\n";
                shot_bot();
            }
        }
        else {
            std::cout << "miss\n";
        }
    }
    else {
        std::cout << "miss\n";
    }
}

void Game::set_result(std::string &res) {
    if(res == "miss" || res == "hit" || res == "kill") {
        std::cout << "ok\n";
    }
    else {
        std::cout << "failed\n";
    }
}

void Game::finished() {
    std::cout << finish_state << "\n";
}

void Game::win() {
    std::cout << win_state << "\n";
}

void Game::lose() {
    std::cout << lose_state << "\n";
}

void Game::dump(std::string &path) {
    std::ofstream file(path.c_str());
    if(!file.is_open()) {
        std::cout << "failed\n";
        return;
    }

    file << field_width << " " << field_height << "\n";

    for(int i = 0; i < 4; i++) {
        file << ships_count[i] << " ";
    }
    file << "\n";

    for(size_t i = 0; i < ships.size(); i++) {
        file << ships[i].type << " " << ships[i].hits << " ";
        for(size_t j = 0; j < ships[i].cells.size(); j++) {
            file << ships[i].cells[j].first << " " << ships[i].cells[j].second << " ";
        }
        file << "\n";
    }

    file.close();
    std::cout << "ok\n";
}

void Game::load(std::string &path) {
    std::ifstream file(path.c_str());
    if(!file.is_open()) {
        std::cout << "failed\n";
        return;
    }

    uint64_t w = 0, h = 0;
    file >> w >> h;
    if(w == 0 || h == 0) {
        std::cout << "failed\n";
        file.close();
        return;
    }
    field_width = w;
    field_height = h;

    for(int i = 0; i < 4; i++) {
        file >> ships_count[i];
    }

    field.assign(field_height, std::vector<uint64_t>(field_width, 0));

    ships.clear();
    std::string line;
    while(std::getline(file, line)) {
        if(line.empty())
            continue;
        std::istringstream iss(line);
        uint64_t type = 0, hits = 0;
        std::cin >> type >> hits;
        ship_info ship;
        ship.type = type;
        ship.hits = hits;
        uint64_t x = 0, y = 0;
        while(std::cin >> x >> y) {
            if(x < field_width && y < field_height) {
                ship.cells.push_back(std::make_pair(x, y));
                field[y][x] = type;
            }
        }
        ships.push_back(ship);
    }

    for(size_t i = 0; i < ships.size(); i++) {
        mark_forbidden_cells(ships[i]);
    }

    file.close();
    std::cout << "ok\n";
}

int main() {
    Game game;
    std::string cmd;

    while (std::cin >> cmd) {
        if (cmd == "create") {
            std::string mode;
            std::cin >> mode;
            game.create_player(mode);
        }
        else if (cmd == "set") {
            std::string command;
            std::cin >> command;
            if(command == "width") {
                uint64_t width;
                std::cin >> width;
                game.set_width(width);
            }
            else if(command == "height") {
                uint64_t height;
                std::cin >> height;
                game.set_height(height);
            }
            else if(command == "count") {
                uint64_t type, count;
                std::cin >> type >> count;
                game.set_count(type, count);
            }
            else if(command == "strategy") {
                std::string strat;
                std::cin >> strat;
                game.set_strategy(strat);
            }
            else if(command == "result") {
                std::string res;
                std::cin >> res;
                game.set_result(res);
            }
            else {
                std::cout << "unknown command\n";
            }
        }
        else if (cmd == "get") {
            std::string subcommand;
            std::cin >> subcommand;
            if(subcommand == "width") {
                game.get_width();
            }
            else if(subcommand == "height") {
                game.get_height();
            }
            else if(subcommand == "count") {
                uint64_t type;
                std::cin >> type;
                game.get_count(type);
            }
            else {
                std::cout << "unknown command\n";
            }
        }
        else if (cmd == "start") {
            game.start();
        }
        else if (cmd == "stop") {
            game.stop();
        }
        else if (cmd == "shot") {
            uint64_t x, y;
            std::cin >> x >> y;
            game.shot_user(x, y);
        }
        else if (cmd == "finished") {
            game.finished();
        }
        else if (cmd == "win") {
            game.win();
        }
        else if (cmd == "lose") {
            game.lose();
        }
        else if (cmd == "dump") {
            std::string path;
            std::cin >> path;
            game.dump(path);
        }
        else if (cmd == "load") {
            std::string path;
            std::cin >> path;
            game.load(path);
        }
        else if (cmd == "exit") {
            break;
        }
        else {
            std::cout << "unknown command\n";
        }
    }

    return 0;
}
