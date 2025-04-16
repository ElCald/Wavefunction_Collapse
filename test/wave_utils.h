#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <random>

using namespace std;
using namespace cv;

class Tile
{
public:
    Mat image;
    int index;
    vector<int> edges; // [up, right, down, left]
    vector<Tile *> up, right, down, left;

    Tile(const Mat &img, int idx, const vector<int> &edgeVals)
        : image(img.clone()), index(idx), edges(edgeVals) {}

    void setRules(const vector<Tile *> &allTiles);
    void draw(Mat &canvas, int x, int y, int size) const;
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
    void draw(Mat &canvas, int tileSize) const;
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

    void draw(Mat &canvas);
    void randomizeAll(mt19937 &rng);
    Cell *heuristicPick();
    Cell *getCell(int x, int y);
    void collapse(mt19937 &rng);
    bool is_ready();
};

// Charge une image avec OpenCV et la redimensionne
Mat loadAndResizeImage(const string &path, int size);

// Crée une liste de tuiles à partir de fichiers image et de leurs règles d’arêtes
vector<Tile *> loadTiles(const vector<string> &assetPaths, const vector<vector<int>> &edgeData, int tileSize);