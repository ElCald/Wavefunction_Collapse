#include "wave_utils.h"
#include <omp.h>

using namespace std;


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

void Tile::draw(Image_int &canvas, int x, int y, int size) const
{
   // Mise de la tuile dans l'image finale
}

int Cell::entropy() const
{
    return static_cast<int>(options.size());
}

void Cell::draw(Image_int &canvas, int tileSize) const
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

void Grid::draw(Image_int &canvas)
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

    sort(uncollapsed.begin(), uncollapsed.end(), [](const Cell *a, const Cell *b){
        return a->entropy() < b->entropy();
    });

    int minEntropy = uncollapsed.front()->entropy();
    vector<Cell *> lowestEntropy;
    std::copy_if(uncollapsed.begin(), uncollapsed.end(), back_inserter(lowestEntropy), [minEntropy](Cell *c){ 
        return c->entropy() == minEntropy; 
    });

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
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            if (!cells.at(i).at(j).collapsed)
            {
                nonCollapsed.push_back(&cells.at(i).at(j));
            }
        }
    }
        


    // 2. Si toutes les cellules sont collapsées, on a fini
    if (nonCollapsed.empty())
        return;


    // 3. Trouver celles avec l'entropie minimale
    size_t minEntropy = SIZE_MAX;

    for(size_t i=0; i<nonCollapsed.size(); i++){
        if (nonCollapsed.at(i)->options.size() < minEntropy)
        {
            minEntropy = nonCollapsed.at(i)->options.size();
        }
    }
 


    // 4. En sélectionner la 1ere parmi celles avec entropie minimale
    vector<Cell *> candidates;

    for(size_t i=0; i<nonCollapsed.size(); i++){
        if (nonCollapsed.at(i)->options.size() == minEntropy)
        {
            candidates.push_back(nonCollapsed.at(i));
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
        {left, &Tile::right}
    };


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


bool Grid::is_ready(){

    for(size_t i=0; i<cells.size(); i++){
        for(size_t j=0; j<cells.at(i).size(); j++){
            if(!cells.at(i).at(j).collapsed){
                return false;
            }
        }
    }
    return true;
}


vector<Tile *> loadTiles(const vector<string> &assetPaths, const vector<vector<int>> &edgeData, int tileSize)
{
    vector<Tile *> tiles;
    for (size_t i = 0; i < assetPaths.size(); ++i)
    {
        tiles.push_back(new Tile(static_cast<int>(i), edgeData[i]));
    }

    // Définir les règles de compatibilité pour chaque tuile
    for (auto &tile : tiles)
    {
        tile->setRules(tiles);
    }

    return tiles;
}


vector<Tile *> loadTiles(const int num_tiles, const vector<vector<int>> &edgeData, int tileSize)
{
    vector<Tile *> tiles;
    for (int i = 0; i < num_tiles; ++i)
    {
        tiles.push_back(new Tile(static_cast<int>(i), edgeData[i]));
    }

    // Définir les règles de compatibilité pour chaque tuile
    for (auto &tile : tiles)
    {
        tile->setRules(tiles);
    }

    return tiles;
}


/**
 * @param n Nombre de tuiles
 * @param d Profondeur
 * 
 * @return Tableau de tâches
 */
int* generateTasks(int n, int d) {
    int rows = pow(n, d);
    int* data = new int[rows * d];

    for (int i = 0; i < rows; ++i) {
        int value = i;
        for (int j = d - 1; j >= 0; --j) {
            data[i * d + j] = value % n;
            value /= n;
        }
    }

    return data;
}


std::pair<int, int> damier_coords(int k, int cols) {
    int half_cols = cols / 2;
    int row = k / half_cols;
    int col_in_row = k % half_cols;
    int col;

    if (row % 2 == 0) {
        col = col_in_row * 2;       // colonnes paires pour lignes paires
    } else {
        col = col_in_row * 2 + 1;   // colonnes impaires pour lignes impaires
    }

    return {row, col};
}



/**
 * Extraction des tuiles qui se trouvent dans le sample initiale en input
 *
 * @param tiles Liste qui contiendra les tuiles de la grille
 * @param grid_sample Grille de
 */
void save_tiles_from_grid_sample(vector<vector<vector<int>>> &tiles, vector<int> &num_tile, const vector<vector<int>> &grid_sample, int tile_size)
{
    const size_t rows = grid_sample.size();
    const size_t cols = grid_sample[0].size();

    for (size_t i = 0; i <= rows - tile_size; i+=tile_size-1)
    {
        for (size_t j = 0; j <= cols - tile_size; j+=tile_size-1)
        {
            // Extraction d'une tuile TILE_SIZE x TILE_SIZE
            vector<vector<int>> tile(tile_size, vector<int>(tile_size));

            for (int k = 0; k < tile_size; ++k)
            {
                for (int l = 0; l < tile_size; ++l)
                {
                    tile[k][l] = grid_sample[i + k][j + l];
                }
            }

            // Vérifie si la tuile est déjà présente
            int indice_tile = tile_is_in_list(tiles, tile);
            if (indice_tile == -1)
            {
                tiles.push_back(tile); // move utile ici
                num_tile.push_back(1);
            }
            else
                ++num_tile[indice_tile];
        }
    }
}



/**
 * Permet de savoir si une tuile se trouve dans notre liste
 *
 * @param tiles Liste de tuiles
 * @param t Tuile a tester
 *
 * @return L'indice de la position de la tuile dans la liste sinon -1
 */
int tile_is_in_list(const vector<vector<vector<int>>> &tiles, const vector<vector<int>> &t)
{
    for (size_t i = 0; i < tiles.size(); ++i)
    {
        if (tiles[i] == t)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}



/**
 * Affichage de toutes les tuiles enregistrées avec leur nombre
 *
 * @param tiles
 */
void print_tiles_list(vector<vector<vector<int>>> &tiles, int tile_size)
{
    int k = 0;

    for (vector<vector<int>> &tile : tiles)
    {

        cout << "Tile: " << k << endl;

        print_tile(tile, tile_size);

        k++;

        cout << endl;
    }
}

/**
 * Affichage d'une tuile
 *
 * @param tile
 */
void print_tile(const vector<vector<int>> tile, int tile_size)
{

    for (int i = 0; i < tile_size; i++)
    {
        for (int j = 0; j < tile_size; j++)
        {
            cout << tile.at(i).at(j) << " ";
        }
        cout << endl;
    }
    cout << endl;
}


int find_bord_in_list(vector<vector<int>>& bords, vector<int>& e){

    for(size_t i=0; i<bords.size(); i++){

        if(bords.at(i) == e){
            return i;
        }

    }

    return -1;
}



/**
 * @param liste_tuiles
 * @param edges 
 * @param tile_size
 */
void generate_edges(vector<vector<vector<int>>>& liste_tuiles, vector<vector<int>>& edges, int tile_size){

    vector<vector<int>> bords;
    vector<int> bord(tile_size);
    vector<int> edge;
    int k=0;
    int indice=0;

    for(size_t i=0; i<liste_tuiles.size(); i++){ // parcours de la liste de tuile

        edge.clear();



        // ---- bord haut ----
        bord.clear();
        bord.resize(tile_size);
        k=0;
        for(int j=0; j<tile_size; j++){ 
            bord.at(k) = liste_tuiles.at(i).at(0).at(j);
            k++;
        }

        if(find_bord_in_list(bords, bord) == -1){
            bords.push_back(bord);
        }

        indice = find_bord_in_list(bords, bord);
        edge.push_back(indice);


        // ---- bord droite ----
        bord.clear();
        bord.resize(tile_size);
        k=0;
        for(int j=0; j<tile_size; j++){ 
            bord.at(k) = liste_tuiles.at(i).at(j).at(tile_size-1);
            k++;
        }

        if(find_bord_in_list(bords, bord) == -1){
            bords.push_back(bord);
        }

        indice = find_bord_in_list(bords, bord);
        edge.push_back(indice);
        

        
        // ---- bord bas ----
        bord.clear();
        bord.resize(tile_size);
        k=0;
        for(int j=0; j<tile_size; j++){ 
            bord.at(k) = liste_tuiles.at(i).at(tile_size-1).at(j);
            k++;
        }

        if(find_bord_in_list(bords, bord) == -1){
            bords.push_back(bord);
        }

        indice = find_bord_in_list(bords, bord);
        edge.push_back(indice);
 


        // ---- bord gauche ----
        bord.clear();
        bord.resize(tile_size);
        k=0;
        for(int j=0; j<tile_size; j++){ 
            bord.at(k) = liste_tuiles.at(i).at(j).at(0);
            k++;
        }

        if(find_bord_in_list(bords, bord) == -1){
            bords.push_back(bord);
        }

        indice = find_bord_in_list(bords, bord);
        edge.push_back(indice);
     

  
   

        // fin
        edges.push_back(edge);
    }

}



void print_edges(vector<vector<int>>& edges){
    for(size_t i=0; i<edges.size(); i++){
        cout << "Tile: " << i << " >> ";
        for(size_t j=0; j<edges.at(i).size(); j++){
            cout << edges.at(i).at(j) << " ";
        }
        cout << endl;
    }
}