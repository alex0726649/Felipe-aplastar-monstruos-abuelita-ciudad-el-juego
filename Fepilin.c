#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NOMBRE 50
#define MAX_EFECTO 100
#define MAX_ASCII 2000
#define HASH_SIZE 50
#define MAX_ARMAS_EQUIPADAS 3
#define TOTAL_ENEMIGOS 100

// ==================== ESTRUCTURAS ====================

typedef struct Arma {
    char nombre[MAX_NOMBRE];
    int danio;
    int precio;
    int usos;
    int usos_max;
    float prob_critico;
    char efecto[MAX_EFECTO];
    struct Arma *siguiente;
} Arma;

typedef struct NodoArmaEquipada {
    Arma arma;
    struct NodoArmaEquipada *siguiente;
    struct NodoArmaEquipada *anterior;
} NodoArmaEquipada;

typedef struct {
    char nombre[MAX_NOMBRE];
    int vida;
    int vida_max;
    int fuerza;
    int nivel_piso;
    char ascii_art[MAX_ASCII];
    int oro_drop;
    int es_miniboss;
    int es_boss;
} Enemigo;

typedef struct NodoEnemigo {
    Enemigo enemigo;
    struct NodoEnemigo *siguiente;
} NodoEnemigo;

typedef struct {
    char nombre[MAX_NOMBRE];
    char efecto[MAX_EFECTO];
    int bonus_danio;
    int bonus_vida;
    float bonus_critico;
    int bonus_velocidad;
} Amuleto;

typedef struct {
    int vida;
    int vida_max;
    int oro;
    int velocidad;
    int nivel_piso;
    NodoArmaEquipada *armas_equipadas;
    int num_armas_equipadas;
    NodoArmaEquipada *arma_actual;
    Amuleto amuleto;
    int tiene_amuleto;
    int danio_temporal;
    int vida_temporal;
    float critico_temporal;
} Personaje;

typedef struct {
    Arma *tabla[HASH_SIZE];
} TablaHashArmas;

typedef struct {
    NodoEnemigo *tope;
    int cantidad;
} PilaEnemigos;

// ==================== PROTOTIPOS ====================

void mostrar_titulo();
void inicializar_juego(Personaje *p, TablaHashArmas *th, PilaEnemigos *pila);
void cargar_armas_desde_archivo(TablaHashArmas *th);
void cargar_ascii_desde_archivo(char ascii_array[][MAX_ASCII], int *num_ascii);
void cargar_partida(Personaje *p, PilaEnemigos *pila);
void guardar_partida(Personaje *p, PilaEnemigos *pila);
void guardar_armas_en_archivo(TablaHashArmas *th);

unsigned int hash(char *str);
void insertar_arma_hash(TablaHashArmas *th, Arma arma);
Arma* buscar_arma_hash(TablaHashArmas *th, char *nombre);
void mostrar_inventario_hash(TablaHashArmas *th, Personaje *p);

void insertar_arma_equipada(Personaje *p, Arma arma);
void eliminar_arma_equipada(Personaje *p, char *nombre);
void ciclar_arma_siguiente(Personaje *p);
void ciclar_arma_anterior(Personaje *p);
void mostrar_arma_actual(Personaje *p);

void push_enemigo(PilaEnemigos *pila, Enemigo enemigo);
Enemigo pop_enemigo(PilaEnemigos *pila);
bool pila_vacia(PilaEnemigos *pila);
void generar_enemigos_iniciales(PilaEnemigos *pila, char ascii_array[][MAX_ASCII], int num_ascii);

void bucle_juego_recursivo(Personaje *p, PilaEnemigos *pila, TablaHashArmas *th, char ascii_array[][MAX_ASCII], int num_ascii);
void combate(Personaje *p, Enemigo *enemigo);
void tienda(Personaje *p, TablaHashArmas *th);
void menu_inventario(Personaje *p, TablaHashArmas *th);

int calcular_danio_recursivo(int danio_base, int profundidad);
void limpiar_pantalla();
void pausar();

char ascii_enemigos[12][MAX_ASCII];
int num_ascii_enemigos = 0;

// ==================== MAIN ====================

int main() {
    srand(time(NULL));
    
    limpiar_pantalla();
    mostrar_titulo();
    
    Personaje jugador = {0};
    TablaHashArmas tabla_hash = {0};
    PilaEnemigos pila_enemigos = {0};
    
    printf("\n\n1. Nueva Partida\n");
    printf("2. Continuar Partida\n");
    printf("Selecciona: ");
    
    int opcion;
    scanf("%d", &opcion);
    getchar();
    
    if (opcion == 2) {
        cargar_partida(&jugador, &pila_enemigos);
        cargar_armas_desde_archivo(&tabla_hash);
        cargar_ascii_desde_archivo(ascii_enemigos, &num_ascii_enemigos);
    } else {
        inicializar_juego(&jugador, &tabla_hash, &pila_enemigos);
    }
    
    bucle_juego_recursivo(&jugador, &pila_enemigos, &tabla_hash, ascii_enemigos, num_ascii_enemigos);
    
    printf("\n=== GAME OVER ===\n");
    printf("Llegaste al piso: %d\n", jugador.nivel_piso);
    printf("Oro recolectado: %d\n", jugador.oro);
    
    return 0;
}

// ==================== TITULO ====================

void mostrar_titulo() {
    FILE *f = fopen("titulo.txt", "r");
    if (!f) {
        printf("=== DUNGEON CRAWLER ===\n");
        return;
    }
    
    char linea[256];
    while (fgets(linea, sizeof(linea), f)) {
        printf("%s", linea);
    }
    fclose(f);
}

// ==================== INICIALIZACIÓN ====================

void inicializar_juego(Personaje *p, TablaHashArmas *th, PilaEnemigos *pila) {
    p->vida = 100;
    p->vida_max = 100;
    p->oro = 50;
    p->velocidad = 10;
    p->nivel_piso = 1;
    p->armas_equipadas = NULL;
    p->num_armas_equipadas = 0;
    p->arma_actual = NULL;
    p->tiene_amuleto = 0;
    p->danio_temporal = 0;
    p->vida_temporal = 0;
    p->critico_temporal = 0.0;
    
    cargar_armas_desde_archivo(th);
    cargar_ascii_desde_archivo(ascii_enemigos, &num_ascii_enemigos);
    
    Arma *espada = buscar_arma_hash(th, "Espada");
    if (espada) insertar_arma_equipada(p, *espada);
    
    generar_enemigos_iniciales(pila, ascii_enemigos, num_ascii_enemigos);
    
    printf("\nJuego iniciado correctamente!\n");
    pausar();
}

void cargar_armas_desde_archivo(TablaHashArmas *th) {
    FILE *f = fopen("armas.txt", "r");
    if (!f) {
        printf("ERROR: No se pudo abrir el archivo armas.txt\n");
        printf("Crea el archivo con el formato:\n");
        printf("NombreArma,Daño,Precio,Usos,ProbCritico,Efecto\n");
        printf("Ejemplo: Espada,15,0,999,0.10,Ataque basico\n");
        exit(1);
    }
    
    char linea[256];
    int armas_cargadas = 0;
    
    while (fgets(linea, sizeof(linea), f)) {
        Arma arma;
        int campos = sscanf(linea, "%[^,],%d,%d,%d,%f,%[^\n]", 
               arma.nombre, &arma.danio, &arma.precio, &arma.usos_max, &arma.prob_critico, arma.efecto);
        
        if (campos >= 5) {
            arma.usos = arma.usos_max;
            arma.siguiente = NULL;
            insertar_arma_hash(th, arma);
            armas_cargadas++;
        }
    }
    fclose(f);
    
    if (armas_cargadas == 0) {
        printf("ERROR: No se cargaron armas. Verifica el formato del archivo armas.txt\n");
        exit(1);
    }
    
    printf("Se cargaron %d armas desde el archivo\n", armas_cargadas);
}

void cargar_ascii_desde_archivo(char ascii_array[][MAX_ASCII], int *num_ascii) {
    FILE *f = fopen("enemigos_ascii.txt", "r");
    if (!f) {
        printf("ERROR: No se pudo abrir el archivo enemigos_ascii.txt\n");
        printf("Asegurate de tener el archivo en el mismo directorio.\n");
        exit(1);
    }
    
    char buffer[MAX_ASCII] = "";
    char linea[256];
    *num_ascii = 0;
    
    while (fgets(linea, sizeof(linea), f) && *num_ascii < 12) {
        if (strcmp(linea, "---\n") == 0 || strcmp(linea, "---\r\n") == 0) {
            if (strlen(buffer) > 0) {
                strcpy(ascii_array[*num_ascii], buffer);
                (*num_ascii)++;
                buffer[0] = '\0';
            }
        } else {
            strcat(buffer, linea);
        }
    }
    
    if (strlen(buffer) > 0 && *num_ascii < 12) {
        strcpy(ascii_array[*num_ascii], buffer);
        (*num_ascii)++;
    }
    
    fclose(f);
    
    if (*num_ascii == 0) {
        printf("ERROR: No se cargaron artes ASCII. Verifica el formato del archivo.\n");
        exit(1);
    }
    
    printf("Se cargaron %d enemigos desde el archivo\n", *num_ascii);
}

void cargar_partida(Personaje *p, PilaEnemigos *pila) {
    FILE *f = fopen("partida.txt", "r");
    if (!f) {
        printf("No hay partida guardada.\n");
        exit(1);
    }
    
    fscanf(f, "%d %d %d %d %d\n", &p->vida, &p->vida_max, &p->oro, &p->velocidad, &p->nivel_piso);
    fclose(f);
    
    p->armas_equipadas = NULL;
    p->num_armas_equipadas = 0;
    p->arma_actual = NULL;
    p->tiene_amuleto = 0;
    
    printf("Partida cargada: Piso %d, Vida %d, Oro %d\n", p->nivel_piso, p->vida, p->oro);
    pausar();
}

void guardar_partida(Personaje *p, PilaEnemigos *pila) {
    FILE *f = fopen("partida.txt", "w");
    if (f) {
        fprintf(f, "%d %d %d %d %d\n", p->vida, p->vida_max, p->oro, p->velocidad, p->nivel_piso);
        fclose(f);
    }
}

// ==================== TABLA HASH ====================

unsigned int hash(char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % HASH_SIZE;
}

void insertar_arma_hash(TablaHashArmas *th, Arma arma) {
    unsigned int indice = hash(arma.nombre);
    Arma *nueva = (Arma*)malloc(sizeof(Arma));
    *nueva = arma;
    nueva->siguiente = th->tabla[indice];
    th->tabla[indice] = nueva;
}

Arma* buscar_arma_hash(TablaHashArmas *th, char *nombre) {
    unsigned int indice = hash(nombre);
    Arma *actual = th->tabla[indice];
    while (actual) {
        if (strcmp(actual->nombre, nombre) == 0) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

void mostrar_inventario_hash(TablaHashArmas *th, Personaje *p) {
    int pagina = 0;
    
    while (1) {
        limpiar_pantalla();
        printf("=== TIENDA DE ARMAS (Oro: %d) ===\n\n", p->oro);
        
        int inicio = pagina * 5;
        int fin = inicio + 5;
        int actual = 0;
        
        for (int i = 0; i < HASH_SIZE; i++) {
            Arma *arma = th->tabla[i];
            while (arma) {
                if (actual >= inicio && actual < fin) {
                    printf("%d. %s - Danio: %d | Precio: %d | Usos: %d | Critico: %.0f%%\n",
                           actual - inicio + 1, arma->nombre, arma->danio, arma->precio, 
                           arma->usos_max, arma->prob_critico * 100);
                    printf("   Efecto: %s\n\n", arma->efecto);
                }
                actual++;
                arma = arma->siguiente;
            }
        }
        
        printf("\n[N]ext | [P]rev | [C]omprar | [S]alir\n");
        printf("Opcion: ");
        
        char opcion;
        scanf(" %c", &opcion);
        getchar();
        
        if (opcion == 'N' || opcion == 'n') pagina++;
        else if (opcion == 'P' || opcion == 'p') { if (pagina > 0) pagina--; }
        else if (opcion == 'S' || opcion == 's') break;
        else if (opcion == 'C' || opcion == 'c') {
            printf("Nombre del arma: ");
            char nombre[MAX_NOMBRE];
            fgets(nombre, MAX_NOMBRE, stdin);
            nombre[strcspn(nombre, "\n")] = 0;
            
            Arma *arma = buscar_arma_hash(th, nombre);
            if (arma && p->oro >= arma->precio) {
                if (p->num_armas_equipadas < MAX_ARMAS_EQUIPADAS) {
                    p->oro -= arma->precio;
                    insertar_arma_equipada(p, *arma);
                    printf("Arma equipada!\n");
                } else {
                    printf("Ya tienes 3 armas equipadas. Desequipa una primero.\n");
                }
                pausar();
            } else {
                printf("Arma no encontrada o no tienes suficiente oro.\n");
                pausar();
            }
        }
    }
}

// ==================== LISTA DOBLE CIRCULAR ====================

void insertar_arma_equipada(Personaje *p, Arma arma) {
    NodoArmaEquipada *nuevo = (NodoArmaEquipada*)malloc(sizeof(NodoArmaEquipada));
    nuevo->arma = arma;
    
    if (p->armas_equipadas == NULL) {
        nuevo->siguiente = nuevo;
        nuevo->anterior = nuevo;
        p->armas_equipadas = nuevo;
        p->arma_actual = nuevo;
    } else {
        NodoArmaEquipada *ultimo = p->armas_equipadas->anterior;
        nuevo->siguiente = p->armas_equipadas;
        nuevo->anterior = ultimo;
        ultimo->siguiente = nuevo;
        p->armas_equipadas->anterior = nuevo;
    }
    p->num_armas_equipadas++;
}

void eliminar_arma_equipada(Personaje *p, char *nombre) {
    if (p->armas_equipadas == NULL) return;
    
    NodoArmaEquipada *actual = p->armas_equipadas;
    do {
        if (strcmp(actual->arma.nombre, nombre) == 0) {
            if (p->num_armas_equipadas == 1) {
                free(actual);
                p->armas_equipadas = NULL;
                p->arma_actual = NULL;
            } else {
                actual->anterior->siguiente = actual->siguiente;
                actual->siguiente->anterior = actual->anterior;
                
                if (actual == p->armas_equipadas) {
                    p->armas_equipadas = actual->siguiente;
                }
                if (actual == p->arma_actual) {
                    p->arma_actual = actual->siguiente;
                }
                free(actual);
            }
            p->num_armas_equipadas--;
            return;
        }
        actual = actual->siguiente;
    } while (actual != p->armas_equipadas);
}

void ciclar_arma_siguiente(Personaje *p) {
    if (p->arma_actual) {
        p->arma_actual = p->arma_actual->siguiente;
    }
}

void ciclar_arma_anterior(Personaje *p) {
    if (p->arma_actual) {
        p->arma_actual = p->arma_actual->anterior;
    }
}

void mostrar_arma_actual(Personaje *p) {
    if (p->arma_actual) {
        Arma *a = &p->arma_actual->arma;
        printf("\n=== ARMA ACTUAL ===\n");
        printf("Nombre: %s\n", a->nombre);
        printf("Danio: %d\n", a->danio);
        printf("Usos restantes: %d/%d\n", a->usos, a->usos_max);
        printf("Prob. Critico: %.0f%%\n", a->prob_critico * 100);
        printf("Efecto: %s\n", a->efecto);
    } else {
        printf("No tienes armas equipadas.\n");
    }
}

// ==================== PILA DE ENEMIGOS ====================

void push_enemigo(PilaEnemigos *pila, Enemigo enemigo) {
    NodoEnemigo *nuevo = (NodoEnemigo*)malloc(sizeof(NodoEnemigo));
    nuevo->enemigo = enemigo;
    nuevo->siguiente = pila->tope;
    pila->tope = nuevo;
    pila->cantidad++;
}

Enemigo pop_enemigo(PilaEnemigos *pila) {
    if (pila->tope == NULL) {
        Enemigo vacio = {0};
        return vacio;
    }
    
    NodoEnemigo *temp = pila->tope;
    Enemigo enemigo = temp->enemigo;
    pila->tope = pila->tope->siguiente;
    free(temp);
    pila->cantidad--;
    return enemigo;
}

bool pila_vacia(PilaEnemigos *pila) {
    return pila->tope == NULL;
}

void generar_enemigos_iniciales(PilaEnemigos *pila, char ascii_array[][MAX_ASCII], int num_ascii) {
    char *nombres[] = {"Duende", "Lobo", "Bruja", "Araña", "Ogro", "Esqueleto", 
                       "Golem", "Dragon", "Felipe", "Osvaldo", "Inge", "Gigante"};
    
    for (int i = TOTAL_ENEMIGOS - 1; i >= 0; i--) {
        Enemigo enemigo;
        
        int piso = (i / 10) + 1;
        int multiplicador = 1 + (piso / 5);
        
        if ((i + 1) % 10 == 0) {
            int boss_idx = (rand() % 3) + 9;
            if (boss_idx >= num_ascii) boss_idx = num_ascii - 1;
            strcpy(enemigo.nombre, nombres[boss_idx]);
            strcpy(enemigo.ascii_art, ascii_array[boss_idx]);
            enemigo.vida = 100 + (piso * 30);
            enemigo.fuerza = 20 + (piso * 5);
            enemigo.oro_drop = 50 + (piso * 20);
            enemigo.es_boss = 1;
            enemigo.es_miniboss = 0;
        } else if ((i + 1) % 3 == 0) {
            int mini_idx = 4 + (rand() % 4);
            if (mini_idx >= num_ascii) mini_idx = num_ascii - 1;
            strcpy(enemigo.nombre, nombres[mini_idx]);
            strcpy(enemigo.ascii_art, ascii_array[mini_idx]);
            enemigo.vida = 40 + (piso * 15);
            enemigo.fuerza = 10 + (piso * 3);
            enemigo.oro_drop = 20 + (piso * 8);
            enemigo.es_miniboss = 1;
            enemigo.es_boss = 0;
        } else {
            int idx = rand() % 4;
            if (idx >= num_ascii) idx = 0;
            strcpy(enemigo.nombre, nombres[idx]);
            strcpy(enemigo.ascii_art, ascii_array[idx]);
            enemigo.vida = 20 + (piso * 8) + (rand() % 10);
            enemigo.fuerza = 5 + (piso * 2) + (rand() % 5);
            enemigo.oro_drop = 5 + (piso * 3) + (rand() % 10);
            enemigo.es_miniboss = 0;
            enemigo.es_boss = 0;
        }
        
        enemigo.vida_max = enemigo.vida;
        enemigo.nivel_piso = piso;
        
        push_enemigo(pila, enemigo);
    }
}

// ==================== RECURSIVIDAD ====================

void bucle_juego_recursivo(Personaje *p, PilaEnemigos *pila, TablaHashArmas *th, char ascii_array[][MAX_ASCII], int num_ascii) {
    if (p->vida <= 0 || pila_vacia(pila)) {
        return;
    }
    
    limpiar_pantalla();
    printf("==========================================\n");
    printf("        PISO %d / %d                  \n", p->nivel_piso, TOTAL_ENEMIGOS / 10);
    printf("  Vida: %d/%d  |  Oro: %d           \n", p->vida, p->vida_max, p->oro);
    printf("==========================================\n\n");
    
    if (p->nivel_piso % 5 == 0) {
        printf("Un mercader ambulante aparece!\n");
        pausar();
        tienda(p, th);
    }
    
    Enemigo enemigo = pop_enemigo(pila);
    
    printf("\nUn %s aparece!\n", enemigo.nombre);
    if (enemigo.es_boss) printf("*** ES UN BOSS! ***\n");
    else if (enemigo.es_miniboss) printf("** Es un mini-boss! **\n");
    
    printf("\n%s\n", enemigo.ascii_art);
    printf("Vida: %d | Fuerza: %d\n", enemigo.vida, enemigo.fuerza);
    pausar();
    
    combate(p, &enemigo);
    
    if (p->vida > 0) {
        p->oro += enemigo.oro_drop;
        printf("\nVictoria! Ganaste %d oro.\n", enemigo.oro_drop);
        
        if (rand() % 100 < 20) {
            printf("El enemigo dejo caer un amuleto!\n");
            Amuleto nuevo_amuleto;
            strcpy(nuevo_amuleto.nombre, "Amuleto de Poder");
            strcpy(nuevo_amuleto.efecto, "Aumenta danio");
            nuevo_amuleto.bonus_danio = 5 + (rand() % 10);
            nuevo_amuleto.bonus_vida = 0;
            nuevo_amuleto.bonus_critico = 0.0;
            nuevo_amuleto.bonus_velocidad = 0;
            
            if (p->tiene_amuleto) {
                printf("Reemplazar amuleto actual? (s/n): ");
                char op;
                scanf(" %c", &op);
                getchar();
                if (op == 's' || op == 'S') {
                    p->amuleto = nuevo_amuleto;
                    printf("Amuleto equipado!\n");
                }
            } else {
                p->amuleto = nuevo_amuleto;
                p->tiene_amuleto = 1;
                printf("Amuleto equipado!\n");
            }
        }
        
        pausar();
        menu_inventario(p, th);
        
        p->nivel_piso++;
        guardar_partida(p, pila);
        
        bucle_juego_recursivo(p, pila, th, ascii_array, num_ascii);
    }
}

// ==================== COMBATE ====================

void combate(Personaje *p, Enemigo *enemigo) {
    p->danio_temporal = 0;
    p->critico_temporal = 0.0;
    
    while (p->vida > 0 && enemigo->vida > 0) {
        limpiar_pantalla();
        printf("============== COMBATE ==============\n");
        printf("  TU: %d/%d HP  |  %s: %d/%d HP   \n", 
               p->vida, p->vida_max, enemigo->nombre, enemigo->vida, enemigo->vida_max);
        printf("=====================================\n\n");
        
        printf("%s\n", enemigo->ascii_art);
        
        if (p->arma_actual) {
            printf("\nArma actual: %s (Danio: %d, Usos: %d)\n", 
                   p->arma_actual->arma.nombre, 
                   p->arma_actual->arma.danio, 
                   p->arma_actual->arma.usos);
        }
        
        printf("\n[A]tacar | [S]iguiente arma | [P]revia arma | [H]uir\n");
        printf("Accion: ");
        
        char accion;
        scanf(" %c", &accion);
        getchar();
        
        if (accion == 'A' || accion == 'a') {
            if (p->arma_actual == NULL) {
                printf("No tienes armas equipadas!\n");
                pausar();
                continue;
            }
            
            int danio_total = calcular_danio_recursivo(p->arma_actual->arma.danio, 3);
            danio_total += p->danio_temporal;
            
            if (p->tiene_amuleto) {
                danio_total += p->amuleto.bonus_danio;
            }
            
            float prob_crit = p->arma_actual->arma.prob_critico + p->critico_temporal;
            if (p->tiene_amuleto) {
                prob_crit += p->amuleto.bonus_critico;
            }
            
            bool es_critico = ((float)rand() / RAND_MAX) < prob_crit;
            if (es_critico) {
                danio_total *= 2;
                printf("\nGOLPE CRITICO!\n");
            }
            
            enemigo->vida -= danio_total;
            printf("Causaste %d de danio!\n", danio_total);
            
            p->arma_actual->arma.usos--;
            if (p->arma_actual->arma.usos <= 0) {
                printf("Tu %s se rompio!\n", p->arma_actual->arma.nombre);
                char nombre_arma[MAX_NOMBRE];
                strcpy(nombre_arma, p->arma_actual->arma.nombre);
                eliminar_arma_equipada(p, nombre_arma);
            }
            
            if (enemigo->vida > 0) {
                int danio_enemigo = enemigo->fuerza + (rand() % 5);
                p->vida -= danio_enemigo;
                printf("%s te causo %d de danio!\n", enemigo->nombre, danio_enemigo);
            }
            
            pausar();
            
        } else if (accion == 'S' || accion == 's') {
            ciclar_arma_siguiente(p);
            mostrar_arma_actual(p);
            pausar();
        } else if (accion == 'P' || accion == 'p') {
            ciclar_arma_anterior(p);
            mostrar_arma_actual(p);
            pausar();
        } else if (accion == 'H' || accion == 'h') {
            if (rand() % 100 < 50) {
                printf("Lograste huir!\n");
                pausar();
                return;
            } else {
                printf("No pudiste huir!\n");
                int danio_enemigo = enemigo->fuerza;
                p->vida -= danio_enemigo;
                printf("%s te causo %d de danio!\n", enemigo->nombre, danio_enemigo);
                pausar();
            }
        }
    }
    
    if (enemigo->vida <= 0) {
        printf("\nDerrotaste al %s!\n", enemigo->nombre);
    }
}

// ==================== TIENDA ====================

void tienda(Personaje *p, TablaHashArmas *th) {
    while (1) {
        limpiar_pantalla();
        printf("============== MERCADER ==============\n");
        printf("  Oro disponible: %d                  \n", p->oro);
        printf("======================================\n\n");
        
        printf("1. Pocion de curacion (20 oro) - Restaura 30 HP\n");
        printf("2. Pocion de fuerza (30 oro) - +10 danio temporal\n");
        printf("3. Pocion de critico (35 oro) - +15%% critico temporal\n");
        printf("4.Totem de vida (50 oro) - +20 vida maxima\n");
        printf("5. Ver armas disponibles\n");
        printf("6. Salir\n");
        printf("\nOpcion: ");
        int opcion;
    scanf("%d", &opcion);
    getchar();
    
    if (opcion == 1 && p->oro >= 20) {
        p->oro -= 20;
        p->vida += 30;
        if (p->vida > p->vida_max) p->vida = p->vida_max;
        printf("Vida restaurada!\n");
        pausar();
    } else if (opcion == 2 && p->oro >= 30) {
        p->oro -= 30;
        p->danio_temporal += 10;
        printf("Fuerza aumentada temporalmente!\n");
        pausar();
    } else if (opcion == 3 && p->oro >= 35) {
        p->oro -= 35;
        p->critico_temporal += 0.15;
        printf("Critico aumentado temporalmente!\n");
        pausar();
    } else if (opcion == 4 && p->oro >= 50) {
        p->oro -= 50;
        p->vida_max += 20;
        p->vida += 20;
        printf("Vida maxima aumentada!\n");
        pausar();
    } else if (opcion == 5) {
        mostrar_inventario_hash(th, p);
    } else if (opcion == 6) {
        break;
    } else {
        printf("Opcion invalida o no tienes suficiente oro.\n");
        pausar();
    }
}}
// ==================== MENÚ INVENTARIO ====================
void menu_inventario(Personaje *p, TablaHashArmas *th) {
limpiar_pantalla();
printf("============== INVENTARIO ==============\n");
printf("  Vida: %d/%d  |  Oro: %d            \n", p->vida, p->vida_max, p->oro);
printf("========================================\n\n");
printf("Armas equipadas (%d/3):\n", p->num_armas_equipadas);
if (p->armas_equipadas) {
    NodoArmaEquipada *actual = p->armas_equipadas;
    int i = 1;
    do {
        printf("%d. %s (Danio: %d, Usos: %d/%d)\n", i++,
               actual->arma.nombre, actual->arma.danio, 
               actual->arma.usos, actual->arma.usos_max);
        actual = actual->siguiente;
    } while (actual != p->armas_equipadas);
} else {
    printf("  (vacio)\n");
}

if (p->tiene_amuleto) {
    printf("\nAmuleto equipado: %s\n", p->amuleto.nombre);
    printf("  Efecto: %s (+%d danio)\n", p->amuleto.efecto, p->amuleto.bonus_danio);
}

printf("\nPresiona Enter para continuar...");
getchar();
}
// ==================== FUNCIONES AUXILIARES ====================
int calcular_danio_recursivo(int danio_base, int profundidad) {
if (profundidad == 0) {
return danio_base;
}
int variacion = (rand() % 3) - 1;
return calcular_danio_recursivo(danio_base + variacion, profundidad - 1);
}
void limpiar_pantalla() {
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
}
void pausar() {
printf("\nPresiona Enter para continuar...");
getchar();
}
void guardar_armas_en_archivo(TablaHashArmas *th) {
FILE *f = fopen("armas.txt", "w");
if (!f) return;
for (int i = 0; i < HASH_SIZE; i++) {
    Arma *actual = th->tabla[i];
    while (actual) {
        fprintf(f, "%s,%d,%d,%d,%.2f,%s\n",
                actual->nombre, actual->danio, actual->precio,
                actual->usos_max, actual->prob_critico, actual->efecto);
        actual = actual->siguiente;
    }
}
fclose(f);
}