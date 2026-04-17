
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "production.h"
#include <signal.h>

// Gestore del segnale SIGINT (CTRL+C)
void handle_sigint(int sig) {
    stop = 1; // Imposta il flag di uscita

    // IMPORTANTE: Risveglia tutti i thread bloccati sulle cond_wait
    // altrimenti i thread rimarrebbero sospesi e il join non avverrebbe mai.
    pthread_cond_broadcast(&notEmpty);
    pthread_cond_broadcast(&notFull);
    pthread_cond_broadcast(&validProduct);
}

int main(int argc, char* argv[]) {
    // Inizializzazione del seme per i numeri casuali (qualità)
    srand(time(NULL));

    pthread_t producer, qualityChecker, fileWriter;
    FILE* fp = fopen("magazzino.log", "w");
    if (fp == NULL) {
        perror("Errore apertura file");
        return 1;
    }
    
    // Intestazione del file CSV
    fprintf(fp, "ID,QUALITY\n");

    printf("Il programma è in esecuzione (CTRL+C per fermare)...\n");

    // Creazione dei thread
    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&qualityChecker, NULL, check, NULL);
    pthread_create(&fileWriter, NULL, fileUpdate, fp);

    // Registrazione del segnale di interruzione
    signal(SIGINT, handle_sigint);

    // Attesa terminazione dei thread (Stop pulito)
    pthread_join(producer, NULL);
    pthread_join(qualityChecker, NULL);
    pthread_join(fileWriter, NULL);

    // Pulizia finale delle risorse
    pthread_mutex_destroy(&lockBuffer);
    pthread_cond_destroy(&notFull);
    pthread_cond_destroy(&notEmpty);
    pthread_cond_destroy(&validProduct);

    printf("\nChiusura... File {magazzino.log} aggiornato correttamente.\n");
    fclose(fp);
    freeBuffer(); // Libera eventuale memoria residua
    
    return 0;
}
