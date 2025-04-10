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
int tile_is_in_list(vector<Tile>& tiles, Tile& t){

    bool res = false;
    int i=0;

    for(Tile tile : tiles) {

        if(tile == t) {
            res = true;
            break;
        }
        i++;
    }

    if(res)
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
void save_tiles_from_grid_sample(vector<Tile>& tiles, vector<int>& num_tile, vector2D& grid_sample) {
    int indice_tile;

    // Parcours de la grille
    for(size_t i=0; i<grid_sample.size()-TILE_SIZE+1; i++) {
        for(size_t j=0; j<grid_sample.at(i).size()-TILE_SIZE+1; j++) {

            Tile tile(TILE_SIZE, vector<int>(TILE_SIZE));

            for(int k=0; k<TILE_SIZE; k++) {
                for(int l=0; l<TILE_SIZE; l++) {
                    tile[k][l] = grid_sample.at(i+k).at(j+l);
                }
            }


            // On vérifie si la tuile existe déjà, ce qui nous permet de compter le nombre qu'on en trouve dans le sample
            if((indice_tile = tile_is_in_list(tiles, tile)) == -1){
                tiles.push_back(tile);
                num_tile.push_back(1);
            }
            else{
                num_tile.at(indice_tile) += 1;
            }

        }
    }

    /* À voir pour save les symétries et les rotations */

}


/**
 * Trouve si deux tuiles peuvent se chevaucher
 * 
 * @param a Tuile a
 * @param b Tuile b
 * @param x offset (positif = droite, négatif = gauche ou 0)
 * @param y offset (positif = bas, négatif = haut ou 0)
 * 
 * @return True si le chevauchement est possible
 */
bool tiles_can_overlap(const Tile &a, const Tile &b, int x, int y) {

    // for (int i = 0; i < TILE_SIZE; ++i) {
    //     for (int j = 0; j < TILE_SIZE; ++j) {

    //         int ax = i, ay = j; // Bord de la tuile "a"
    //         int bx = i - x, by = j - y; // Bord de la tuile "b"

    //         if (bx >= 0 && by >= 0 && bx < TILE_SIZE && by < TILE_SIZE){

    //             if (a[ay][ax] != b[by][bx]){ // On compare si les deux bords adjacents des tuiles sont égales
    //                 return false;
    //             }

    //         }
                
    //     }
    // }

    return true;
}


/**
 * Génère les voisins compatibles pour chaque tuile
 * 
 * @param tiles Liste de tuiles
 * 
 * @return Dictionnaire des tuiles adjacentes pour toutes les tuiles
*/ 
dicoADJtiles compute_adjacency(const vector<Tile> &tiles) {

    dicoADJtiles dico;

    for (size_t i = 0; i < tiles.size(); ++i) {
        for (size_t j = 0; j < tiles.size(); ++j) {

            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {

                    // if (abs(x) + abs(y) != 1) continue;
                    if (x == 0 && y ==0) continue;

                    // Si la tuile peut être chevauchée alors on save celle qui peut
                    if (tiles_can_overlap(tiles[i], tiles[j], x, y)) {
                        dico[i][{x, y}].insert(j); // Insertion à la tuile i et à l'offset (dx,dy) l'indice de la tuile adjacente j
                    }

                }
            }

        }
    }

    return dico;
}



/**
 * Sélectionne la case de plus faible entropie
 * 
 * @param grille
 * 
 * @return Les coordonnées dans la grille de la case avec la plus faible entropie, si la plus petite entropie c'est 1, alors les coordonnées retournées sont (-1,-1)
 */
pair<int, int> find_lowest_entropy(const Wave_grid &grille) {

    int minChoices = INT_MAX;

    pair<int, int> result = {-1, -1};

    for (size_t i = 0; i < grille.size(); i++) {
        for (size_t j = 0; j < grille[i].size(); j++) {

            int options = grille[i][j].size();

            if (options > 1 && options < minChoices) {
                minChoices = options;
                result = {i, j};
            }
        }
    }
    return result;
}





/**
 * Calcul de l'entropie de toutes les cases de la grilles
 * 
 * @param grille
 * @param dicoADJ Dictionnaire des tuiles adjacentes
 */
void entropy(Wave_grid &grille, dicoADJtiles& dicoADJ){

    bool miseAjour;


    do{
        miseAjour = false; 

        // Parcours de la grille
        for(size_t i=0; i<grille.size(); i++){
            for(size_t j=0; j<grille[i].size(); j++){

                if (grille[i][j].size() == 1) {
                    // Dans le cas où la case de la grille possède déjà 1 seule tuile on n'a pas besoin de l'a traiter donc on skip la prochaine boucle for
                }
                else{
                    // Parcours des tuiles possible pour une case (i,j) de la grille
                    for(int k : grille[i][j]){

                        bool estCompatible = true; // variable qui dit si la tuile k est compatible avec ses voisines dans la grille

                        // Pour une tuile k dans la case (i,j) on récupère tous ses voisins possibles
                        for (auto [offset, tiles_adj] : dicoADJ.at(k)) {
                            
                            size_t voisin_x = i + offset.first;
                            size_t voisin_y = j + offset.second;

                            if(voisin_x >= 0 && voisin_x < grille.size() && voisin_y >= 0 && voisin_y < grille[i].size()){ // On vérifie que le voisin n'est pas en dehors de la grille
                                // On récupère toutes les tuiles qui peuvent se trouver à la case du voisin dans la grille

                                set<int> liste_tile_voisin = grille[voisin_x][voisin_y];

                                bool found = false;

                                // On cherche si on trouve au moins une tuile qui se trouve dans le même espace que la voisine (offset) de la tuile "k"
                                for (int t : liste_tile_voisin) {
                                    if (dicoADJ.at(k).at(offset).count(t)) {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found) {
                                    estCompatible = false;
                                    break;
                                }

                            }

                            // Si c'est pas possible de placer cette tuile à la case (i,j) alors on l'a retire de la case
                            if (!estCompatible) {
                                grille[i][j].erase(k);
                                miseAjour = true;
                            }

                        } // for
                    } // for

                } // if la case ne possède pas juste 1 tuile de possible

            } // for grille j
        } // for grille i


    }while(miseAjour); // Si on met à jour une des cases de la grille, on recommence les calculs
} 





/**
 * Affichage de toutes les tuiles enregistrées
 * 
 * @param tiles
 */
void print_tiles_list(vector<Tile>& tiles){
    int k=0;

    for(Tile& tile : tiles) {

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
void print_tiles_list(vector<Tile>& tiles, vector<int>& num_tile){
    int k=0;

    for(Tile& tile : tiles) {

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
void print_tile(Tile tile){

    for(int i=0; i<TILE_SIZE; i++) {
        for(int j=0; j<TILE_SIZE; j++) {
            cout << tile.at(i).at(j) << " ";
        }
        cout << endl;
    }
    cout << endl;
}



/**
 * Créer une tuile qui a subit une rotation de 180
 * 
 * @param matrix Tuile d'origine
 * 
 * @return Tuile avec rotation
 */
Tile rotateTile(const Tile& matrix) {
    int rows = matrix.size();
    if (rows == 0) return {};
    int cols = matrix[0].size();

    Tile result(rows, vector<int>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[rows - 1 - i][cols - 1 - j] = matrix[i][j];
        }
    }

    return result;
}
