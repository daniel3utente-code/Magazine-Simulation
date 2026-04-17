#include "production.h"

// Inizializzazione delle variabili globali e dei meccanismi di sincronizzazione
volatile sig_atomic_t stop = 0;
int countProduct = 0;
pthread_mutex_t lockBuffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t validProduct = PTHREAD_COND_INITIALIZER;

Product* dataLog = NULL;
Product* buffer[DIM_BUFFER];
int in = 0;
int out = 0;
int count = 0;

// ------------ THREAD 1: PRODUZIONE ------------

// Crea dinamicamente un prodotto con ID incrementale e qualità casuale
Product* createProduct() {
    Product* ret = (Product*)malloc(sizeof(Product));
    ret->id = countProduct++;
    ret->quality = rand() % 100;
    return ret;
}

// Inserisce il prodotto nel buffer circolare (chiamata sotto Mutex)
void addProduct() {
    buffer[in] = createProduct();
    in = (in + 1) % DIM_BUFFER;
    count++;
    // Sveglia il Thread 2: ora il buffer non è più vuoto
    pthread_cond_signal(&notEmpty);
}

void* produce(void* args) {
    while (!stop) {
        pthread_mutex_lock(&lockBuffer);

        // Attesa bloccante se il buffer è pieno (monitor)
        while (count == DIM_BUFFER && !stop) {
            pthread_cond_wait(&notFull, &lockBuffer);
        }

        if (stop) {
            pthread_mutex_unlock(&lockBuffer);
            break;
        }

        addProduct();
        pthread_mutex_unlock(&lockBuffer);

        printf("\n\t[PRODUTTORE] Pezzo creato!");
        sleep(2); // Specifica della consegna: ogni 2 secondi
    }
    return NULL;
}

// ------------ THREAD 2: CONTROLLO QUALITÀ ------------

// Estrae dal buffer e verifica la qualità
Product* qualityCheck() {
    Product* ret = NULL;
    // Se la qualità è > 50, duplichiamo il prodotto per passarlo al Thread 3
    if (buffer[out]->quality > 50) {
        ret = dupProduct(buffer[out]);
    }
    
    // Libera sempre la memoria del pezzo originale nel buffer
    freeProduct(buffer[out]);
    buffer[out] = NULL;
    count--;
    out = (out + 1) % DIM_BUFFER;
    
    // Sveglia il Thread 1: ora c'è spazio nel buffer
    pthread_cond_signal(&notFull);
    return ret;
}

Product* dupProduct(Product* product) {
    Product* ret = (Product*)malloc(sizeof(Product));
    ret->id = product->id;
    ret->quality = product->quality;
    return ret;
}

void* check(void* args) {
    while (!stop) {
        pthread_mutex_lock(&lockBuffer);

        // Attesa bloccante se il buffer è vuoto
        while(count == 0 && !stop) {
            pthread_cond_wait(&notEmpty, &lockBuffer);
        }

        if (stop) {
            pthread_mutex_unlock(&lockBuffer);
            break;
        }

        dataLog = qualityCheck();
        pthread_mutex_unlock(&lockBuffer);

        // Se il prodotto è valido, sveglia il Thread 3 (Log)
        if(dataLog != NULL) {
            pthread_cond_signal(&validProduct);
        }
    }
    return NULL;
}

// ------------ THREAD 3: MAGAZZINO (LOG) ------------

void productToLog(FILE* fp, Product* product) {
    fprintf(fp, "%d,%d\n", product->id, product->quality);
    fflush(fp); // Assicura la scrittura immediata su disco
    freeProduct(product);  
    dataLog = NULL; // Reset della variabile di scambio
}

void* fileUpdate(void* args) {
    FILE* fp = (FILE*)args;
    while (!stop) {
        pthread_mutex_lock(&lockBuffer);

        // Attesa finché non c'è un prodotto valido da loggare
        while(dataLog == NULL && !stop) {
            pthread_cond_wait(&validProduct, &lockBuffer);
        }

        if (stop) {
            pthread_mutex_unlock(&lockBuffer);
            break;
        }

        productToLog(fp, dataLog);
        pthread_mutex_unlock(&lockBuffer);
    }
    return NULL;
}

// ------------ CLEANUP ------------

void freeProduct(Product* product) {
    if(product) free(product);
}

// Svuota i residui nel buffer e in dataLog alla chiusura del programma
void freeBuffer() {
    pthread_mutex_lock(&lockBuffer);
    for(int i = 0; i < DIM_BUFFER; i++) {
        if(buffer[i] != NULL) {
            freeProduct(buffer[i]);
            buffer[i] = NULL;
        }
    }
    if(dataLog != NULL) freeProduct(dataLog);
    dataLog = NULL;
    pthread_mutex_unlock(&lockBuffer);
}