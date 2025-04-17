/**
 * Version parallélisé avec tâches
 * WFC avec tuiles préchargées sous forme d'images
 */


#include "wave_utils.h"
#include <fstream>
#include <omp.h>
#include <random>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[])
{
    // Préparer les assets et leurs arêtes
    vector<string> paths = {
        "templates/0.png",
        "templates/1.png",
        "templates/2.png",
        "templates/3.png",
        "templates/4.png",
        // "templates/5.png",
        // "templates/6.png",
        // "templates/7.png",
        // "templates/8.png",
        // "templates/9.png",
        // "templates/10.png"
        // "templates/11.png",
        // "templates/12.png",
    };

    // Chaque tuile a des arêtes représentées par 4 entiers : [top, right, bottom, left]
    vector<vector<int>> edges = {
        {0, 0, 0, 0}, // 0
        {1, 1, 0, 1}, // 1
        {1, 1, 1, 0}, // 2
        {0, 1, 1, 1}, // 3
        {1, 0, 1, 1}, // 4
        // {1, 1, 0, 0}, // 5
        // {0, 1, 1, 0}, // 6
        // {0, 0, 1, 1}, // 7
        // {1, 0, 0, 1}, // 8
        // {1, 0, 1, 0}, // 9
        // {0, 1, 0, 1} // 10
        // {0, 2, 0, 1}, // 11
        // {0, 1, 0, 2}, // 12
    };

    int tileSize = 100;

    vector<Tile *> tiles = loadTiles(paths, edges, tileSize);

    int canvasWidth = 900;
    int canvasHeight = 800;
    Mat canvas(canvasHeight, canvasWidth, CV_8UC3, Scalar(0, 0, 0));

    // Initialise la grille
    Grid grid(canvasWidth, canvasHeight, tileSize, tiles);


    
    double t_avant = omp_get_wtime(), t_apres;


    int n = 5, d = 3;
    int rows = pow(n, d);
    int* data = generateTasks(rows, d);

    random_device grd;
    mt19937 g(grd());

    std::shuffle(data, data+(n * d), g);



    bool onContinue = true;
    #pragma omp parallel
    {
        // Juste pour test visuel : randomize chaque cellule
        random_device rd;
        mt19937 rng(rd());


        Grid grid2(canvasWidth, canvasHeight, tileSize, tiles);

        for (int k = 0; k < d; ++k) {

            auto [row, col] = damier_coords(k, grid2.cols);
    
            grid2.cells.at(row).at(col).options.clear();
            grid2.cells.at(row).at(col).options.push_back(tiles.at(data[omp_get_thread_num() * d + k]));
            grid2.cells.at(row).at(col).collapsed = true;

        }


        do{
            
            Grid grid3(canvasWidth, canvasHeight, tileSize, tiles);
    
            grid3 = grid2;

            // Algorithme WFC principal
            for (int i = 0; i < grid.cols * grid.rows; ++i)
            {
                grid3.collapse(rng);
            }

            if(grid3.is_ready()){
                #pragma omp critical
                {
                    if(onContinue){
                        onContinue = false;
                        grid = grid3;

                        // Dessine et sauvegarde
                        grid.draw(canvas);
                        imwrite("output.png", canvas);

                        t_apres = omp_get_wtime();
                        cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec from: " << omp_get_thread_num() << endl;

                        exit(EXIT_SUCCESS);
                    }
                }
            }

        }while(onContinue);
        
        
    }


    delete[] data;


    return 0;
}