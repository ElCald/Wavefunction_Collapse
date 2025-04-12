#include "wavefunction.h"
#include <fstream>



void savePPM(const string &filename, const Image &image, int width, int height)
{
    ofstream file(filename);
    if (!file.is_open())
        return;

    file << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int color = image[y][x];
            color = (color == 1)?255:1;
            file << color << " " << color << " " << color << " ";
        }
        file << "\n";
    }
    file.close();
}




int main(int argc, char* argv[]){

    srand(time(nullptr));

    // Liste des tuiles de la grille

    vector<Tile> list_tile; // Liste de tuiles
    vector<int> num_tile; // Nombre de tuile à l'indice de celle-ci dans la liste

    dicoADJtiles dicoADJ;


    // Grille sample
    // vector2D grid_sample {
    //     {0, 0, 0, 0, 0, 0, 0, 0, 0},
    //     {0, 0, 0, 0, 1, 0, 0, 0, 0},
    //     {0, 0, 0, 1, 1, 1, 0, 0, 0},
    //     {0, 0, 1, 1, 0, 1, 1, 0, 0},
    //     {0, 1, 1, 0, 0, 0, 1, 1, 0},
    //     {0, 0, 1, 1, 0, 1, 1, 0, 0},
    //     {0, 0, 0, 1, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 0, 1, 0, 0, 0, 0},
    //     {0, 0, 0, 0, 0, 0, 0, 0, 0}
    // };

    // vector2D grid_sample {
    //     {0, 0, 0, 0, 0},
    //     {0, 0, 1, 0, 0},
    //     {0, 1, 0, 1, 0},
    //     {0, 0, 1, 0, 0},
    //     {0, 0, 0, 0, 0}
    // };

    // vector2D grid_sample {
    //     {0, 1, 0, 1},
    //     {1, 0, 1, 0},
    //     {0, 1, 0, 1},
    //     {1, 0, 1, 0}
    // };

    // vector2D grid_sample {
    //     {0, 2, 1, 2, 0, 0},
    //     {0, 2, 1, 2, 0, 0},
    //     {2, 2, 1, 2, 2, 2},
    //     {1, 1, 1, 1, 1, 2},
    //     {2, 2, 2, 2, 1, 2},
    //     {0, 0, 0, 2, 1, 2}
    // };

    // vector2D grid_sample {
    //     {1, 1, 1, 1, 0, 0, 0},
    //     {1, 2, 2, 1, 0, 0, 0},
    //     {1, 2, 2, 1, 0, 0, 0},
    //     {1, 1, 1, 1, 0, 0, 0},
    //     {0, 0, 0, 0, 0, 0, 0},
    //     {0, 0, 0, 0, 3, 3, 0},
    //     {0, 0, 0, 0, 3, 3, 0},
    // };


    // vector2D grid_sample {
    //     {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    //     {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    //     {1, 1, 1, 0, 0, 1, 1, 1, 1, 1},
    //     {0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    //     {0, 1, 1, 0, 0, 1, 1, 1, 0, 0},
    //     {0, 0, 1, 0, 0, 0, 1, 1, 0, 0},
    //     {1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
    //     {0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    //     {0, 1, 1, 1, 0, 0, 1, 0, 0, 0},
    //     {0, 0, 1, 0, 0, 0, 1, 0, 0, 0}
    // };




    // Chargement de l'image sample

    cv::Mat img = cv::imread("template2.png", cv::IMREAD_COLOR);

    // Convertir RGB -> intMatrix (cv::Mat)
    cv::Mat intMat(img.rows, img.cols, CV_32SC1);
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j) {
            cv::Vec3b px = img.at<cv::Vec3b>(i, j);
            intMat.at<int>(i, j) = (px[2] << 16) | (px[1] << 8) | px[0];
        }

    // Matrice vers vector<vector<int>>
    vector2D grid_sample = matToVector(intMat);


    

    // Extraction des tuiles
    save_tiles_from_grid_sample(list_tile, num_tile, grid_sample);
    // print_tiles_list(list_tile, num_tile);
    

    // Recherche de toutes les tuiles voisines
    dicoADJ = compute_adjacency(list_tile);

    // Affichage du dico
    // for(int i=0; i<(int)list_tile.size(); i++){
    //     print_dico(i, dicoADJ);
    // }

    




    // Initialisation de la grille
    Wave_grid grille(GRILLE_SIZE_HEIGHT, vector<set<int>>(GRILLE_SIZE_WIDTH));

    
    for (size_t i = 0; i < grille.size(); i++){
        for (size_t j = 0; j < grille.at(i).size(); j++){
            for (size_t k = 0; k < list_tile.size(); k++){
                grille[i][j].insert(k);
            }
        }
    }


    int nb_tour=0;

    /*
    On parallelise sous forme de tâches.
    1 tache correspond à un case avec une tuile de départ
    */

    // Génération de la map
    while (true) {
        nb_tour++;
        // grille.clear();

        // On cherche dans la grille, la case avec la plus petite entropie
        auto [x, y] = find_lowest_entropy(grille);

        // Dans le cas où toutes les cases on une tuile adapté, alors la plus petite entropie sera 1, donc on s'arrête
        if (x == -1) break;

        // Si c'est le 1er tour ou que notre grille n'est pas parfaite

        // On choisi aléatoirement dans la grille une case pour lui fixer une tuile parmis toutes celles qu'il pourrait avoir (soit toutes les tuiles enfaite)
        vector<int> options(grille.at(x).at(y).begin(), grille.at(x).at(y).end());

        // Choix aléatoire d'une des tuiles
        int choice = options[rand() % options.size()];
        grille.at(x).at(y) = {choice}; // tuile fixé

        // Calcul de la map
        entropy(grille, dicoADJ);
    }


    cout << "nb tour: " << nb_tour << endl;


    vector2D image(IMAGE_HEIGHT, vector<int>(IMAGE_WIDTH));

    // Affichage de la grille finale
    for(int i=0; i<(int)grille.size(); i++){
        for(int j=0; j<(int)grille.at(i).size(); j++){

            int id = *grille[i][j].begin();

            for(int x=0; x<TILE_SIZE; x++){
                for(int y=0; y<TILE_SIZE; y++){
                    if((i*(TILE_SIZE-1))+x >= 0 && (i*(TILE_SIZE-1))+x < (int)image.size() && (j*(TILE_SIZE-1))+y >= 0 && (j*(TILE_SIZE-1))+y < (int)image.size()){
                        image.at((i*(TILE_SIZE-1))+x).at((j*(TILE_SIZE-1))+y) = list_tile[id][x][y];
                    }
                }
            }
        }
    }

    // print_vector2D(image);



    // // Vector vers Mat
    cv::Mat recoveredMat = vectorToMat(image);

    // Recréer une image couleur à partir de la matrice d'entiers
    cv::Mat reconstructedImage = intMatrixToImage(image);

    // Sauvegarde
    cv::imwrite("reconstructed.png", reconstructedImage);



    return EXIT_SUCCESS;
}