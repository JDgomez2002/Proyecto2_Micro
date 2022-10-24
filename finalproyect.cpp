//Universidad del Valle de Guatemala
//Programacion de Microprocesadores
//Seccion 21
//Catedratico Roger Diaz
//Segundo Semestre 2022
//Abner Ivan Garcia Alegria 21285
//Samuel Alejandro Chamale Rac 21881
//Adrian Rodriguez 21691
//Jose Daniel Gomez Cabrera 21429
//Actividad: Proyecto 02

#include <iostream>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

int num_cars;
int num_pits;
int total_distance;
bool race_finished = false;
int race_time_seconds;

pthread_mutex_t pitMutex;
pthread_cond_t pitCond;
bool pitUsed = false;

std::vector<int> ranking;

pthread_mutex_t rankingMutex;
sem_t lapSemaphore;

struct car_data
{
    int distance;
    int fuel;
    int id;
    int cars_competing;
    int pit_stops;
};
car_data *cars;

void *car(void *arg)
{

    car_data *car = (car_data *)arg;
    car = (car_data *)arg;
    car->fuel = 200;
    int temp_distance = 0;
    int laps = 0;
    int stops =0; 
    while (car->distance < total_distance)
    {
        printf("Car %d: %d m \n", car->id, car->distance);
        printf("Car %d: current fuel %d \n", car->id, car->fuel);
        if (car->fuel > 0)
        {
            srand(car->id);
            int randSpeed = 1 + (rand() % 3);
            int fuelRand = 30 + (rand() % 70);

            temp_distance += 200 * randSpeed;
            printf("Temp Distance of Car %d: %d m \n", car->id, temp_distance);
            printf("Car %d: current fuel %d \n", car->id, car->fuel);
            if (temp_distance >= 1000 * (laps + 1))
            {
                printf("Car %d: attempting to cross a lap \n", car->id);
                sem_wait(&lapSemaphore);
                sleep(1);
                printf("Car %d: completed a lap \n", car->id);
                sem_post(&lapSemaphore);
                laps += 1;
                car->distance += 200 * randSpeed;
                //car->fuel -= 40 * randSpeed;
                car->fuel -= fuelRand * randSpeed; 
            }
            else
            {
                car->distance += 200 * randSpeed;
                car->fuel -= fuelRand * randSpeed;
                sleep(1);
            }
        }
        else
        {
            printf("Car %d: need fuel \n", car->id);
             stops+=1;
            car->pit_stops = stops;
            pthread_mutex_lock(&pitMutex);
            // wait 
            pitUsed = true;
            while (pitUsed == true)
            {
                printf("Car %d: in pit... \n", car->id);
                pthread_cond_wait(&pitCond, &pitMutex);
            }

            printf("Car %d: got fuel, left the pit... \n", car->id);
            pthread_mutex_unlock(&pitMutex);
            car->fuel = 200;
        }
    }
    pthread_mutex_lock(&rankingMutex);
    ranking.push_back(car->id);
    pthread_mutex_unlock(&rankingMutex);

    printf("Car %d: Finished \n", car->id);

    if (ranking.size() == car->cars_competing)
    {
        race_finished = true;
    }
    return NULL;
}

void *pit(void *arg)
{
    while (race_finished == false)
    {
        if (pitUsed == true)
        {
            pthread_mutex_lock(&pitMutex);
            printf("\tFrom Pit: Fixed Car\n");
            pitUsed = false;
            pthread_mutex_unlock(&pitMutex);
            pthread_cond_broadcast(&pitCond);
            sleep(1);
        }
    }

    return NULL;
}

void *race_timer(void *arg)
{
    while (race_finished == false)
    {
        sleep(1);
        race_time_seconds++;
    }

    return NULL;
}

int main()
{
    std::cout << "---------------------------------------------------------------------------------------------\n";
    std::cout << "******** Welcome to the racing car simulator, it's a pleasure to have you here :) *******\n";
    std::cout << "---------------------------------------------------------------------------------------------\n";
    std::cout << "Number of cars: \n >> ";
    std::cin >> num_cars;
    std::cout << "\nNumber of pits (< Number of cars): \n >> ";
    std::cin >> num_pits;
    std::cout << "\nTotal distance (m): \n >> ";
    std::cin >> total_distance;

    pthread_t cars_tid[num_cars];
    pthread_t pits_tid[num_pits];
    pthread_t thread_timer;

    cars = new car_data[num_cars];

    race_time_seconds = 0;

    pthread_mutex_init(&rankingMutex, NULL);
    sem_init(&lapSemaphore, 0, 3);

    pthread_mutex_init(&pitMutex, NULL);
    pthread_cond_init(&pitCond, NULL);
    long i;

    for (i = 0; i < num_cars; i++)
    {
        cars[i] = {
            .distance = 0,
            .fuel = 140, 
            .id = i+1,
            .cars_competing = num_cars,
            .pit_stops = 0
        };
        pthread_create(&cars_tid[i], NULL, car, (void *)&cars[i]);
    }
    for (i = 0; i < num_pits; i++)
    {
        pthread_create(&pits_tid[i], NULL, pit, (void *)i);
    }

    pthread_create(&thread_timer, NULL, race_timer, (void *)i);
    
    for (i = 0; i < num_cars; i++)
    {
        pthread_join(cars_tid[i], NULL);
    }
    for (i = 0; i < num_pits; i++)
    {
        pthread_join(pits_tid[i], NULL);
    }

    pthread_join(thread_timer, NULL);

    pthread_mutex_destroy(&rankingMutex);
    pthread_mutex_destroy(&pitMutex);
    pthread_cond_destroy(&pitCond);
    std::cout << "\n---------------------------------------------------------";
    std::cout << "\n"
              << "**************************RESUlTS*********************** \n";
    std::cout << "---------------------------------------------------------\n"; 
    for (int i = 0; i < ranking.size(); i++)
    {
        std::cout << i + 1 << ". CAR " << ranking.at(i) <<  ", PITSTOPS: " << cars[ranking.at(i)-1].pit_stops << "\n";
    }
    std::cout << "\nTiempo en carrera: " << race_time_seconds << " segundos.\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << "*****End of the race, I hope you come back soon :)*****\n";
    std::cout << "---------------------------------------------------------\n";
    return 0;
}