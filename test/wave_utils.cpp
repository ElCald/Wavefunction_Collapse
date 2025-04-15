#include "wave_utils.h"

using namespace std;
using namespace cv;

void Tile::setRules(const vector<Tile *> &allTiles)
{
    for (auto &tile : allTiles)
    {
        if (edges[0] == tile->edges[2])
            up.push_back(tile);
        if (edges[1] == tile->edges[3])
            right.push_back(tile);
        if (edges[2] == tile->edges[0])
            down.push_back(tile);
        if (edges[3] == tile->edges[1])
            left.push_back(tile);
    }
}

void Tile::draw(Mat &canvas, int x, int y, int size) const
{
    Mat resized;
    resize(image, resized, Size(size, size));
    resized.copyTo(canvas(Rect(x, y, size, size)));
}

int Cell::entropy() const
{
    return static_cast<int>(options.size());
}

void Cell::draw(Mat &canvas, int tileSize) const
{
    if (collapsed && !options.empty())
    {
        options[0]->draw(canvas, x * tileSize, y * tileSize, tileSize);
    }
}

void Cell::observe(mt19937 &rng)
{
    if (!collapsed && !options.empty())
    {
        uniform_int_distribution<int> dist(0, static_cast<int>(options.size() - 1));
        options = {options[dist(rng)]};
        collapsed = true;
    }
}

void Cell::update()
{
    collapsed = (options.size() == 1);
}

void Grid::draw(Mat &canvas)
{
    for (auto &row : cells)
    {
        for (auto &cell : row)
        {
            cell.draw(canvas, tileSize);
        }
    }
}

// Randomise toutes les cellules pour test visuel (non-collapse)
void Grid::randomizeAll(mt19937 &rng)
{
    for (auto &row : cells)
    {
        for (auto &cell : row)
        {
            uniform_int_distribution<int> dist(0, static_cast<int>(cell.options.size() - 1));
            int choice = dist(rng);
            cell.options = {cell.options[choice]};
            cell.collapsed = true;
        }
    }
}

Cell *Grid::heuristicPick()
{
    vector<Cell *> uncollapsed;

    for (auto &row : cells)
    {
        for (auto &cell : row)
        {
            if (!cell.collapsed && !cell.options.empty())
                uncollapsed.push_back(&cell);
        }
    }

    if (uncollapsed.empty())
        return nullptr;

    sort(uncollapsed.begin(), uncollapsed.end(),
         [](const Cell *a, const Cell *b)
         {
             return a->entropy() < b->entropy();
         });

    int minEntropy = uncollapsed.front()->entropy();
    vector<Cell *> lowestEntropy;
    copy_if(uncollapsed.begin(), uncollapsed.end(), back_inserter(lowestEntropy),
            [minEntropy](Cell *c)
            { return c->entropy() == minEntropy; });

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(0, static_cast<int>(lowestEntropy.size() - 1));
    return lowestEntropy[dist(rng)];
}

Cell *Grid::getCell(int x, int y)
{
    if (x < 0 || y < 0 || x >= cols || y >= rows)
        return nullptr;
    return &cells[y][x];
}

void Grid::collapse(std::mt19937 &gen)
{
    // 1. Trouver les cellules non encore collapsées
    vector<Cell *> nonCollapsed;
    for (auto &row : cells)
    {
        for (auto &cell : row)
        {
            if (!cell.collapsed)
            {
                nonCollapsed.push_back(&cell);
            }
        }
    }

    // 2. Si toutes les cellules sont collapsées, on a fini
    if (nonCollapsed.empty())
        return;

    // 3. Trouver celles avec l'entropie minimale
    size_t minEntropy = SIZE_MAX;
    for (Cell *c : nonCollapsed)
    {
        if (c->options.size() < minEntropy)
        {
            minEntropy = c->options.size();
        }
    }

    // 4. En sélectionner une au hasard parmi celles avec entropie minimale
    vector<Cell *> candidates;
    for (Cell *c : nonCollapsed)
    {
        if (c->options.size() == minEntropy)
        {
            candidates.push_back(c);
        }
    }

    std::uniform_int_distribution<> dist(0, candidates.size() - 1);
    Cell *chosen = candidates[dist(gen)];

    // 5. Appliquer la contrainte des voisins
    int x = chosen->x;
    int y = chosen->y;

    vector<Tile *> cumulative = chosen->options;

    Cell *top = getCell(x, y - 1);
    Cell *right = getCell(x + 1, y);
    Cell *bottom = getCell(x, y + 1);
    Cell *left = getCell(x - 1, y);

    vector<pair<Cell *, const vector<Tile *> Tile::*>> neighbors = {
        {top, &Tile::down},
        {right, &Tile::left},
        {bottom, &Tile::up},
        {left, &Tile::right}};

    for (const auto &[neighbor, direction] : neighbors)
    {
        if (neighbor == nullptr || neighbor->options.empty())
            continue;

        vector<Tile *> validFromNeighbor;
        for (Tile *neighborTile : neighbor->options)
        {
            const vector<Tile *> &compatible = neighborTile->*direction;
            validFromNeighbor.insert(validFromNeighbor.end(), compatible.begin(), compatible.end());
        }

        vector<Tile *> filtered;
        for (Tile *tile : cumulative)
        {
            if (std::find(validFromNeighbor.begin(), validFromNeighbor.end(), tile) != validFromNeighbor.end())
            {
                filtered.push_back(tile);
            }
        }

        cumulative = filtered;
    }

    // 6. Collapse : choisir un tile parmi les options valides
    if (!cumulative.empty())
    {
        std::uniform_int_distribution<> tileDist(0, cumulative.size() - 1);
        Tile *selected = cumulative[tileDist(gen)];
        chosen->collapsed = true;
        chosen->options = {selected};
    }
}

Mat loadAndResizeImage(const string &path, int size)
{
    Mat img = imread(path, IMREAD_UNCHANGED);
    if (img.empty())
    {
        cerr << "Erreur de chargement : " << path << endl;
        exit(EXIT_FAILURE);
    }
    resize(img, img, Size(size, size));
    return img;
}

vector<Tile *> loadTiles(const vector<string> &assetPaths, const vector<vector<int>> &edgeData, int tileSize)
{
    vector<Tile *> tiles;
    for (size_t i = 0; i < assetPaths.size(); ++i)
    {
        Mat img = loadAndResizeImage(assetPaths[i], tileSize);
        tiles.push_back(new Tile(img, static_cast<int>(i), edgeData[i]));
    }

    // Définir les règles de compatibilité pour chaque tuile
    for (auto &tile : tiles)
    {
        tile->setRules(tiles);
    }

    return tiles;
}