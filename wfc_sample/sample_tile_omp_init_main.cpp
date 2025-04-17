/**
 * Version parallèle avec tâches
 * WFC avec génération de tuiles à partir d'une grille sample 
 * On lance plusieurs threads qui vont résoudre une grille chacun, le 1er arrivé termine le programme
 * Chaque thread commence avec une grille pré-initialisée
 */

#include "wave_utils.h"
#include <fstream>
#include <omp.h>

using namespace std;

int main(int argc, char *argv[])
{

    vector<vector<int>> sample_grid = {
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 0, 0},
    };



    int tileSize = 3;

    vector<vector<vector<int>>> liste_tuiles;
    vector<vector<int>> liste_edges;
    vector<int> num_tile;

    save_tiles_from_grid_sample(liste_tuiles, num_tile, sample_grid, tileSize);
    // print_tiles_list(liste_tuiles, tileSize);


    // Chaque tuile a des arêtes représentées par 4 entiers : [top, right, bottom, left]
    generate_edges(liste_tuiles, liste_edges, tileSize);
    // print_edges(liste_edges);



    

    vector<Tile *> tiles = loadTiles(liste_tuiles.size(), liste_edges, tileSize);

    int canvasWidth = 21;
    int canvasHeight = 21;

    Image_int canvas(canvasHeight, vector<int>(canvasWidth));

    // Initialise la grille
    Grid grid(canvasWidth, canvasHeight, tileSize, tiles);

    
    
    int n = liste_tuiles.size(), d = 5;
    int rows = pow(n, d);
    int* data = generateTasks(n, d);


    
    random_device grd;
    mt19937 g(grd());

    // On randomise le tableau de tâche pour pouvoir faire des tâches différentes à chaque exécution
    std::shuffle(data, data+(n * d), g);
   

    
    double t_avant = omp_get_wtime(), t_apres;

    bool onContinue = true;
    string result;


    // intitialisation d'une grille pour chaque thread
    #pragma omp parallel
    {
        // Juste pour test visuel : randomize chaque cellule
        random_device rd;
        mt19937 rng(rd());

        Grid grid2(canvasWidth, canvasHeight, tileSize, tiles); // grid préchargée

        int t = omp_get_thread_num();
        t = (t>rows) ? 0 : t;


        for (int k = 0; k < d; ++k) {

            auto [row, col] = damier_coords(k, grid2.cols);
    
            grid2.cells.at(row).at(col).options.clear();
            grid2.cells.at(row).at(col).options.push_back(tiles.at(data[t]));
            grid2.cells.at(row).at(col).collapsed = true;

        }


        do{
            // init grid
            Grid grid3(canvasWidth, canvasHeight, tileSize, tiles); // grid préchargée

            grid3 = grid2;
    
    
            // Algorithme WFC principal
            for (int k = 0; k < grid3.cols * grid3.rows; ++k)
            {
                grid3.collapse(rng);
            }
    
            if(grid3.is_ready()){
                #pragma omp critical
                {
                    if(onContinue){
                        grid = grid3;

           
                        onContinue = false;

                        t_apres = omp_get_wtime();
                        cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec, from: " << omp_get_thread_num() << endl;

                        exit(EXIT_SUCCESS);
                    }
                }
               
            }
            
            // cout << i++ << endl;
        }while(onContinue);


    
    }



    
    double t_total = omp_get_wtime();

    
    delete[] data;


    // Dessine et sauvegarde
    result = "output" + to_string(0) + ".png";  
    // grid.draw(canvas);
    // imwrite(result, canvas);

    cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec." << endl;
    cout << "Temps total : " << t_total - t_avant << "sec." << endl;

    return 0;
}