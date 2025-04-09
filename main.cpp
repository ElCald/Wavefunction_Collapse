#include "wavefunction.h"

int main(int argc, char* argv[]){

    srand(time(nullptr));

    // Liste des tuiles de la grille

    vector<Tile> list_tile; // Liste de tuiles
    vector<int> num_tile; // Nombre de tuile à l'indice de celle-ci dans la liste

    dicoADJtiles dicoADJ;


    // Grille sample
    vector2D grid_sample {
        {0, 1, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0}
    };


    
    

    // Extraction des tuiles
    save_tiles_from_grid_sample(list_tile, num_tile, grid_sample);
    print_tiles_list(list_tile, num_tile);
    

    // Recherche de toutes les tuiles voisines
    dicoADJ = compute_adjacency(list_tile);


    // Initialisation de la grille
    Wave_grid grille(GRILLE_SIZE_HEIGHT, vector<set<int>>(GRILLE_SIZE_HEIGHT));

    for (size_t i = 0; i < grille.size(); i++){
        for (size_t j = 0; j < grille[i].size(); j++){
            for (size_t k = 0; k < list_tile.size(); k++){
                grille[i][j].insert(i);
            }
        }
    }
        

    // Génération de la map
    while (true) {
        // On cherche dans la grille, la case avec la plus petite entropie
        auto [x, y] = find_lowest_entropy(grille);

        // Dans le cas où toutes les cases on une tuile adapté, alors la plus petite entropie sera 1, donc on s'arrête
        if (x == -1) break;

        // Si c'est le 1er tour ou que notre grille n'est pas parfaite

        // On choisi aléatoirement dans la grille une case pour lui fixer une tuile parmis toutes celles qu'il pourrait avoir (soit toutes les tuiles enfaite)
        vector<int> options(grille[x][y].begin(), grille[x][y].end());

        // Choix aléatoire d'une des tuiles
        int choice = options[rand() % options.size()];
        grille[x][y] = {choice}; // tuile fixé

        // Calcul de la map
        entropy(grille, dicoADJ);
    }

    

    // Affichage de la grille finale (coin haut gauche de chaque tuile)
    for (size_t i = 0; i < grille.size(); i++) {
        for (size_t j = 0; j < grille[i].size(); j++) {
            int id = *grille[i][j].begin();
            cout << list_tile[id][0][0] << " "; // coin supérieur gauche
        }
        cout << "\n";
    }



    return EXIT_SUCCESS;
}