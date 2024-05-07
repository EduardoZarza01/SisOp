#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#define FILAS 9
#define COLUMNAS 9
#define NUM_PALABRAS 6

char crucigrama[FILAS][COLUMNAS];
char palabras[NUM_PALABRAS][20];
char pistas[NUM_PALABRAS][50];
int respuestas_correctas[NUM_PALABRAS] = {0};
int set_actual = 0;
int juego_activo = 1;

sem_t semaforo;

int posiciones[NUM_PALABRAS][7][2] = {
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
        {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}},
        {{4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}},
        {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}},
        {{1, 5}, {1, 6}, {1, 7}, {1, 8}},
        {{1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}}
};

void inicializar_crucigrama() {
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            if (crucigrama[i][j] == ' ' || crucigrama[i][j] == '\0') {
                crucigrama[i][j] = ' ';
            }
        }
    }

    for (int k = 0; k < NUM_PALABRAS; k++) {
        int len = strlen(palabras[k]);
        for (int i = 0; i < len; i++) {
            int fila = posiciones[k][i][0];
            int columna = posiciones[k][i][1];
            if (crucigrama[fila][columna] == ' ' || crucigrama[fila][columna] == '\0') {
                crucigrama[fila][columna] = '?';
            }
        }
    }
}

void llenar_palabras(int set) {
    char* palabras_sets[5][6] = {
            {"mago", "actor", "osito", "libro", "ikea", "armario"},
            {"palo", "adios", "opaco", "bizco", "inca", "antiguo"},
            {"pato", "arpon", "otono", "circo", "isla", "artista"},
            {"sano", "autor", "oxido", "hielo", "idea", "abierto"},
            {"caos", "avion", "ozono", "bicho", "isla", "anciano"}
    };

    char* pistas_sets[5][6] = {
            {"Hábil en las artes mágicas", "Persona que interpreta un papel en una obra", "Diminutivo de un animal", "Conjunto de hojas escritas sujetas por un lado", "Tienda de muebles sueca", "Mueble para almacenar cosas"},
            {"Objeto largo y delgado, a menudo de madera", "Palabra utilizada para despedirse", "Que no permite que la luz pase a través", "La accion de cruzar los ojos", "Sociedad antigua de sudamerica", "De épocas pasadas"},
            {"Ave acuática con pico plano", "Arma para cazar ballenas", "Estación del año después del verano", "Espectáculo con acróbatas y animales", "Porción de tierra rodeada de agua", "Persona que crea obras de arte"},
            {"En buen estado físico", "Persona que escribe libros", "Corrosion por humedad en el metal", "Agua en estado sólido", "Concepción, pensamiento o plan", "No cerrado o sellado"},
            {"Estado de confusión y desorden", "Medio de transporte aéreo", "Capa protectora en la atmósfera", "Pequeño insecto", "Porción de tierra rodeada de agua", "Persona mayor"}
    };

    for (int i = 0; i < NUM_PALABRAS; i++) {
        strcpy(palabras[i], palabras_sets[set][i]);
        strcpy(pistas[i], pistas_sets[set][i]);
    }

    inicializar_crucigrama();
}

void imprimir_crucigrama() {
    printf(" \n Ingrese el número de respuesta y la palabra separada por un espacio (0 para cambiar el set o salir): \n");
    printf("  ");
    for (int j = 0; j < COLUMNAS; j++) {
        printf("%d ", j + 1);
    }
    printf("\n");
    for (int i = 0; i < FILAS; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < COLUMNAS; j++) {
            printf("%c ", crucigrama[i][j]);
        }
        printf("\n");
    }
}

void imprimir_pistas() {
    printf("Pistas:\n");
    for (int i = 0; i < NUM_PALABRAS; i++) {
        if (respuestas_correctas[i] == 0) {
            printf("%d. %s\n", i + 1, pistas[i]);
        }
    }
}

void reemplazar_letras_descubiertas(int palabra_idx) {
    int len = strlen(palabras[palabra_idx]);
    for (int i = 0; i < len; i++) {
        int fila = posiciones[palabra_idx][i][0];
        int columna = posiciones[palabra_idx][i][1];
        crucigrama[fila][columna] = palabras[palabra_idx][i];
    }
}

void cambiar_set(int signal) {
    set_actual = (set_actual + 1) % 5;
    llenar_palabras(set_actual);
    imprimir_crucigrama();
    imprimir_pistas();
    alarm(10);
}

void *temporizador(void *vargp) {
    sleep(60);
    if (juego_activo) {
        printf("Tiempo terminado, perdiste.\n");
        juego_activo = 0;
    }
    return NULL;
}

int main() {
    signal(SIGALRM, cambiar_set);
    pthread_t thread_id;
    pid_t pid = fork();

    if (pid == 0) {
        while (1) {
            sleep(10);
            kill(getppid(), SIGALRM);
        }
    } else {
        sem_init(&semaforo, 0, 1);
        llenar_palabras(set_actual);
        pthread_create(&thread_id, NULL, temporizador, NULL);

        while (1) {
            imprimir_crucigrama();
            imprimir_pistas();


            sem_wait(&semaforo);
            sem_post(&semaforo);

            int numero_respuesta;
            char respuesta[20];

            printf("Ingresa tu respuesta:");
            scanf("%d", &numero_respuesta);

            if (numero_respuesta == 0) {
                set_actual = (set_actual + 1) % 5;
                llenar_palabras(set_actual);
                continue;
            } else if (numero_respuesta == -1) {
                break;
            }

            getchar();

            scanf("%s", respuesta);

            if (strcmp(respuesta, palabras[numero_respuesta - 1]) == 0) {
                respuestas_correctas[numero_respuesta - 1] = 1;
                reemplazar_letras_descubiertas(numero_respuesta - 1);

                int todas_descubiertas = 1;
                for (int i = 0; i < NUM_PALABRAS; i++) {
                    if (respuestas_correctas[i] == 0) {
                        todas_descubiertas = 0;
                        break;
                    }
                }
                if (todas_descubiertas) {
                    printf("¡Haz completado el crucigrama!\n");
                    imprimir_crucigrama();
                    break;
                }

                printf("¡Respuesta correcta!\n");
            } else {
                printf("Respuesta incorrecta.\n");
            }
        }
        waitpid(pid, NULL, 0);
        pthread_join(thread_id, NULL);
        sem_destroy(&semaforo);

    return 0;
    }
}