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
    };

    int tileSize = 100;

    vector<Tile *> tiles = loadTiles(paths, edges, tileSize);

    int canvasWidth = 900;
    int canvasHeight = 800;
    Mat canvas(canvasHeight, canvasWidth, CV_8UC3, Scalar(0, 0, 0));

    // Initialise la grille
    Grid grid(canvasWidth, canvasHeight, tileSize, tiles);

    // Juste pour test visuel : randomize chaque cellule
    random_device rd;
    mt19937 rng(rd());


    // for (int i = 0; i < grid.cols * grid.rows; ++i)
    // {
    //     grid.collapse(rng);
    // }

    
    double t_avant = omp_get_wtime(), t_apres;


    bool onContinue = true;
    string result;

    int n = 5, d = 5;
    int rows = pow(n, d);
    int* data = generateTasks(n, d);

    #pragma omp parallel 
    {

        #pragma omp for schedule(dynamic)
        for (int i = 0; i < rows; ++i) {

            // init grid
            Grid grid2(canvasWidth, canvasHeight, tileSize, tiles); // grid préchargée
            // Grid grid3(canvasWidth, canvasHeight, tileSize, tiles);

            for (int k = 0; k < d; ++k) {

                auto [row, col] = damier_coords(k, grid2.cols);
        
                grid2.cells.at(row).at(col).options.clear();
                grid2.cells.at(row).at(col).options.push_back(tiles.at(data[i * d + k]));
                grid2.cells.at(row).at(col).collapsed = true;

            }

            // lancer les calculs

            bool onContinue_local = true;

            do{

                Grid grid3(canvasWidth, canvasHeight, tileSize, tiles);
                grid3 = grid2;

                // Algorithme WFC principal
                for (int k = 0; k < grid2.cols * grid2.rows; ++k)
                {
                    grid3.collapse(rng);
                }
        
                if(grid3.is_ready()){
                    onContinue_local = false;

                    #pragma omp critical
                    {
                        if(onContinue){
                            grid = grid3;
                            cout << omp_get_thread_num() << " à terminé avec la tache : " << i << endl;
                    
                            t_apres = omp_get_wtime();

                            cout << "Image trouvée en : " << t_apres - t_avant << "sec." << endl;        

                            result = "output" + to_string(i) + ".png";  
                            grid.draw(canvas);
                            imwrite(result, canvas);        

                            onContinue = false;
                            exit(EXIT_SUCCESS);
                        }
                        
                    }
                }
            
            }while(onContinue_local);
        }
    }


    t_apres = omp_get_wtime();
    double t_total = omp_get_wtime();


    delete[] data;


    // Dessine et sauvegarde
    result = "output" + to_string(0) + ".png";  
    grid.draw(canvas);
    imwrite(result, canvas);

    cout << "Image sauvegardée dans output.png en : " << t_apres - t_avant << "sec." << endl;
    cout << "Temps total : " << t_total - t_avant << "sec." << endl;

    return 0;
}