#include "header.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void inserisci_operando(MonitorOperandi * m, int operando){

    /* TODO: Implementare l'operazione di produzione operandi all'interno del pool di buffer
     * considerando l'uso di una coda circolare e del costrutto monitor signal and wait
     */
// 1. Entro nel Monitor (Mutua Esclusione)
    enter_monitor(&(m->m));

    // 2. Controllo se PIENO
    // TRAPPOLA: Uso WHILE, non IF. Se mi sveglio devo ricontrollare!
    while (m->conteggio == DIM_BUFFER) {
        wait_condition(&(m->m), CV_PROD);
    }

    // 3. Inserimento dati (Gestione Circolare)
    m->operandi[m->testa] = operando;
    m->testa = (m->testa + 1) % DIM_BUFFER;
    m->conteggio++;

    // 4. Avviso i consumatori che c'è un dato in più
    signal_condition(&(m->m), CV_CONS);

    // 5. Esco dal Monitor
    leave_monitor(&(m->m));
}

int *preleva_operandi(MonitorOperandi * m){

    int *operando = (int*)malloc(sizeof(int)*3);
    
    /* TODO: Implementare l'operazione di prelievo operandi dal pool di buffer
     * considerando l'uso di una coda circolare e del costrutto monitor signal and wait
     * N.B.: l'operazione di prelievo è intesa per 3 operandi e non uno solo come nell'esempio classico visto a lezione!
     */
    
    // 1. Entro nel Monitor
    enter_monitor(&(m->m));

    // 2. Controllo: aspetto se ci sono MENO di 3 elementi
    while (m->conteggio < 3) {
        wait_condition(&(m->m), CV_CONS);
    }
    
    // --- PRELIEVO 1 ---
    // Riempio operando[0] copiando dalla coda
    operando[0] = m->operandi[m->coda];
    
    // Aggiorno coda e conteggio
    m->coda = (m->coda + 1) % DIM_BUFFER;
    m->conteggio--;

    // --- PRELIEVO 2 ---
    operando[1] = m->operandi[m->coda];
    
    // Aggiorno coda e conteggio
    m->coda = (m->coda + 1) % DIM_BUFFER;
    m->conteggio--;
    
    // --- PRELIEVO 3 ---
    operando[2] = m->operandi[m->coda];
    
    // Aggiorno coda e conteggio
    m->coda = (m->coda + 1) % DIM_BUFFER;
    m->conteggio--;
    
    // 3. Ho finito di prelevare. Segnalo ai Produttori (Spazio libero)
    signal_condition(&(m->m), CV_PROD);

    // 4. Esco
    leave_monitor(&(m->m));
    
    return operando;
}

void inserisci_risultato(statistica *stats_ptr, int ds_sem, statistica value){

    /* TODO: Aggiungere codice per la sincronizzazione */
    // 1. Aspetto lo SPAZIO (semaforo verde -> rosso)
    Wait_Sem(ds_sem, SPAZIO_DISPONIBILE);
    
    // 2. Scrivo nella memoria condivisa
    *stats_ptr = value;
    
    // 3. Segnalo il MESSAGGIO (semaforo rosso -> verde)
    Signal_Sem(ds_sem, MESSAGGIO_DISPONIBILE);
    
}

void preleva_risultato(statistica *stats_ptr, int ds_sem){
    
    int i;
    for (i=0; i<4; i++){
/* TODO: Aggiungere codice per la sincronizzazione */
        
        // --- PRIMA DELLA PRINTF (Aspetto il dato) ---
        Wait_Sem(ds_sem, MESSAGGIO_DISPONIBILE);
        // --------------------------------------------

        printf("[preleva_risultato #%d] Il valore consumato stats: average %.2f, min %d, max %d\n", i,  stats_ptr->average, stats_ptr->min, stats_ptr->max);

        // --- DOPO LA PRINTF (Libero lo spazio) ---
        Signal_Sem(ds_sem, SPAZIO_DISPONIBILE);
        // -----------------------------------------
          }
}

void genera_operandi(MonitorOperandi *mon){

        int i, op;
        srand(time(NULL)*getpid());
        for (i=0; i<6; i++){
                
                // call inserisci_operando generato da 1 a 10
                op = 1 + rand() % 100;
                inserisci_operando(mon, op);
        }
}


void calcola(MonitorOperandi *mon_op, int ds_sem, statistica *stats_ptr){

        int i;
        int min = 0, max = 0;
        float average;
        int *op;
    
        statistica value;
        
        for (i=0; i<4; i++){
        
                // preleva operandi
                op = preleva_operandi(mon_op);
                int op1 = *op;
                int op2 = *(op+1);
                int op3 = *(op+2);
            
                printf("[processo calcola #%d] Prelevati op1: %d op2: %d op3: %d...CALCOLA STATS...\n", i, op1, op2, op3);

                // calcola average, min, max
                value.average = (float)(op1 + op2 + op3)/3;
                min = max = op[0];
                int j;
                for(j=1; j<3; j++){
                    
                    if(min > op[j])
                        min = op[j];
                    if(max < op[j])
                        max = op[j];
                }
            
                value.min = min;
                value.max = max;
            
                // inserisci risultato
                printf("[processo calcola #%d] Inserisci risultato\n", i);
                sleep(2);
                inserisci_risultato(stats_ptr, ds_sem, value);
                
                free(op);
        }
}
