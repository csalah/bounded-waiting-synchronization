#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define NB_CHAISES 3

// Variables principales
// nombre de chaises actuellement occupées
int nb_chaises_occupees = -1;   

int indice_chaise_suivante = 0;
int compteur_aides = 0;
int NB_MAX_AIDES;

// les threads

pthread_t *taches_etudiants; 
pthread_t tache_aide;

// les sémaphores 

pthread_mutex_t verrou_chaises;
sem_t notif_aide;
sem_t notif_etudiant;
sem_t tab_chaises[NB_CHAISES];
sem_t verif_compteur;
sem_t maj_compteur;


void *execution_etudiant(void *id) {
        
    while (1) {
        printf("\n>> L'étudiant %ld programme ...\n", (long) id);
        
        // simule le temps d'aide avec un étudiant
                sleep(rand() % 10 + 1);

        // verrouillage pour vérifier les chaises disponibles
            pthread_mutex_lock(&verrou_chaises);
        int nb_occup = nb_chaises_occupees;
        pthread_mutex_unlock(&verrou_chaises);

        if (nb_occup < NB_CHAISES) {

            printf("\n>> Étudiant %ld cherche de l'aide\n", (long) id);

            pthread_mutex_lock(&verrou_chaises);
            int chaise_actuelle = (indice_chaise_suivante + nb_chaises_occupees) % NB_CHAISES;
            nb_chaises_occupees = (nb_chaises_occupees >= -1 && nb_chaises_occupees < NB_CHAISES) ? nb_chaises_occupees + 1 : NB_CHAISES;
            
            printf(">> %d chaise restantes\n", NB_CHAISES - nb_chaises_occupees);
            pthread_mutex_unlock(&verrou_chaises);

            if (nb_occup > 0) {
                printf("Étudiant %ld attend sur une chaise\n", (long) id);
                sem_wait(&tab_chaises[chaise_actuelle]);
                sem_wait(&notif_etudiant);
                printf("✓ Aide reçue par l'étudiant %ld / retour au travail\n", (long) id);
                sem_post(&verif_compteur);
                sem_wait(&maj_compteur);
            } else {
                sem_post(&notif_aide);
                sem_wait(&notif_etudiant);
                printf("✓ Aide reçue par l'étudiant %ld. Reprend du travail\n", (long) id);
                sem_post(&verif_compteur);
                sem_wait(&maj_compteur);
            }
        } else {
            printf("!!!!!! Aucune chaise disponible pour l'étudiant %ld. Retour à la programmation\n", (long) id);
        }
    }
}


void *execution_aide() {
        
    // boucle principale de l'aide, attend les demandes des étudiants
    while (1) {
        sem_wait(&notif_aide);

        puts("\n** L'aide s'est réveillé **");

        while (1) {

            pthread_mutex_lock(&verrou_chaises);

            if (nb_chaises_occupees == 0) {
                puts("Aucun étudiant en attente \n");
                puts("L'aide assiste directement celui qui l'a réveillé \n");

                sleep(rand() % 5 + 1);

                sem_post(&notif_etudiant);

                sem_wait(&verif_compteur);

                compteur_aides++;
                printf("########### Aides effectuées : %d ###############\n", compteur_aides);

                if (compteur_aides == NB_MAX_AIDES) {
                    puts("*** Fin de la session d'aide ***");
                    exit(0);
                }

                sem_post(&maj_compteur);


                pthread_mutex_unlock(&verrou_chaises);
                break;
            } else {

                sem_post(&tab_chaises[indice_chaise_suivante]);
                nb_chaises_occupees = nb_chaises_occupees > -1 ? nb_chaises_occupees - 1 : -1;

                printf("%d chaise libres \n", NB_CHAISES - nb_chaises_occupees);
                indice_chaise_suivante = (indice_chaise_suivante + 1) % NB_CHAISES;

                pthread_mutex_unlock(&verrou_chaises);

                puts("L'aide travaille avec un étudiant...");

                sleep(rand() % 10 + 1);

                sem_post(&notif_etudiant);

                pthread_mutex_lock(&verrou_chaises);

                sem_wait(&verif_compteur);

                compteur_aides++;
                printf("Aides total : %d \n", compteur_aides);

                if (compteur_aides == NB_MAX_AIDES) {
                    puts("ZzzzzzzzZZZZZZZZZzzzzz L'aide se rendort ");
                    puts("*** Fin de la session d'aide ***");
                    exit(0);
                }

                sem_post(&maj_compteur);

                pthread_mutex_unlock(&verrou_chaises);
            }
        }
    }
}


int main() {

    int total_etudiants = 6;
    
    NB_MAX_AIDES = 12;

    char buffer[16];

    printf("Entrez le nombre d'étudiants: ");
    if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n') {
        sscanf(buffer, "%d", &total_etudiants);
    }

    printf("Entrez le nombre d'aides du TA maximum: ");
    if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n') 
    {
        sscanf(buffer, "%d", &NB_MAX_AIDES);
    }

printf("Nombre d'étudiants : %d | L'aide arrêtera après %d sessions\n", total_etudiants, NB_MAX_AIDES);
    sem_init(&notif_aide, 0, 0);
    sem_init(&maj_compteur, 0, 0);

    sem_init(&notif_etudiant, 0, 0);
  
    sem_init(&verif_compteur, 0, 0);

    for (int i = 0; i < NB_CHAISES; i++) 
    {
        sem_init(&tab_chaises[i], 0, 0);
    }

    if (pthread_mutex_init(&verrou_chaises, NULL) != 0) 
    {
        puts("Erreur: probleme avec le mutex");
        return 1;
    }

    srand(time(NULL));

    // allocation mémoire pour les threads des étudiants
    taches_etudiants = malloc(total_etudiants * sizeof(pthread_t));

    // création du thread pour l'aide
    pthread_create(&tache_aide, NULL, execution_aide, NULL);




    // création des threads pour les étudiants
    for (int i = 0; i < total_etudiants; i++) {
        pthread_create(&taches_etudiants[i], NULL, execution_etudiant, (void*)(long)i);
    }


    pthread_join(tache_aide, NULL);



    for (int i = 0; i < total_etudiants; i++) {
        pthread_join(taches_etudiants[i], NULL);
    }

    // fin  du programme
    return 0;
}
