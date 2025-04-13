/**
 * Wavefunction Collape
 *
 *
 * @version 1.0
 * @author Eliot CALD
 * @author Clement JOURD'HEUIL
 */

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
#include <opencv2/opencv.hpp>

#include <omp.h>

#include "wavefunction.h"

using namespace std;

/**
 * - Enregistrer une liste de tuile de taille N depuis une grille de taille S
 *      > Savoir si une tuile est déjà dans la liste
 */

/**
 * Permet de savoir si une tuile se trouve dans notre liste
 *
 * @param tiles Liste de tuiles
 * @param t Tuile a tester
 *
 * @return L'indice de la position de la tuile dans la liste sinon -1
 */
int tile_is_in_list(vector<Tile> &tiles, Tile &t)
{

    bool res = false;
    int i = 0;

    for (Tile tile : tiles)
    {

        if (tile == t)
        {
            res = true;
            break;
        }
        i++;
    }

    if (res)
        return i;
    else
        return -1;
}

/**
 * Extraction des tuiles qui se trouvent dans le sample initiale en input
 *
 * @param tiles Liste qui contiendra les tuiles de la grille
 * @param grid_sample Grille de
 */
void save_tiles_from_grid_sample(vector<Tile> &tiles, vector<int> &num_tile, vector2D &grid_sample)
{
    int indice_tile;

    // Parcours de la grille
    for (size_t i = 0; i < grid_sample.size() - TILE_SIZE + 1; i++)
    {
        for (size_t j = 0; j < grid_sample.at(i).size() - TILE_SIZE + 1; j++)
        {

            Tile tile(TILE_SIZE, vector<int>(TILE_SIZE));

            for (int k = 0; k < TILE_SIZE; k++)
            {
                for (int l = 0; l < TILE_SIZE; l++)
                {
                    tile[k][l] = grid_sample.at(i + k).at(j + l);
                }
            }

            // On vérifie si la tuile existe déjà, ce qui nous permet de compter le nombre qu'on en trouve dans le sample
            if ((indice_tile = tile_is_in_list(tiles, tile)) == -1)
            {
                tiles.push_back(tile);
                num_tile.push_back(1);
            }
            else
            {
                num_tile.at(indice_tile) += 1;
            }
        }
    }

    /* À voir pour save les symétries et les rotations */
}

/**
 * Génère les voisins compatibles pour chaque tuile
 *
 * @param tiles Liste de tuiles
 *
 * @return Dictionnaire des tuiles adjacentes pour toutes les tuiles
 */
dicoADJtiles compute_adjacency(const vector<Tile> &tiles)
{

    dicoADJtiles dico;

    bool isCompatible = true;

    int i_start_A, i_end_A, j_start_A, j_end_A;
    int i_start_B, j_start_B;

    int i_tile_B = 0, j_tile_B = 0;

    int real_x = 0, real_y = 0;

    for (int ta = 0; ta < (int)tiles.size(); ta++)
    { // Parcours de toutes les tuiles du vecteur (ta est l'indice de la tuile_A dans la liste)

        // OFFSET
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {

                if (x == 0 && y == 0)
                { // Pas besoin de traiter le cas (0,0)
                    continue;
                }

                real_x = x * (TILE_SIZE - 1);
                real_y = y * (TILE_SIZE - 1);

                // if((x>-(TILE_SIZE-1) && x<TILE_SIZE-1) && (y>-(TILE_SIZE-1) && y<TILE_SIZE-1)){
                //     continue;
                // }

                for (int tb = 0; tb < (int)tiles.size(); tb++)
                {

                    isCompatible = true;

                    i_start_A = (real_x < 0) ? 0 : real_x;
                    i_end_A = (real_x < 0) ? (TILE_SIZE + real_x) : TILE_SIZE;
                    j_start_A = (real_y < 0) ? 0 : real_y;
                    j_end_A = (real_y < 0) ? (TILE_SIZE + real_y) : TILE_SIZE;

                    i_start_B = (real_x < 0) ? std::abs(real_x) : 0;
                    j_start_B = (real_y < 0) ? std::abs(real_y) : 0;

                    i_tile_B = i_start_B;

                    for (int i = i_start_A; i < i_end_A; i++)
                    {
                        j_tile_B = j_start_B;
                        for (int j = j_start_A; j < j_end_A; j++)
                        {

                            if (tiles.at(ta).at(i).at(j) != tiles.at(tb).at(i_tile_B).at(j_tile_B))
                            {
                                isCompatible = false;
                            }

                            j_tile_B++;
                        }
                        i_tile_B++;
                    }

                    if (isCompatible)
                    {
                        /* Insertion pour la tuile A de la tuile B aux coordonnées (x,y) */
                        dico[ta][{x, y}].insert(tb);
                    }
                }
            }
        }

    } // fin boucle liste tiles

    return dico;
}

/**
 * Sélectionne la case de plus faible entropie
 *
 * @param grille
 *
 * @return Les coordonnées dans la grille de la case avec la plus faible entropie, si la plus petite entropie c'est 1, alors les coordonnées retournées sont (-1,-1)
 */
pair<int, int> find_lowest_entropy(const Wave_grid &grille)
{

    int minChoices = INT_MAX;

    pair<int, int> result = {-1, -1};

    for (size_t i = 0; i < grille.size(); i++)
    {
        for (size_t j = 0; j < grille[i].size(); j++)
        {

            int options = grille[i][j].size();

            if (options > 1 && options < minChoices)
            {
                minChoices = options;
                result = {i, j};
            }
        }
    }
    return result;
}

/**
 * Calcul de l'entropie avec propagation locale
 *
 * @param grille
 * @param dicoADJ Dictionnaire des tuiles adjacentes
 *
 * @return True si une des cases est vide
 */
void entropy(Wave_grid &grille, const dicoADJtiles dicoADJ)
{

    using Position = std::pair<int, int>;

    std::queue<Position> propagationQueue;
    std::set<Position> dansLaFile;

    int height = grille.size();
    int width = grille[0].size(); // on suppose grille rectangulaire

    // Initialisation : on ajoute toutes les cases avec plus d’une tuile possible
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (grille[i][j].size() > 1)
            {
                propagationQueue.push({i, j});
                dansLaFile.insert({i, j});
            }
        }
    }

    while (!propagationQueue.empty())
    {
        auto [i, j] = propagationQueue.front();
        propagationQueue.pop();
        dansLaFile.erase({i, j});

        std::set<int> tuilesASupprimer;

        for (int tuile : grille[i][j])
        {
            bool estCompatible = true;

            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if ((dx == 0 && dy == 0) ||
                        i + dx < 0 || i + dx >= height ||
                        j + dy < 0 || j + dy >= width)
                        continue;

                    bool found = false;

                    if (dicoADJ.at(tuile).count({dx, dy}))
                    {
                        for (int voisin : dicoADJ.at(tuile).at({dx, dy}))
                        {
                            if (grille[i + dx][j + dy].count(voisin))
                            {
                                found = true;
                                break;
                            }
                        }
                    }

                    if (!found)
                    {
                        estCompatible = false;
                        break;
                    }
                }
            }

            if (!estCompatible)
            {
                tuilesASupprimer.insert(tuile);
            }
        }

        if (!tuilesASupprimer.empty())
        {
            for (int t : tuilesASupprimer)
            {
                grille[i][j].erase(t);
            }

            // Propagation : on ajoute les voisins à la queue
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if ((dx == 0 && dy == 0) ||
                        i + dx < 0 || i + dx >= height ||
                        j + dy < 0 || j + dy >= width)
                        continue;

                    Position voisin = {i + dx, j + dy};
                    if (!dansLaFile.count(voisin))
                    {
                        propagationQueue.push(voisin);
                        dansLaFile.insert(voisin);
                    }
                }
            }
        }
    }
} // fin entropy

/**
 * Affichage de toutes les tuiles enregistrées
 *
 * @param tiles
 */
void print_tiles_list(vector<Tile> &tiles)
{
    int k = 0;

    for (Tile &tile : tiles)
    {

        cout << "Tile: " << k << endl;

        print_tile(tile);

        k++;

        cout << endl;
    }
}

/**
 * Affichage de toutes les tuiles enregistrées avec leur nombre
 *
 * @param tiles
 * @param num_tile
 */
void print_tiles_list(vector<Tile> &tiles, vector<int> &num_tile)
{
    int k = 0;

    for (Tile &tile : tiles)
    {

        cout << "Tile: " << k << " > " << num_tile.at(k) << endl;

        print_tile(tile);

        k++;

        cout << endl;
    }
}

/**
 * Affichage d'une tuile
 *
 * @param tile
 */
void print_tile(const Tile tile)
{

    for (int i = 0; i < TILE_SIZE; i++)
    {
        for (int j = 0; j < TILE_SIZE; j++)
        {
            cout << tile.at(i).at(j) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

/**
 * Affichage des voisines d'une tuile
 *
 * @param n Numéro de la tuile
 * @param dico Dictionnaire des voisines
 */
void print_dico(const int n, dicoADJtiles dico)
{

    cout << "Tuile " << n << ":" << endl;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            printf("[%d,%d] : ", x, y);

            for (auto a : dico[n][{x, y}])
            {
                cout << a << " ";
            }

            cout << endl;
        }
    }

} // fin print_dico

/**
 * Affichage d'un vector2D
 *
 * @param vec
 */
void print_vector2D(const vector2D vec)
{

    for (int i = 0; i < (int)vec.size(); i++)
    {
        for (int j = 0; j < (int)vec.at(i).size(); j++)
        {
            cout << vec.at(i).at(j) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

vector2D matToVector(const cv::Mat &mat)
{
    std::vector<std::vector<int>> vec(mat.rows, std::vector<int>(mat.cols));

    for (int i = 0; i < mat.rows; ++i)
        for (int j = 0; j < mat.cols; ++j)
            vec[i][j] = mat.at<int>(i, j);

    return vec;
}

cv::Mat vectorToMat(const std::vector<std::vector<int>> &vec)
{
    int rows = vec.size();
    int cols = vec[0].size();
    cv::Mat mat(rows, cols, CV_32SC1);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            mat.at<int>(i, j) = vec[i][j];

    return mat;
}

cv::Mat intMatrixToImage(const std::vector<std::vector<int>> &intMatrix)
{
    int rows = intMatrix.size();
    int cols = intMatrix[0].size();
    cv::Mat img(rows, cols, CV_8UC3); // image RGB

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            int value = intMatrix[i][j];
            uchar r = (value >> 16) & 0xFF;
            uchar g = (value >> 8) & 0xFF;
            uchar b = value & 0xFF;

            // uchar r = value * 255;
            // uchar g = value * 255;
            // uchar b = value * 255;

            img.at<cv::Vec3b>(i, j) = cv::Vec3b(b, g, r); // OpenCV: BGR
        }
    }

    return img;
}

int rgbToInt(int r, int g, int b)
{
    return (r << 16) | (g << 8) | b;
}