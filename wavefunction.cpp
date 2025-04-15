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
int tile_is_in_list(const vector<Tile> &tiles, const Tile &t)
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
 * Extraction des tuiles qui se trouvent dans le sample initiale en input
 *
 * @param tiles Liste qui contiendra les tuiles de la grille
 * @param grid_sample Grille de
 */
void save_tiles_from_grid_sample(vector<Tile> &tiles, vector<int> &num_tile, const vector2D &grid_sample)
{
    const size_t rows = grid_sample.size();
    const size_t cols = grid_sample[0].size();

    for (size_t i = 0; i <= rows - TILE_SIZE; ++i)
    {
        for (size_t j = 0; j <= cols - TILE_SIZE; ++j)
        {
            // Extraction d'une tuile TILE_SIZE x TILE_SIZE
            Tile tile(TILE_SIZE, vector<int>(TILE_SIZE));
            for (int k = 0; k < TILE_SIZE; ++k)
            {
                for (int l = 0; l < TILE_SIZE; ++l)
                {
                    tile[k][l] = grid_sample[i + k][j + l];
                }
            }

            // Vérifie si la tuile est déjà présente
            int indice_tile = tile_is_in_list(tiles, tile);
            if (indice_tile == -1)
            {
                tiles.push_back(std::move(tile)); // move utile ici
                num_tile.push_back(1);
            }
            else
            {
                ++num_tile[indice_tile];
            }
        }
    }

    // TODO: Ajouter symétries / rotations si besoin
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

                real_x = x;
                real_y = y;

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

    const size_t rows = grille.size();
    const size_t cols = grille[0].size();

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            size_t options = grille[i][j].size();

            if (options > 1 && static_cast<int>(options) < minChoices)
            {
                minChoices = static_cast<int>(options);
                result = {static_cast<int>(i), static_cast<int>(j)};
            }
        }
    }

    return result;
}

bool entropy(Wave_grid &grille, const dicoADJtiles dicoADJ)
{

    bool miseAjour;
    bool found = false;

    int real_x = 0;
    int real_y = 0;

    do
    {
        miseAjour = false;

        // Parcours de la grille
        for (int i = 0; i < (int)grille.size(); i++)
        {
            for (int j = 0; j < (int)grille.at(i).size(); j++)
            {

                if (grille.at(i).at(j).size() == 1)
                {
                    // Dans le cas où la case de la grille possède déjà 1 seule tuile on n'a pas besoin de l'a traiter donc on skip la prochaine boucle for
                    continue;
                }

                // Parcours des tuiles possible pour une case (i,j) de la grille
                for (int k : grille.at(i).at(j))
                {

                    bool estCompatible = true; // variable qui dit si la tuile k est compatible avec ses voisines dans la grille

                    for (int x = -1; x <= 1; x++)
                    {
                        for (int y = -1; y <= 1; y++)
                        {

                            // real_x = x*(TILE_SIZE-1);
                            // real_y = y*(TILE_SIZE-1);

                            real_x = x;
                            real_y = y;

                            if ((i + real_x < 0 || i + real_x >= (int)grille.size()) || (j + real_y < 0 || j + real_y >= (int)grille.size()) || (x == 0 && y == 0))
                            { // On vérifie si on sort de la grille
                                continue;
                            }

                            found = false;

                            // On cherche ici à savoir si à la case de la grille ([i+x],[j+y]), on trouve au moins un voisin de la tuile k qui se trouve en (i,j)
                            if (dicoADJ.at(k).count({x, y}))
                            {
                                for (int a : dicoADJ.at(k).at({x, y}))
                                {

                                    if (grille.at(i + real_x).at(j + real_y).count(a))
                                    {
                                        found = true;
                                    }
                                }
                            }

                            if (!found)
                            {
                                estCompatible = false;
                                // break;
                            }
                        }
                    }

                    // Si c'est pas possible de placer cette tuile à la case (i,j) alors on l'a retire de la case
                    if (!estCompatible)
                    {
                        // printf("i: %d, j: %d, k: %d\n", i, j, k);

                        if (grille.at(i).at(j).count(k))
                        {
                            grille.at(i).at(j).erase(k);
                        }

                        miseAjour = true;
                        break;
                    }

                } // for

            } // for grille j
        } // for grille i

    } while (miseAjour); // Renvoie vrai si au moins une case a été modifiée
    // Si on met à jour une des cases de la grille, on recommence les calculs

    return miseAjour;

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

// Reconstruction de l'image à partir d'une grille et stockage dans `image`
void write_image_from_grille(const Wave_grid &grille, vector2D &image, const vector<Tile> &list_tile)
{
    for (int i = 0; i < (int)grille.size(); i++)
    {
        for (int j = 0; j < (int)grille.at(i).size(); j++)
        {

            int id = *grille[i][j].begin();
            // cout << id << " ";

            for (int x = 0; x < TILE_SIZE; x++)
            {
                for (int y = 0; y < TILE_SIZE; y++)
                {
                    int img_x = (i * (TILE_SIZE - 1)) + x;
                    int img_y = (j * (TILE_SIZE - 1)) + y;

                    if (img_x >= 0 && img_x < (int)image.size() && img_y >= 0 && img_y < (int)image[0].size())
                    {
                        image[img_x][img_y] = list_tile[id][x][y];
                    }
                }
            }
        }

        // cout << endl;
    }

    /*for (int i = 0; i < (int)grille.size(); i++)
    {
        for (int j = 0; j < (int)grille.at(i).size(); j++)
        {
            int id = *grille[i][j].begin();
            print_tile(list_tile.at(id));
        }
    }*/
}

bool propagate(Wave_grid &grille, const dicoADJtiles &dicoADJ)
{
    bool changed = true;
    while (changed)
    {
        changed = entropy(grille, dicoADJ);
    }

    // Vérifie si la grille est complètement résolue (chaque case a une seule possibilité)
    for (const auto &row : grille)
    {
        for (const auto &cell : row)
        {
            if (cell.size() != 1)
                return false;
        }
    }

    return true; // propagation complète et grille résolue
}
