#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "fryzjer.h"
#include "klient.h"
#include "kierownik.h"

#define F 3   // Liczba fryzjerów
#define N 3   // Liczba foteli
#define K 5   // Liczba miejsc w poczekalni

sem_t fotel[N];
sem_t poczekalnia;
pthread_mutex_t kasa_mutex;

Fryzjer fryzjerzy[F];
Klient klienci[10];  // W tym przypadku 10 klientów

// Deklaracje funkcji
void* fryzjer_praca(void* arg);
void* klient_praca(void* arg);

int main() {
    // Inicjalizacja semaforów
    sem_init(&poczekalnia, 0, K);  // K miejsc w poczekalni
    for (int i = 0; i < N; i++) {
        sem_init(&fotel[i], 0, 1);  // 1 fotel dostępny
    }
    pthread_mutex_init(&kasa_mutex, NULL);

    // Tworzenie fryzjerów
    pthread_t fryzjer_threads[F];  // Tablica dla wątków fryzjerów
    for (int i = 0; i < F; i++) {
        fryzjerzy[i].id = i + 1;
        fryzjerzy[i].poczekalnia = &poczekalnia;
        fryzjerzy[i].kasa = &fotel[i];
        fryzjerzy[i].mutex_kasa = &kasa_mutex;

        // Tworzenie wątku dla każdego fryzjera
        if (pthread_create(&fryzjer_threads[i], NULL, fryzjer_praca, &fryzjerzy[i]) != 0) {
            perror("Błąd przy tworzeniu wątku fryzjera");
            exit(1);
        }
    }

    // Tworzenie klientów
    pthread_t klient_threads[10];  // Tablica wątków klientów
    for (int i = 0; i < 10; i++) {
        klienci[i].id = i + 1;
        klienci[i].fotel = &fotel[i % N];  // Rozdzielanie foteli do klientów
        if (pthread_create(&klient_threads[i], NULL, klient_praca, &klienci[i]) != 0) {
            perror("Błąd przy tworzeniu wątku klienta");
            exit(1);
        }
    }

    // Czekanie na zakończenie wątków klientów
    for (int i = 0; i < 10; i++) {
        pthread_join(klient_threads[i], NULL);
    }

    // Czekanie na zakończenie wątków fryzjerów
    for (int i = 0; i < F; i++) {
        pthread_cancel(fryzjer_threads[i]);  // Zakończenie wątków fryzjerów
    }

    // Sprzątanie
    pthread_mutex_destroy(&kasa_mutex);
    for (int i = 0; i < N; i++) {
        sem_destroy(&fotel[i]);
    }
    sem_destroy(&poczekalnia);

    return 0;
}
