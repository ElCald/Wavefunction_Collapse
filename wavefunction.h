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
#include <queue>
#include <set>
#include <map>
#include <random>
#include <climits>
#include <opencv2/opencv.hpp>

#include <omp.h>

using namespace std;

#define TILE_SIZE 25 // Taille des tuiles

#define IMAGE_WIDTH 64
#define IMAGE_HEIGHT 64

#define GRILLE_SIZE_WIDTH ((int)(IMAGE_WIDTH / TILE_SIZE) + 1)
#define GRILLE_SIZE_HEIGHT ((int)(IMAGE_HEIGHT / TILE_SIZE) + 1)

using Tile = vector<vector<int>>;           // Tuile
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
using Image = vector<vector<int>>;

int tile_is_in_list(vector<Tile> &tiles, Tile &t);
void save_tiles_from_grid_sample(vector<Tile> &tiles, vector<int> &num_tile, vector2D &grid_sample);
dicoADJtiles compute_adjacency(const vector<Tile> &tiles);
void entropy(Wave_grid &grille, const dicoADJtiles dicoADJ);
void entropy_2(Wave_grid &grille, const dicoADJtiles dicoADJ);
pair<int, int> find_lowest_entropy(const Wave_grid &grille);
void print_tiles_list(vector<Tile> &tiles);
void print_tiles_list(vector<Tile> &tiles, vector<int> &num_tile);
void print_tile(const Tile tile);
void print_dico(const int n, dicoADJtiles dico);
void print_vector2D(const vector2D vec);

cv::Mat vectorToMat(const std::vector<std::vector<int>> &vec);
std::vector<std::vector<int>> matToVector(const cv::Mat &mat);
cv::Mat intMatrixToImage(const std::vector<std::vector<int>> &intMatrix);
int rgbToInt(int r, int g, int b);
void write_image_from_grille(const Wave_grid &grille, vector2D &image, const vector<Tile> &list_tile);

#endif