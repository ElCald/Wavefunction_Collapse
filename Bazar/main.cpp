#include "wavefunction.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{

    srand(time(nullptr));

    // Liste des tuiles de la grille

    vector<Tile> list_tile; // Liste de tuiles
    vector<int> num_tile;   // Nombre de tuile à l'indice de celle-ci dans la liste

    dicoADJtiles dicoADJ;

    // Chargement de l'image sample

    cv::Mat img = cv::imread("template1.png", cv::IMREAD_COLOR);

    // Convertir RGB -> intMatrix (cv::Mat)
    cv::Mat intMat(img.rows, img.cols, CV_32SC1);
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
        {
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
    // print_dico(i, dicoADJ);
    //}

    // Initialisation de la grille
    // Wave_grid grille(GRILLE_SIZE_HEIGHT, vector<set<int>>(GRILLE_SIZE_WIDTH));

    // for (size_t i = 0; i < grille.size(); i++){
    //     for (size_t j = 0; j < grille.at(i).size(); j++){
    //         for (size_t k = 0; k < list_tile.size(); k++){
    //             grille[i][j].insert(k);
    //         }
    //     }
    // }

    int nb_tour = 0;

    int nb_tasks = (GRILLE_SIZE_HEIGHT * GRILLE_SIZE_WIDTH * (int)list_tile.size());

    cout << "Taille grille: " << GRILLE_SIZE_HEIGHT << "x" << GRILLE_SIZE_WIDTH << " = " << (GRILLE_SIZE_HEIGHT * GRILLE_SIZE_WIDTH) << endl;
    cout << "Nb tuiles: " << list_tile.size() << endl;
    cout << "Nb tasks: " << nb_tasks << endl;

    /*
    On parallelise sous forme de tâches.
    1 tache correspond à un case avec une tuile de départ
    */

    // // Génération de la map
    // while (true) {
    //     nb_tour++;
    //     // grille.clear();

    //     // On cherche dans la grille, la case avec la plus petite entropie
    //     auto [x, y] = find_lowest_entropy(grille);

    //     // Dans le cas où toutes les cases on une tuile adapté, alors la plus petite entropie sera 1, donc on s'arrête
    //     if (x == -1) break;

    //     // Si c'est le 1er tour ou que notre grille n'est pas parfaite

    //     // On choisi aléatoirement dans la grille une case pour lui fixer une tuile parmis toutes celles qu'il pourrait avoir (soit toutes les tuiles enfaite)
    //     vector<int> options(grille.at(x).at(y).begin(), grille.at(x).at(y).end());

    //     // Choix aléatoire d'une des tuiles
    //     int choice = options[rand() % options.size()];
    //     grille.at(x).at(y) = {choice}; // tuile fixé

    //     // Calcul de la map
    //     entropy(grille, dicoADJ);
    // }

    bool trouve = false;
    int start_i = -1, start_j = -1, start_k = -1;

    Wave_grid base(GRILLE_SIZE_HEIGHT, vector<set<int>>(GRILLE_SIZE_WIDTH));
    for (int a = 0; a < GRILLE_SIZE_HEIGHT; a++)
    {
        for (int b = 0; b < GRILLE_SIZE_WIDTH; b++)
        {
            for (int c = 0; c < (int)list_tile.size(); c++)
            {
                base[a][b].insert(c);
            }
        }
    }

    vector2D image(IMAGE_HEIGHT, vector<int>(IMAGE_WIDTH));

    Wave_grid grille;

    double t_avant = omp_get_wtime();

#pragma omp parallel shared(trouve, start_i, start_j, start_k, nb_tour, base, image) firstprivate(list_tile, dicoADJ)
    {
#pragma omp for collapse(3) schedule(dynamic)
        for (int i = 0; i < (int)GRILLE_SIZE_HEIGHT; i++)
        {
            for (int j = 0; j < (int)GRILLE_SIZE_WIDTH; j++)
            {
                for (int k = 0; k < (int)list_tile.size(); k++)
                {
                    if (trouve)
                        continue;

                    grille = base; // copie indépendante de la grille
                    grille[i][j].clear();
                    grille[i][j].insert(k);

                    entropy(grille, dicoADJ);

                    if (propagate(grille, dicoADJ))
                    {
                        // La grille est complètement résolue, on peut construire l’image
                        vector2D local_image(IMAGE_HEIGHT, vector<int>(IMAGE_WIDTH));
                        write_image_from_grille(grille, local_image, list_tile);

#pragma omp critical
                        {
                            if (!trouve)
                            {
                                trouve = true;
                                start_i = i;
                                start_j = j;
                                start_k = k;
                                image = local_image; // écriture protégée
                            }
                            else
                            {
#pragma omp atomic
                                nb_tour++;
                            }
                        }
                    }
                }
            }
        }
    }

    double t_apres = omp_get_wtime();

    for (int i = 0; i < (int)grille.size(); i++)
    {
        for (int j = 0; j < (int)grille.at(i).size(); j++)
        {

            int id = *grille[i][j].begin();
            cout << id << " ";

            // Recréer une image couleur à partir de la matrice d'entiers
            cv::Mat reconstructedImage = intMatrixToImage(list_tile.at(id));

            // Sauvegarde
            cv::imwrite("reconstructed" + std::to_string(id) + ".png", reconstructedImage);
        }
        cout << endl;
    }

    cout << "Trouvé: " << trouve << endl;
    printf("start i: %d, j: %d, k: %d\n", start_i, start_j, start_k);
    cout << "nb tour: " << nb_tour << endl;
    printf("Algo resolu en : %f seconds\n", t_apres - t_avant);

    // print_vector2D(image);

    // Recréer une image couleur à partir de la matrice d'entiers
    cv::Mat reconstructedImage = intMatrixToImage(image);

    // Sauvegarde
    cv::imwrite("reconstructed.png", reconstructedImage);

    return EXIT_SUCCESS;
}