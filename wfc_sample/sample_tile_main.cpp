/**
 * Version séquentielle
 * WFC avec génération de tuiles à partir d'une grille sample 
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
    print_tiles_list(liste_tuiles, tileSize);


    // Chaque tuile a des arêtes représentées par 4 entiers : [top, right, bottom, left]
    generate_edges(liste_tuiles, liste_edges, tileSize);
    print_edges(liste_edges);



    

    vector<Tile *> tiles = loadTiles(liste_tuiles.size(), liste_edges, tileSize);

    int canvasWidth = 15;
    int canvasHeight = 15;
    // Mat canvas(canvasHeight, canvasWidth, CV_8UC3, Scalar(0, 0, 0));
    Image_int canvas(canvasHeight, vector<int>(canvasWidth));

    // Initialise la grille
    Grid grid(canvasWidth, canvasHeight, tileSize, tiles);

    // Juste pour test visuel : randomize chaque cellule
    random_device rd;
    mt19937 rng(rd());


    
    double t_avant = omp_get_wtime(), t_apres;

    bool onContinue = true;
    string result;


    do{
        
        // init grid
        Grid grid2(canvasWidth, canvasHeight, tileSize, tiles); // grid préchargée


        // Algorithme WFC principal
        for (int k = 0; k < grid2.cols * grid2.rows; ++k)
        {
            grid2.collapse(rng);
        }

        if(grid2.is_ready()){

            if(onContinue){
                grid = grid2;

                onContinue = false;
            }
            
        }
        
    }while(onContinue);


    t_apres = omp_get_wtime();
    double t_total = omp_get_wtime();

    // Affichage du numéro des tuiles dans la grille
    for(int i=0; i<grid.rows; i++){
        for(int j=0; j<grid.cols; j++){
            if(grid.getCell(j,i)->collapsed){
                cout << grid.getCell(j,i)->options.at(0)->index << " ";
            }
            else{
                cout << "A "; 
            }
        }
        cout << endl;
    }



    // Dessine et sauvegarde
    result = "output" + to_string(0) + ".png";  
    // grid.draw(canvas);
    // imwrite(result, canvas);

    cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec." << endl;
    cout << "Temps total : " << t_total - t_avant << "sec." << endl;

    return 0;
}