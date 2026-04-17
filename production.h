#ifndef PRODUCT_H
#define PRODUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define DIM_BUFFER 10

typedef struct {
    int id;
    int quality;
} Product;

// Flag atomico per gestire la terminazione sicura di tutti i thread
extern volatile sig_atomic_t stop;

// --- Variabili di Sincronizzazione ---
extern int countProduct;
extern pthread_mutex_t lockBuffer;    // Mutex per l'accesso esclusivo al buffer e alle variabili condivise
extern pthread_cond_t notEmpty;       // Segnala al Thread 2 che c'è almeno un pezzo nel buffer
extern pthread_cond_t notFull;        // Segnala al Thread 1 che c'è spazio nel buffer
extern pthread_cond_t validProduct;   // Segnala al Thread 3 che un prodotto ha superato il controllo qualità

// --- Stato del Buffer ---
extern Product* dataLog;              // Puntatore temporaneo per passare il prodotto al thread di log
extern Product* buffer[DIM_BUFFER];   // Buffer circolare di puntatori a Product
extern int in;                        // Indice di inserimento
extern int out;                       // Indice di estrazione
extern int count;                     // Numero attuale di elementi nel buffer

// --- Thread Functions ---
// Thread 1: Generatore
Product* createProduct(); 
void addProduct();
void* produce(void* args);

// Thread 2: Controllo Qualità
Product* qualityCheck();
Product* dupProduct(Product* product);
void* check(void* args);

// Thread 3: Scrittura Log
void productToLog(FILE* fp,Product* product);
void* fileUpdate(void* args);

// --- Cleanup ---
void freeProduct(Product* product);
void freeBuffer();

#endif