#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <random>

using namespace std;


using Image_int = vector<vector<int>>;

#define TILE_SIZE 25 // Taille des tuiles

class Tile
{
public:

    int index;
    vector<int> edges; // [up, right, down, left]
    vector<Tile *> up, right, down, left;

    Tile(int idx, const vector<int> &edgeVals)
        : index(idx), edges(edgeVals) {}

    void setRules(const vector<Tile *> &allTiles);
    void draw(Image_int &canvas, int x, int y, int size) const;
};

class Cell
{
public:
    int x, y;
    vector<Tile *> options;
    bool collapsed = false;

    Cell(int x_, int y_, const vector<Tile *> &tiles)
        : x(x_), y(y_), options(tiles) {}

    int entropy() const;
    void draw(Image_int &canvas, int tileSize) const;
    void observe(mt19937 &rng); // collapses to one tile
    void update();              // updates collapsed state
};

class Grid
{
public:
    int cols, rows, tileSize;
    vector<vector<Cell>> cells;

    Grid(int width, int height, int tileSize_, const vector<Tile *> &tiles)
        : tileSize(tileSize_)
    {
        cols = width / tileSize;
        rows = height / tileSize;

        // Crée une grille de cellules
        cells.resize(rows, vector<Cell>(cols, Cell(0, 0, tiles)));

        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                cells[y][x] = Cell(x, y, tiles);
            }
        }
    }

    void draw(Image_int &canvas);
    void randomizeAll(mt19937 &rng);
    Cell *heuristicPick();
    Cell *getCell(int x, int y);
    void collapse(mt19937 &rng);
    bool is_ready();
};


// Crée une liste de tuiles à partir de fichiers image et de leurs règles d’arêtes
vector<Tile *> loadTiles(const vector<string> &assetPaths, const vector<vector<int>> &edgeData, int tileSize);
vector<Tile *> loadTiles(const int num_tiles, const vector<vector<int>> &edgeData, int tileSize);

int* generateTasks(int n, int d);

std::pair<int, int> damier_coords(int k, int cols);

void save_tiles_from_grid_sample(vector<vector<vector<int>>> &tiles, vector<int> &num_tile, const vector<vector<int>> &grid_sample, int tile_size);
int tile_is_in_list(const vector<vector<vector<int>>> &tiles, const vector<vector<int>> &t);
void print_tiles_list(vector<vector<vector<int>>> &tiles, int tile_size);
void print_tile(const vector<vector<int>> tile, int tile_size);
int find_bord_in_list(vector<vector<int>>& bords, vector<int>& e);
void generate_edges(vector<vector<vector<int>>>& liste_tuiles, vector<vector<int>>& edges, int tile_size);
void print_edges(vector<vector<int>>& edges);