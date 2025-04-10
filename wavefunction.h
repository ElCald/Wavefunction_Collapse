#ifndef WAVEFUNCTION_H  
#define WAVEFUNCTION_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <array>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <random>
#include <climits> 

#include <omp.h>


using namespace std;


#define TILE_SIZE 2 // Taille des tuiles
#define GRILLE_SIZE_WIDTH 20
#define GRILLE_SIZE_HEIGHT 20



using Tile = vector<vector<int>>; // Tuile
using Wave_grid = vector<vector<set<int>>>; // Grille qui contient l'indice des tuiles avec comme point de repère le coin au gauche d'une tuile
using vector2D = vector<vector<int>>;
/*  
    Double dictionnaire: map<clé, valeur>
    On stocke dans la première clé, le numéro de la tuile.
    Dans la deuxième clé, l'offset calculé.
    Au final on a un set d'indice de tuiles qui peuvent être adjacente à la tuile (clé 1) pour l'offset
    donnée (clé 2). 
*/
using dicoADJtiles = map<int, map<pair<int, int>, set<int>>>; 



int tile_is_in_list(vector<Tile>& tiles, Tile& t);
void save_tiles_from_grid_sample(vector<Tile>& tiles, vector<int>& num_tile, vector2D& grid_sample);
bool tiles_can_overlap(const Tile &a, const Tile &b, int x, int y);
dicoADJtiles compute_adjacency(const vector<Tile> &tiles);
void entropy(Wave_grid &grille, dicoADJtiles& dicoADJ);
pair<int, int> find_lowest_entropy(const Wave_grid &grille);
void print_tiles_list(vector<Tile>& tiles);
void print_tiles_list(vector<Tile>& tiles, vector<int>& num_tile);
Tile rotateTile(const Tile& matrix);
void print_tile(Tile tile);




#endif