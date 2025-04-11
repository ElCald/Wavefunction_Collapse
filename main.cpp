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




    // Chargement de l'image sample

    cv::Mat img = cv::imread("template1.png", cv::IMREAD_COLOR);

    // Convertir RGB -> intMatrix (cv::Mat)
    cv::Mat intMat(img.rows, img.cols, CV_32SC1);
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j) {
            cv::Vec3b px = img.at<cv::Vec3b>(i, j);
            intMat.at<int>(i, j) = (px[2] << 16) | (px[1] << 8) | px[0];
        }

    // Matrice vers vector<vector<int>>
    auto grid_sample = matToVector(intMat);






    

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


    // Affichage de la grille finale (coin haut gauche de chaque tuile)
    int k=0, l=0;
   Image image(GRILLE_SIZE_HEIGHT, vector<int>(GRILLE_SIZE_WIDTH));

    for (size_t i = 0; i < grille.size(); i+=TILE_SIZE) {
        for (size_t x = 0; x < TILE_SIZE; x++) {
            for (size_t j = 0; j < grille[i].size(); j+=TILE_SIZE) {

                int id = *grille[i][j].begin();

                for (size_t y = 0; y < TILE_SIZE; y++) {
                    // cout << list_tile[id][x][y] << " ";
                    image.at(k).at(l) = list_tile[id][x][y];
                    l = (l<GRILLE_SIZE_WIDTH-1)?l+1:0;
                }

            }
            // cout << endl;
            k = (k<GRILLE_SIZE_HEIGHT-1)?k+1:0;
        }
    }



    // Vector vers Mat
    cv::Mat recoveredMat = vectorToMat(image);

    // Recréer une image couleur à partir de la matrice d'entiers
    cv::Mat reconstructedImage = intMatrixToImage(image);

    // Sauvegarde
    cv::imwrite("reconstructed.png", reconstructedImage);



    return EXIT_SUCCESS;
}