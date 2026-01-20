#ifndef HEADER_H
#define HEADER_H

#include "monitor_hoare.h"
#include "semafori.h"

/* TODO: definire macro a supporto della sincronizzazione */
// Produttori e Consumatori si devono bloccare su condizioni diverse e quindi definiamo:
#define CV_PROD 0   //coda per i produttori
#define CV_CONS 1   //coda per i consumatori


#define SPAZIO_DISPONIBILE 0
#define MESSAGGIO_DISPONIBILE 1

#define DIM_BUFFER 5

typedef struct {

    // pool di buffer condivisi
    int operandi[DIM_BUFFER];

    /* TODO: definire variabile per la sincronizzazione */
    //A questo punto definiamo: 1. IL MONITOR (Meccanismo di sincronizzazione)
    Monitor m;

    // 2. GLI INDICI (Gestione Coda Circolare)
    int testa;      // Dove scrivo
    int coda;       // Dove leggo
    int conteggio;  // Quanti elementi ci sono (Fondamentale per la wait!)

} MonitorOperandi;

typedef struct {
    
    float average;
    int min;
    int max;
    
} statistica;


void inserisci_operando(MonitorOperandi * m, int operando);
int * preleva_operandi(MonitorOperandi * m);

void inserisci_risultato(statistica *stats_ptr, int ds_sem, statistica value);
void preleva_risultato(statistica *stats_ptr, int ds_sem);

void genera_operandi(MonitorOperandi *mon);

void calcola(MonitorOperandi *mon_op, int ds_sem, statistica *stats_ptr);

#endif

