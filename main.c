#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#include "header.h"

#define NUM_PROCESSI_OP 2
#define NUM_PROCESSI_CALCOLO 1
#define NUM_PROCESSI_RIS 1

int main(){

    /* TODO: definire la chiave per il monitor operandi */
    key_t key_shm_operandi = IPC_PRIVATE; 
    key_t key_ris_shm = IPC_PRIVATE;
    key_t key_ris_sem = IPC_PRIVATE;

    // semaforo per disciplinare la shm risultato
    int ds_ris_sem = semget(key_ris_sem, 2, IPC_CREAT|0664);

    if(ds_ris_sem<0) {
        perror("ds_ris_sem errore");
        exit(1);
        
    }

    /* TODO: ottenere il descrittore per il monitor operandi */
    int id_shm_operandi = shmget(key_shm_operandi, sizeof(MonitorOperandi), IPC_CREAT|0664); // Nota: usiamo sizeof(MonitorOperandi) perché include sia il Monitor che l'array dati

    /* TODO: ottenere il riferimento alla shared memory che contiene il monitor operandi */
    MonitorOperandi * pc_op = MonitorOperandi * pc_op = (MonitorOperandi *) shmat(id_shm_operandi, NULL, 0); 

    int id_ris_shm = shmget(key_ris_shm, sizeof(statistica), IPC_CREAT|0664);

        if( id_ris_shm < 0 ) {
        perror("errore shm id_ris_shm");
        exit(1);
    }

    statistica * ris_shm = shmat(id_ris_shm, NULL, 0);

    if( pc_op == (void*)-1 ) {
        perror("errore shmat ris_shm");
        exit(1);
    }

    /* TODO: Inizializzare il monitor operandi con le relative variabili per la sincronizzazione */
    // Inizializza il Monitor Hoare.
    // Il "2" indica il numero di Condition Variables (CV_PROD e CV_CONS definite in header.h)
    init_monitor(&(pc_op->m), 2); 

    // Inizializza le variabili per la gestione circolare (Sezione 8 Formulario)
    pc_op->testa = 0;
    pc_op->coda = 0;
    pc_op->conteggio = 0;


    semctl(ds_ris_sem, SPAZIO_DISPONIBILE, SETVAL, 1);
    semctl(ds_ris_sem, MESSAGGIO_DISPONIBILE, SETVAL, 0);


    int k;
    pid_t pid;

    for (k=0; k<NUM_PROCESSI_OP; k++){
        /* TODO: aggiungere codice per generare i processi generazione operandi */
        pid = fork();
        if (pid == 0) {
            // CODICE DEL FIGLIO
            srand(getpid() * time(NULL)); // Inizializzo random (Sez. 1)
            genera_operandi(pc_op);       // Funzione lavoratore
            exit(0);                      // TERMINAZIONE OBBLIGATORIA (Sez. 2)
        }
    }
    
    /* TODO: aggiungere codice per generare il processo prelievo operandi */
        pid = fork();
        if (pid == 0) {
            srand(getpid() * time(NULL));
            // Questo processo preleva dalla SHM risultati
            preleva_risultato(ris_shm, ds_ris_sem);
            exit(0);
        }
    
    /* TODO: aggiungere codice per generare il processo di calcolo */
        pid = fork();
        if (pid == 0) {
            srand(getpid() * time(NULL));
            // Questo processo prende dal Monitor e scrive nella SHM risultati
            calcola(pc_op, ds_ris_sem, ris_shm);
            exit(0);
        }
    
    int status;
    // WAIT FOR TERMINATION
    // Attendiamo Produttori (2) + Calcolo (1) + Prelievo Risultati (1) = 4 processi
    for (k=0; k<NUM_PROCESSI_OP+NUM_PROCESSI_RIS+NUM_PROCESSI_CALCOLO; k++) {
        /* TODO: attendere la terminazione */
        wait(NULL);
    }

    /*TODO: Deallocazione risorse */
    remove_monitor(&(pc_op->m));          // Spegne il monitor Hoare
    shmctl(id_shm_operandi, IPC_RMID, 0); // Rimuove SHM operandi
    shmctl(id_ris_shm, IPC_RMID, 0);      // Rimuove SHM risultati
    semctl(ds_ris_sem, 0, IPC_RMID);      // Rimuove array semafori/

    exit(0);
}

