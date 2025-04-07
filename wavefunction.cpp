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
 
 #include <omp.h>


using namespace std;

#define TILE_SIZE 2 // Taille des tuiles
#define SAMPLE_SIZE 6 // Taille de la grille
#define PROBLEM_SIZE 16 // Taille de la grille


/**
 * - Enregistrer une liste de tuile de taille N depuis une grille de taille S
 *      > Savoir si une tuile est déjà dans la liste
 */



/**
 * @param tiles Liste de tuiles
 * @param t Tuile a tester
 * 
 * @return L'indice de la position de la tuile dans la liste sinon -1
 */
int tile_is_in_list(vector<array<array<int, TILE_SIZE>, TILE_SIZE>>& tiles, array<array<int, TILE_SIZE>, TILE_SIZE>& t){

    bool res = false;
    int i=0;

    for(array<array<int, TILE_SIZE>, TILE_SIZE>& tile : tiles) {

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
 * @param tiles Liste qui contiendra les tuiles de la grille
 * @param grid_sample Grille de 
 */
void save_tiles_from_grid_sample(vector<array<array<int, TILE_SIZE>, TILE_SIZE>>& tiles, vector<int>& num_tile, array<array<int, SAMPLE_SIZE>, SAMPLE_SIZE>& grid_sample) {
    int indice_tile;

    // Parcours de la grille
    for(int i=0; i<SAMPLE_SIZE-TILE_SIZE+1; i++) {
        for(int j=0; j<SAMPLE_SIZE-TILE_SIZE+1; j++) {

            array<array<int, TILE_SIZE>, TILE_SIZE> tile;
            for(array<int, TILE_SIZE>& t : tile){
                t.fill(0);
            }

            for(int k=0; k<TILE_SIZE; k++) {
                for(int l=0; l<TILE_SIZE; l++) {
                    tile.at(k).at(l) = grid_sample.at(i+k).at(j+l);
                }
            }

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
 * Trouve les tuiles overlape d'une tuile dans la grille selon un offset (x,y).
 * 
 * @param over_tiles Liste des tuiles cherchée. Ne pas oublier de reset() avant remplissage
 * @param main_tile Tuile à chercher
 * @param x offset
 * @param y offset
 * @param grid_sample Grille
 * 
 * @return True si au moins une tuile est trouvée
 */
bool find_overlapping_tiles(vector<array<array<int, TILE_SIZE>, TILE_SIZE>>& over_tiles, array<array<int, TILE_SIZE>, TILE_SIZE>& main_tile, int x, int y, array<array<int, SAMPLE_SIZE>, SAMPLE_SIZE>& grid_sample){
    

    for(int i=0; i<SAMPLE_SIZE-TILE_SIZE+1; i++) {
        for(int j=0; j<SAMPLE_SIZE-TILE_SIZE+1; j++) {

            array<array<int, TILE_SIZE>, TILE_SIZE> tile;
            for(array<int, TILE_SIZE>& t : tile){
                t.fill(0);
            }

            for(int k=0; k<TILE_SIZE; k++) {
                for(int l=0; l<TILE_SIZE; l++) {
                    tile.at(k).at(l) = grid_sample.at(i+k).at(j+l);
                }
            }

            // Lorsqu'on trouve la tuile cherchée dans la grille
            if(main_tile == tile){

                // Sécurité pour ne pas sortir de la grille
                if(i+x >= 0 && j+y >= 0 && i+x < SAMPLE_SIZE && j+y < SAMPLE_SIZE){ 

                    array<array<int, TILE_SIZE>, TILE_SIZE> temp_tile;

                    for(int k=i+x; k<i+x+TILE_SIZE; k++) {
                        for(int l=j+y; l<j+y+TILE_SIZE; l++) {
                            temp_tile.at(k-i-x).at(l-j-y) = grid_sample.at(k).at(l);
                        }
                    }

                    if(tile_is_in_list(over_tiles, temp_tile) == -1){
                        over_tiles.push_back(temp_tile);
                    }
                    

                }

            }

        }
    }

    return !over_tiles.empty();
}


/**
 * Calcul l'entropie de la grille contenant le problème
 * 
 * @param grid_problem
 * 
 */
void entropy_problem(array<array<int, PROBLEM_SIZE>, PROBLEM_SIZE>& grid_problem){

    array<array<vector<int>, PROBLEM_SIZE>, PROBLEM_SIZE> grid_entropy;

    stack<array<array<int, TILE_SIZE>, TILE_SIZE>> stack_tile; // pile qui contient les tuiles qui contiennent le pixel (i,j)
    

    for(int i=0; i<PROBLEM_SIZE; i++) {
        for(int j=0; j<PROBLEM_SIZE; j++) {
            




        }
    }

}




/**
 * Affichage de toutes les tuiles enregistrées
 * 
 * @param tiles
 */
void print_tiles_list(vector<array<array<int, TILE_SIZE>, TILE_SIZE>>& tiles){
    int k=0;

    for(array<array<int, TILE_SIZE>, TILE_SIZE>& tile : tiles) {

        cout << "Tile: " << k << endl;

        for(int i=0; i<TILE_SIZE; i++) {
            for(int j=0; j<TILE_SIZE; j++) {
                cout << tile.at(i).at(j) << " ";
            }
            cout << endl;
        }

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
void print_tiles_list(vector<array<array<int, TILE_SIZE>, TILE_SIZE>>& tiles, vector<int>& num_tile){
    int k=0;

    for(array<array<int, TILE_SIZE>, TILE_SIZE>& tile : tiles) {

        cout << "Tile: " << k << " > " << num_tile.at(k) << endl;

        for(int i=0; i<TILE_SIZE; i++) {
            for(int j=0; j<TILE_SIZE; j++) {
                cout << tile.at(i).at(j) << " ";
            }
            cout << endl;
        }

        k++;

        cout << endl;
    }
}



int main(int argc, char* argv[]){

    // Liste des tuiles de la grille
    vector<array<array<int, TILE_SIZE>, TILE_SIZE>> list_tile; // Liste de tuiles
    vector<int> num_tile; // Nombre de tuile à l'indice de celle-ci dans la liste

    vector<array<array<int, TILE_SIZE>, TILE_SIZE>> overlap_tile;

    // Grille sample
    array<array<int, SAMPLE_SIZE>, SAMPLE_SIZE> grid_sample {{
        {0, 1, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0}
    }};


    // Grille problem
    array<array<int, PROBLEM_SIZE>, PROBLEM_SIZE> grid_problem {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    }};


    save_tiles_from_grid_sample(list_tile, num_tile, grid_sample);

    print_tiles_list(list_tile, num_tile);

    overlap_tile.clear();

    if(find_overlapping_tiles(overlap_tile, list_tile.at(0), 1, 1, grid_sample)){
        print_tiles_list(overlap_tile);
    }
        

    


    return EXIT_SUCCESS;
}