#include "wave_utils.h"
#include <fstream>
#include <omp.h>

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
    int canvasHeight = 900;
    Mat canvas(canvasHeight, canvasWidth, CV_8UC3, Scalar(0, 0, 0));

    // Initialise la grille
    Grid grid(canvasWidth, canvasHeight, tileSize, tiles);

    // Juste pour test visuel : randomize chaque cellule
    random_device rd;
    mt19937 rng(rd());

    
    double t_avant = omp_get_wtime();

    bool onContinue = true;
    do{
        Grid grid2(canvasWidth, canvasHeight, tileSize, tiles);

        // Algorithme WFC principal
        for (int i = 0; i < grid.cols * grid.rows; ++i)
        {
            grid2.collapse(rng);
        }

        if(grid2.is_ready()){
            onContinue = false;
            grid = grid2;
        }

    }while(onContinue);

    double t_apres = omp_get_wtime();

    // for(int i=0; i<grid.rows; i++){
    //     for(int j=0; j<grid.cols; j++){
    //         if(grid.getCell(i,j)->collapsed){
    //             cout << grid.getCell(j,i)->options.at(0)->index << " ";
    //         }
    //         else{
    //             cout << "A "; 
    //         }
    //     }
    //     cout << endl;
    // }



    // Dessine et sauvegarde
    grid.draw(canvas);
    imwrite("output.png", canvas);

    cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec." << endl;

    return 0;
}