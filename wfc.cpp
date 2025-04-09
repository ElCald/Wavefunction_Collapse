// Wave Function Collapse en C++
// Version simplifiée pour motifs binaires (0 et 1)

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <ctime>
#include <algorithm>

using namespace std;

const int TILE_SIZE = 2;
const int OUTPUT_WIDTH = 10;
const int OUTPUT_HEIGHT = 10;

using Tile = vector<vector<int>>;

// Compare deux tuiles
bool areTilesEqual(const Tile &a, const Tile &b) {
    for (int i = 0; i < TILE_SIZE; ++i)
        for (int j = 0; j < TILE_SIZE; ++j)
            if (a[i][j] != b[i][j]) return false;
    return true;
}

// Affiche une tuile
void printTile(const Tile &tile) {
    for (const auto &row : tile) {
        for (int v : row) cout << v << " ";
        cout << "\n";
    }
}

// Grille de l'output, chaque cellule contient les indices des tuiles possibles
using Wave = vector<vector<set<int>>>;

// Exemple d'input : motif binaire 4x4
vector<vector<int>> input = {
    {1, 0, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 1, 0},
    {0, 1, 0, 1}
};


vector<vector<int>> input2 = {
    {0, 0, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 0},
    {1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 1, 0},
};

// Extrait toutes les tuiles possibles de l'input
vector<Tile> extractTiles(const vector<vector<int>> &input) {
    vector<Tile> tiles;
    int w = input[0].size(), h = input.size();
    for (int y = 0; y <= h - TILE_SIZE; ++y) {
        for (int x = 0; x <= w - TILE_SIZE; ++x) {
            Tile tile(TILE_SIZE, vector<int>(TILE_SIZE));
            for (int dy = 0; dy < TILE_SIZE; ++dy)
                for (int dx = 0; dx < TILE_SIZE; ++dx)
                    tile[dy][dx] = input[y + dy][x + dx];

            // Unicité
            bool exists = false;
            for (const auto &t : tiles)
                if (areTilesEqual(t, tile)) {
                    exists = true;
                    break;
                }
            if (!exists)
                tiles.push_back(tile);
        }
    }
    return tiles;
}

// Génère les contraintes de chevauchement : ici, simple vérification que les bords correspondent
bool tilesCanOverlap(const Tile &a, const Tile &b, int dx, int dy) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int ax = x, ay = y;
            int bx = x - dx, by = y - dy;
            if (bx >= 0 && by >= 0 && bx < TILE_SIZE && by < TILE_SIZE)
                if (a[ay][ax] != b[by][bx]) return false;
        }
    }
    return true;
}

// Génère les voisins compatibles pour chaque tuile
map<int, map<pair<int, int>, set<int>>> computeAdjacency(const vector<Tile> &tiles) {
    map<int, map<pair<int, int>, set<int>>> adjacency;
    for (int i = 0; i < tiles.size(); ++i) {
        for (int j = 0; j < tiles.size(); ++j) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (abs(dx) + abs(dy) != 1) continue;
                    if (tilesCanOverlap(tiles[i], tiles[j], dx, dy)) {
                        adjacency[i][{dx, dy}].insert(j);
                    }
                }
            }
        }
    }
    return adjacency;
}

// Sélectionne la cellule de plus faible entropie
pair<int, int> findLowestEntropy(const Wave &wave) {
    int minChoices = 9999;
    pair<int, int> result = {-1, -1};
    for (int y = 0; y < OUTPUT_HEIGHT; ++y) {
        for (int x = 0; x < OUTPUT_WIDTH; ++x) {
            int options = wave[y][x].size();
            if (options > 1 && options < minChoices) {
                minChoices = options;
                result = {x, y};
            }
        }
    }
    return result;
}

// Propagation des contraintes
void propagate(Wave &wave, const map<int, map<pair<int, int>, set<int>>> &adjacency) {
    bool changed;
    do {
        changed = false;
        for (int y = 0; y < OUTPUT_HEIGHT; ++y) {
            for (int x = 0; x < OUTPUT_WIDTH; ++x) {
                if (wave[y][x].size() == 1) continue;

                for (auto it = wave[y][x].begin(); it != wave[y][x].end(); ) {
                    int tile = *it;
                    bool valid = true;
                    for (auto [dx, dy_set] : adjacency.at(tile)) {
                        int nx = x + dx.first, ny = y + dx.second;
                        if (nx < 0 || ny < 0 || nx >= OUTPUT_WIDTH || ny >= OUTPUT_HEIGHT) continue;

                        const auto &neighborSet = wave[ny][nx];
                        bool found = false;
                        for (int t : neighborSet) {
                            if (adjacency.at(tile).at(dx).count(t)) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            valid = false;
                            break;
                        }
                    }
                    if (!valid) {
                        it = wave[y][x].erase(it);
                        changed = true;
                    } else ++it;
                }
            }
        }
    } while (changed);
}

int main() {
    srand(time(nullptr));
    auto tiles = extractTiles(input2);
    auto adjacency = computeAdjacency(tiles);

    // Initialise la grille
    Wave wave(OUTPUT_HEIGHT, vector<set<int>>(OUTPUT_WIDTH));

    for (int y = 0; y < OUTPUT_HEIGHT; ++y)
        for (int x = 0; x < OUTPUT_WIDTH; ++x)
            for (int i = 0; i < tiles.size(); ++i)
                wave[y][x].insert(i);
                

    while (true) {
        auto [x, y] = findLowestEntropy(wave);
        if (x == -1) break;

        // Choix aléatoire
        vector<int> options(wave[y][x].begin(), wave[y][x].end());
        int choice = options[rand() % options.size()];
        wave[y][x] = {choice};

        propagate(wave, adjacency);
    }

    // Affiche résultat (coin haut gauche de chaque tuile)
    for (int y = 0; y < OUTPUT_HEIGHT; ++y) {
        for (int x = 0; x < OUTPUT_WIDTH; ++x) {
            int id = *wave[y][x].begin();
            cout << tiles[id][0][0] << " ";
        }
        cout << "\n";
    }
    return 0;
}
