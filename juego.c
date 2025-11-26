/*
    Abismo de las Pesadillas - version simple y coloreada
    Estilo combinado: estructuras del equipo + flujo mas sencillo.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

/* ==================== COLORES ANSI ==================== */
#define RESET     "\033[0m"
#define BOLD      "\033[1m"
#define ROJO      "\033[31m"
#define VERDE     "\033[32m"
#define AMARILLO  "\033[33m"
#define AZUL      "\033[34m"
#define MAGENTA   "\033[35m"
#define CYAN      "\033[36m"

/* ==================== CONSTANTES ==================== */

#define MAX_NOMBRE 50
#define MAX_EFECTO 100
#define MAX_ASCII  2000
#define HASH_SIZE  50
#define MAX_ARMAS_EQUIPADAS 3
#define TOTAL_ENEMIGOS 30   // pisos

/* ==================== ESTRUCTURAS ==================== */

typedef struct Arma {
    char nombre[MAX_NOMBRE];
    int danio;
    int precio;
    int usos;
    int usos_max;
    float prob_critico;
    char efecto[MAX_EFECTO];
    struct Arma *siguiente;   // para tabla hash
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
} Amuleto;

/* Armas que el jugador posee en su inventario (puede tener muchas) */
typedef struct NodoArmaInventario {
    Arma arma;
    struct NodoArmaInventario *siguiente;
} NodoArmaInventario;

typedef struct {
    char nombre[MAX_NOMBRE];

    int vida;
    int vida_max;
    int oro;
    int nivel_piso;
    int fuerza;

    NodoArmaEquipada *armas_equipadas;   // max 3 equipadas
    int num_armas_equipadas;
    NodoArmaEquipada *arma_actual;

    NodoArmaInventario *inventario_armas; // todas las armas que posee

    Amuleto amuleto;
    int tiene_amuleto;      // 1 = ya existe un amuleto
    int amuleto_equipado;   // 1 = el amuleto está activo

    int danio_temporal;
    float critico_temporal;

    /* efectos de enemigos */
    int veneno_turnos;
    int debuff_dano_turnos;
    int debuff_dano_cantidad;
} Personaje;

typedef struct {
    Arma *tabla[HASH_SIZE];
} TablaHashArmas;

typedef struct {
    NodoEnemigo *tope;
    int cantidad;
} PilaEnemigos;

/* ==================== VARIABLES GLOBALES ==================== */

char ascii_enemigos[12][MAX_ASCII];
int num_ascii_enemigos = 0;

/* ==================== PROTOTIPOS ==================== */

/* Utilidad */
void limpiar_pantalla();
void pausar();
int rand_entre(int min, int max);

/* Titulo */
void mostrar_titulo();

/* Creacion de personaje */
Personaje crearPersonajeInicial(void);
void mostrarPersonaje(Personaje *p);

/* Inicializacion de juego */
void inicializar_juego(Personaje *p, TablaHashArmas *th, PilaEnemigos *pila);

/* Game Over */
void mostrar_game_over(Personaje *p);

/* Archivos */
void cargar_armas_desde_archivo(TablaHashArmas *th);
void cargar_ascii_desde_archivo(char ascii_array[][MAX_ASCII], int *num_ascii);

/* Tabla hash de armas */
unsigned int hash(char *str);
void insertar_arma_hash(TablaHashArmas *th, Arma arma);
Arma* buscar_arma_hash(TablaHashArmas *th, char *nombre);

/* Inventario de armas del jugador */
void agregar_arma_inventario(Personaje *p, Arma arma);

/* Tienda / inventario visual */
void mostrar_inventario_hash(TablaHashArmas *th, Personaje *p);

/* Lista doble circular de armas equipadas */
void insertar_arma_equipada(Personaje *p, Arma arma);
void eliminar_arma_equipada(Personaje *p, char *nombre);
void ciclar_arma_siguiente(Personaje *p);
void ciclar_arma_anterior(Personaje *p);
void mostrar_arma_actual(Personaje *p);

/* Pila de enemigos */
void push_enemigo(PilaEnemigos *pila, Enemigo enemigo);
Enemigo pop_enemigo(PilaEnemigos *pila);
bool pila_vacia(PilaEnemigos *pila);
void generar_enemigos_iniciales(PilaEnemigos *pila,
                                char ascii_array[][MAX_ASCII], int num_ascii);

/* Juego principal */
void bucle_juego_recursivo(Personaje *p, PilaEnemigos *pila, TablaHashArmas *th);
void combate(Personaje *p, Enemigo *enemigo);
void tienda(Personaje *p, TablaHashArmas *th);
void menu_inventario(Personaje *p);
void quitar_amuleto(Personaje *p);

/* Dano recursivo */
int calcular_danio_recursivo(int danio_base, int profundidad);

/* ==================== MAIN ==================== */

int main() {
    srand((unsigned int)time(NULL));

    Personaje jugador;
    TablaHashArmas tabla_hash;
    PilaEnemigos pila_enemigos;

    memset(&tabla_hash, 0, sizeof(tabla_hash));
    memset(&pila_enemigos, 0, sizeof(pila_enemigos));

    limpiar_pantalla();
    mostrar_titulo();

    /* Objetivo y mini-guia */
    printf("\n" CYAN "Objetivo:" RESET " baja piso a piso en el abismo, derrota enemigos\n");
    printf("y mejora tus armas y amuleto. Si tu vida llega a 0, la partida termina.\n");
    printf("En combate: " AMARILLO "A" RESET " ataca, " AMARILLO "S/P" RESET " cambia de arma, " AMARILLO "H" RESET " intenta huir.\n");
    printf("Cada 5 pisos puede aparecer un mercader.\n");

    printf("\n" AMARILLO "Presiona Enter para crear tu personaje..." RESET);
    getchar();   // espera un Enter

    jugador = crearPersonajeInicial();

    inicializar_juego(&jugador, &tabla_hash, &pila_enemigos);

    bucle_juego_recursivo(&jugador, &pila_enemigos, &tabla_hash);

    mostrar_game_over(&jugador);

    return 0;
}

/* ==================== UTILIDADES ==================== */

void limpiar_pantalla() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar() {
    printf("\n" CYAN "Presiona Enter para continuar..." RESET);
    getchar();   // solo espera Enter
}

int rand_entre(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}

/* ==================== TITULO ==================== */

void mostrar_titulo() {
    FILE *f = fopen("titulo.txt", "r");
    if (!f) {
        printf(AZUL BOLD "=== ABISMO DE LAS PESADILLAS ===" RESET "\n");
        return;
    }

    printf(AZUL);  // todo el titulo en azul
    char linea[256];
    while (fgets(linea, sizeof(linea), f)) {
        printf("%s", linea);
    }
    printf(RESET);
    fclose(f);
}

/* ==================== CREACION DE PERSONAJE ==================== */

Personaje crearPersonajeInicial(void) {
    Personaje p;
    memset(&p, 0, sizeof(Personaje));

    printf("\n" BOLD "=== CREACION DE PERSONAJE ===" RESET "\n\n");
    printf("Nombre del personaje: ");
    fgets(p.nombre, MAX_NOMBRE, stdin);
    p.nombre[strcspn(p.nombre, "\n")] = 0;
    if (strlen(p.nombre) == 0) {
        strcpy(p.nombre, "SinNombre");
    }

    p.vida_max = 100;
    p.vida = p.vida_max;
    p.oro = 15;        /* menos oro inicial */
    p.nivel_piso = 1;
    p.fuerza = 10;

    mostrarPersonaje(&p);
    pausar();

    return p;
}

void mostrarPersonaje(Personaje *p) {
    printf("\n" BOLD "=== DATOS DEL PERSONAJE ===" RESET "\n");
    printf("Nombre: %s\n", p->nombre);
    printf("Vida: %d/%d\n", p->vida, p->vida_max);
    printf("Fuerza: %d\n", p->fuerza);
    printf("Oro inicial: %d\n", p->oro);
    printf("Piso inicial: %d\n", p->nivel_piso);
}

/* ==================== GAME OVER ==================== */

void mostrar_game_over(Personaje *p) {
    limpiar_pantalla();

    FILE *f = fopen("game_over.txt", "r");
    if (f) {
        printf(ROJO);
        char linea[256];
        while (fgets(linea, sizeof(linea), f)) {
            printf("%s", linea);
        }
        printf(RESET "\n");
        fclose(f);
    } else {
        printf(ROJO BOLD "=====================================\n");
        printf("              GAME OVER\n");
        printf("=====================================\n" RESET);
    }

    printf("\nCaiste en el abismo...\n\n");
    printf("Personaje: %s\n", p->nombre);
    printf("Piso alcanzado: %d\n", p->nivel_piso);
    printf("Oro obtenido: %d\n", p->oro);

#ifdef _WIN32
    system("pause");
#else
    printf("\nPresiona Enter para salir...");
    getchar();
#endif
}

/* ==================== INICIALIZACION DEL JUEGO ==================== */

void inicializar_juego(Personaje *p, TablaHashArmas *th, PilaEnemigos *pila) {
    p->armas_equipadas = NULL;
    p->num_armas_equipadas = 0;
    p->arma_actual = NULL;

    p->inventario_armas = NULL;

    p->tiene_amuleto = 0;
    p->amuleto_equipado = 0;
    p->amuleto.bonus_danio = 0;
    strcpy(p->amuleto.nombre, "Sin amuleto");
    strcpy(p->amuleto.efecto, "Ninguno");

    p->danio_temporal = 0;
    p->critico_temporal = 0.0f;

    p->veneno_turnos = 0;
    p->debuff_dano_turnos = 0;
    p->debuff_dano_cantidad = 0;

    cargar_armas_desde_archivo(th);
    cargar_ascii_desde_archivo(ascii_enemigos, &num_ascii_enemigos);

    /* Equipa armas basicas si existen */
    {
        Arma *espada = buscar_arma_hash(th, "Espada");
        Arma *punos  = buscar_arma_hash(th, "Punos");
        if (punos) {
            Arma copia_punos = *punos;
            copia_punos.usos = copia_punos.usos_max; // asegurar usos completos
            agregar_arma_inventario(p, copia_punos);
        }
        if (espada) {
            Arma copia_espada = *espada;
            copia_espada.usos = copia_espada.usos_max; // asegurar usos completos
            agregar_arma_inventario(p, copia_espada);
            insertar_arma_equipada(p, copia_espada);
        } else if (punos) {
            Arma copia_punos = *punos;
            copia_punos.usos = copia_punos.usos_max;
            insertar_arma_equipada(p, copia_punos);
        }
    }

    generar_enemigos_iniciales(pila, ascii_enemigos, num_ascii_enemigos);

    printf("\n" VERDE "Todo listo, %s. El abismo te espera..." RESET "\n", p->nombre);
    pausar();
}

/* ==================== ARCHIVOS ==================== */

void cargar_armas_desde_archivo(TablaHashArmas *th) {
    FILE *f = fopen("armas.txt", "r");
    if (!f) {
        printf(ROJO "ERROR: No se pudo abrir el archivo armas.txt\n" RESET);
        printf("Formato esperado:\n");
        printf("Nombre,Dano,Precio,UsosMax,ProbCritico,Efecto\n");
        exit(1);
    }

    char linea[256];
    int armas_cargadas = 0;

    while (fgets(linea, sizeof(linea), f)) {
        Arma arma;
        int campos = sscanf(linea, "%[^,],%d,%d,%d,%f,%[^\n]",
                            arma.nombre, &arma.danio, &arma.precio,
                            &arma.usos_max, &arma.prob_critico, arma.efecto);

        if (campos >= 5) {
            arma.usos = arma.usos_max;
            arma.siguiente = NULL;
            insertar_arma_hash(th, arma);
            armas_cargadas++;
        }
    }
    fclose(f);

    if (armas_cargadas == 0) {
        printf(ROJO "ERROR: No se cargaron armas. Verifica armas.txt\n" RESET);
        exit(1);
    }

    printf(VERDE "Se cargaron %d armas desde el archivo.\n" RESET, armas_cargadas);
}

void cargar_ascii_desde_archivo(char ascii_array[][MAX_ASCII], int *num_ascii) {
    FILE *f = fopen("enemigos_ascii.txt", "r");
    if (!f) {
        printf(ROJO "ERROR: No se pudo abrir enemigos_ascii.txt\n" RESET);
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
            if (strlen(buffer) + strlen(linea) + 1 < MAX_ASCII) {
                strcat(buffer, linea);
            }
        }
    }

    if (strlen(buffer) > 0 && *num_ascii < 12) {
        strcpy(ascii_array[*num_ascii], buffer);
        (*num_ascii)++;
    }

    fclose(f);

    if (*num_ascii == 0) {
        printf(ROJO "ERROR: No se cargaron artes ASCII.\n" RESET);
        exit(1);
    }

    printf(VERDE "Se cargaron %d enemigos ASCII.\n" RESET, *num_ascii);
}

/* ==================== TABLA HASH ==================== */

unsigned int hash(char *str) {
    unsigned int h = 0;
    while (*str) {
        h = (h << 5) + (unsigned char)(*str++);
    }
    return h % HASH_SIZE;
}

void insertar_arma_hash(TablaHashArmas *th, Arma arma) {
    unsigned int indice = hash(arma.nombre);
    Arma *nueva = (Arma*)malloc(sizeof(Arma));
    if (!nueva) return;
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

/* ==================== INVENTARIO DE ARMAS (MOCHILA) ==================== */

void agregar_arma_inventario(Personaje *p, Arma arma) {
    NodoArmaInventario *nuevo = (NodoArmaInventario*)malloc(sizeof(NodoArmaInventario));
    if (!nuevo) return;
    nuevo->arma = arma;
    nuevo->siguiente = p->inventario_armas;
    p->inventario_armas = nuevo;
}

/* ==================== LISTA DOBLE CIRCULAR (ARMAS EQUIPADAS) ==================== */

void insertar_arma_equipada(Personaje *p, Arma arma) {
    if (p->armas_equipadas != NULL) {
        NodoArmaEquipada *actual = p->armas_equipadas;
        do {
            if (strcmp(actual->arma.nombre, arma.nombre) == 0) {
                printf(AMARILLO "Ya tienes %s equipada.\n" RESET, arma.nombre);
                return;
            }
            actual = actual->siguiente;
        } while (actual != p->armas_equipadas);
    }
    NodoArmaEquipada *nuevo = (NodoArmaEquipada*)malloc(sizeof(NodoArmaEquipada));
    if (!nuevo) return;
    nuevo->arma = arma;

    if (p->armas_equipadas == NULL) {
        nuevo->siguiente = nuevo;
        nuevo->anterior = nuevo;
        p->armas_equipadas = nuevo;
        p->arma_actual = nuevo;
    } else if (p->num_armas_equipadas < MAX_ARMAS_EQUIPADAS) {
        NodoArmaEquipada *ultimo = p->armas_equipadas->anterior;
        nuevo->siguiente = p->armas_equipadas;
        nuevo->anterior = ultimo;
        ultimo->siguiente = nuevo;
        p->armas_equipadas->anterior = nuevo;
    } else {
        /* ya hay 3, solo reemplaza la actual */
        p->arma_actual->arma = arma;
        free(nuevo);
        printf(AMARILLO "Se reemplazo el arma actual.\n" RESET);
    }

    if (p->num_armas_equipadas < MAX_ARMAS_EQUIPADAS)
        p->num_armas_equipadas++;
}

void eliminar_arma_equipada(Personaje *p, char *nombre) {
    if (p->armas_equipadas == NULL) return;

    NodoArmaEquipada *actual = p->armas_equipadas;
    do {
        if (strcmp(actual->arma.nombre, nombre) == 0) {
            NodoArmaInventario *inv = p->inventario_armas;
            while (inv) {
                if (strcmp(inv->arma.nombre, nombre) == 0) {
                    inv->arma.usos = actual->arma.usos; // guardar usos actuales
                    break;
                }
                inv = inv->siguiente;
            }
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
    if (p->arma_actual)
        p->arma_actual = p->arma_actual->siguiente;
}

void ciclar_arma_anterior(Personaje *p) {
    if (p->arma_actual)
        p->arma_actual = p->arma_actual->anterior;
}

void mostrar_arma_actual(Personaje *p) {
    if (p->arma_actual) {
        Arma *a = &p->arma_actual->arma;
        printf("\n" BOLD "ARMA ACTUAL" RESET "\n");
        printf("Nombre: %s\n", a->nombre);
        printf("Dano: %d\n", a->danio);
        printf("Usos: %d/%d\n", a->usos, a->usos_max);
        printf("Critico: %.0f%%\n", a->prob_critico * 100);
        printf("Efecto: %s\n", a->efecto);
    } else {
        printf("No tienes armas equipadas.\n");
    }
}

/* ==================== PILA DE ENEMIGOS ==================== */

void push_enemigo(PilaEnemigos *pila, Enemigo enemigo) {
    NodoEnemigo *nuevo = (NodoEnemigo*)malloc(sizeof(NodoEnemigo));
    if (!nuevo) return;
    nuevo->enemigo = enemigo;
    nuevo->siguiente = pila->tope;
    pila->tope = nuevo;
    pila->cantidad++;
}

Enemigo pop_enemigo(PilaEnemigos *pila) {
    if (pila->tope == NULL) {
        Enemigo vacio;
        memset(&vacio, 0, sizeof(vacio));
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

void generar_enemigos_iniciales(PilaEnemigos *pila,
                                char ascii_array[][MAX_ASCII], int num_ascii) {
    char *nombres[] = {"Duende", "Lobo", "Bruja", "Arana", "Ogro", "Esqueleto",
                       "Golem", "Dragon", "Felipe", "Osvaldo", "Inge", "Gigante"};
    int total_nombres = 12;
    int i;

    for (i = TOTAL_ENEMIGOS - 1; i >= 0; i--) {
        Enemigo enemigo;
        int piso = i + 1;

        int idx = rand_entre(0, total_nombres - 1);
        if (idx >= num_ascii) idx = num_ascii - 1;

        strcpy(enemigo.nombre, nombres[idx]);
        strcpy(enemigo.ascii_art, ascii_array[idx]);

        enemigo.nivel_piso = piso;
        enemigo.es_boss = ((piso % 10) == 0);
        enemigo.es_miniboss = (!enemigo.es_boss && (piso % 3) == 0);

        int vida_base   = 18 + piso * 4 + rand_entre(0, piso * 2);
        int fuerza_base = 4  + piso * 2 + rand_entre(0, piso);

        if (enemigo.es_miniboss) {
            vida_base += 20;
            fuerza_base += 5;
        } else if (enemigo.es_boss) {
            vida_base += 50;
            fuerza_base += 10;
        }

        enemigo.vida_max = enemigo.vida = vida_base;
        enemigo.fuerza = fuerza_base;

        int oro_base = 4 + piso * 2;
        if (enemigo.es_miniboss) oro_base += 4;
        if (enemigo.es_boss) oro_base += 10;
        enemigo.oro_drop = oro_base;

        push_enemigo(pila, enemigo);
    }
}

/* ==================== RECURSIVIDAD DEL JUEGO ==================== */

void bucle_juego_recursivo(Personaje *p, PilaEnemigos *pila, TablaHashArmas *th) {
    if (p->vida <= 0 || pila_vacia(pila)) {
        return;
    }

    limpiar_pantalla();
    printf("==========================================\n");
    printf("   " BOLD "PISO %d" RESET " de %d\n", p->nivel_piso, TOTAL_ENEMIGOS);
    printf("   Vida: " VERDE "%d/%d" RESET "  |  Oro: " AMARILLO "%d" RESET "\n",
           p->vida, p->vida_max, p->oro);
    printf("==========================================\n\n");

    /* Mercader cada 5 pisos, con opcion */
    if (p->nivel_piso % 5 == 0) {
        printf(AMARILLO "Un mercader ambulante aparece.\n" RESET);
        printf("Hablar con el? (1 = si, 2 = no): ");
        int opTi;
        int valido = 0;
        while (!valido) {
            if (scanf("%d", &opTi) == 1 && (opTi == 1 || opTi == 2)) {
            valido = 1;
        } else {
            printf(ROJO "Opcion invalida. Elige 1 o 2: " RESET);
        }
        while(getchar() != '\n'); // limpiar buffer
    }
    if (opTi == 1) {
        tienda(p, th);
    }

    }

    Enemigo enemigo = pop_enemigo(pila);

    printf("Un " ROJO "%s" RESET " aparece!\n", enemigo.nombre);
    if (enemigo.es_boss) printf(MAGENTA "*** ES UN BOSS! ***" RESET "\n");
    else if (enemigo.es_miniboss) printf(MAGENTA "** Es un mini-boss! **" RESET "\n");

    printf("\n%s\n", enemigo.ascii_art);
    printf("Vida: " VERDE "%d" RESET " | Fuerza: %d\n", enemigo.vida, enemigo.fuerza);
    pausar();

    combate(p, &enemigo);

    if (p->vida > 0 && enemigo.vida <= 0) {
        /* solo si el enemigo murio ganas oro y loot */
        p->oro += enemigo.oro_drop;
        printf("\n" VERDE "Victoria! Ganaste %d de oro.\n" RESET, enemigo.oro_drop);

        /* Chance simple de amuleto */
        if (rand_entre(1, 100) <= 20) {
            Amuleto nuevo_amuleto;
            strcpy(nuevo_amuleto.nombre, "Amuleto de Poder");
            strcpy(nuevo_amuleto.efecto, "Aumenta el dano");
            nuevo_amuleto.bonus_danio = rand_entre(3, 8);

            printf(AMARILLO "\nTe encuentras un amuleto: %s\n" RESET,
                   nuevo_amuleto.nombre);
            printf("Tipo: %s (+%d de dano)\n",
                   nuevo_amuleto.efecto, nuevo_amuleto.bonus_danio);

            if (p->tiene_amuleto) {
                printf("Ya tienes un amuleto: %s (+%d de dano)\n",
                       p->amuleto.nombre, p->amuleto.bonus_danio);
                printf("Quieres reemplazarlo por el nuevo? (s/n): ");
                char op;
                scanf(" %c", &op);
                getchar();
                if (op == 's' || op == 'S') {
                    p->amuleto = nuevo_amuleto;
                    p->tiene_amuleto = 1;
                    p->amuleto_equipado = 1;
                    printf(VERDE "Nuevo amuleto equipado!\n" RESET);
                } else {
                    printf("Conservas tu amuleto actual.\n");
                }
            } else {
                p->amuleto = nuevo_amuleto;
                p->tiene_amuleto = 1;
                p->amuleto_equipado = 1;
                printf(VERDE "Amuleto equipado!\n" RESET);
            }
        }

        /* acceso al inventario despues del piso */
        printf("\nQuieres revisar tu inventario? (1 = si, 2 = no): ");
        {
            int opInv;
            int valido = 0;
            while (!valido) {
                if (scanf("%d", &opInv) == 1 && (opInv == 1 || opInv == 2)) {
                    valido = 1;
                }else{
                    printf("Teclea 1 o 2: " RESET);
                }
                while(getchar() != '\n'); 
            }
            if (opInv == 1) {
                menu_inventario(p);
            }
        }

        p->nivel_piso++;
        bucle_juego_recursivo(p, pila, th);

    } else if (p->vida > 0 && enemigo.vida > 0) {
        /* aqui significa que huiste: no hay oro ni loot */
        printf("\nEscapaste del combate, pero sigues descendiendo...\n");
        pausar();
        p->nivel_piso++;
        bucle_juego_recursivo(p, pila, th);
    }
}

/* ==================== COMBATE ==================== */

void combate(Personaje *p, Enemigo *enemigo) {
    p->danio_temporal = 0;
    p->critico_temporal = 0.0f;

    p->veneno_turnos = 0;
    p->debuff_dano_turnos = 0;
    p->debuff_dano_cantidad = 0;

    while (p->vida > 0 && enemigo->vida > 0) {
        limpiar_pantalla();

        /* dano por veneno al inicio del turno */
        if (p->veneno_turnos > 0) {
            int dano_ven = 3;
            p->vida -= dano_ven;
            if (p->vida < 0) p->vida = 0;
            p->veneno_turnos--;
            printf(ROJO "El veneno te hace %d de dano. Turnos restantes: %d\n" RESET,
                   dano_ven, p->veneno_turnos);
            if (p->vida <= 0) {
                pausar();
                break;
            }
        }

        printf("============== " BOLD "COMBATE" RESET " ==============\n");
        printf("TU: " VERDE "%d/%d" RESET "  |  %s: " ROJO "%d/%d" RESET "\n",
               p->vida, p->vida_max, enemigo->nombre,
               enemigo->vida, enemigo->vida_max);
        printf("=====================================\n\n");

        printf("%s\n", enemigo->ascii_art);

        if (p->arma_actual) {
            printf("\nArma actual: " CYAN "%s" RESET
                   " (Dano: %d, Usos: %d)\n",
                   p->arma_actual->arma.nombre,
                   p->arma_actual->arma.danio,
                   p->arma_actual->arma.usos);
        } else {
            printf("\nNo tienes armas equipadas (pegas con los punios: 5 de dano).\n");
        }

        printf("\n[A]tacar | [S]iguiente arma | [P]revia arma | [H]uir\n");
        printf("Accion: ");

        char accion;
        int entrada_valida = 0;
        while (!entrada_valida) {
            if (scanf(" %c", &accion) == 1) {
        // Convertir a mayúscula para comparar
                char accion_upper = (accion >= 'a' && accion <= 'z') ? accion - 32 : accion;
                if (accion_upper == 'A' || accion_upper == 'S' || 
                    accion_upper == 'P' || accion_upper == 'H') {
                    entrada_valida = 1;
                    accion = accion_upper; // usar la versión mayúscula
                }else {
                printf(ROJO "Accion invalida. Usa A, S, P o H: " RESET);
            }
        } else {
            printf(ROJO "Entrada invalida. Usa A, S, P o H: " RESET);
        }
        while(getchar() != '\n'); // limpiar buffer
}

        if (accion == 'A') {
            int danio_base = 5;
            float prob_crit = 0.0f;

            if (p->arma_actual) {
                danio_base = p->arma_actual->arma.danio;
                prob_crit = p->arma_actual->arma.prob_critico;
            }

            int danio_total = calcular_danio_recursivo(danio_base, 2);
            danio_total += p->danio_temporal;

            if (p->tiene_amuleto && p->amuleto_equipado)
                danio_total += p->amuleto.bonus_danio;

            /* debuff de la bruja */
            if (p->debuff_dano_turnos > 0) {
                danio_total -= p->debuff_dano_cantidad;
                if (danio_total < 1) danio_total = 1;
                printf(AMARILLO "Tu dano esta reducido por la maldicion (%d turnos).\n" RESET,
                       p->debuff_dano_turnos);
                p->debuff_dano_turnos--;
            }

            prob_crit += p->critico_temporal;
            if (prob_crit < 0.0f) prob_crit = 0.0f;
            if (prob_crit > 1.0f) prob_crit = 1.0f;

            {
                bool es_critico = ((float)rand() / RAND_MAX) < prob_crit;
                if (es_critico) {
                    danio_total *= 2;
                    printf("\n" AMARILLO BOLD "GOLPE CRITICO!" RESET "\n");
                }
            }

            enemigo->vida -= danio_total;
            if (enemigo->vida < 0) enemigo->vida = 0;
            printf("Causaste " VERDE "%d" RESET " de dano.\n", danio_total);

            if (p->arma_actual) {
                p->arma_actual->arma.usos--;
                if (p->arma_actual->arma.usos <= 0) {
                    printf(ROJO "Tu %s se rompio.\n" RESET,
                           p->arma_actual->arma.nombre);
                    {
                        char nombre_arma[MAX_NOMBRE];
                        strcpy(nombre_arma, p->arma_actual->arma.nombre);
                        eliminar_arma_equipada(p, nombre_arma);
                    }
                }
            }

            if (enemigo->vida > 0) {
                int danio_enemigo = enemigo->fuerza + rand_entre(0, 3);
                p->vida -= danio_enemigo;
                if (p->vida < 0) p->vida = 0;
                printf("%s te causa " ROJO "%d" RESET " de dano.\n",
                       enemigo->nombre, danio_enemigo);

                /* efectos especiales por tipo de enemigo */
                if (strcmp(enemigo->nombre, "Arana") == 0) {
                    if (rand_entre(1, 100) <= 40) {
                        p->veneno_turnos = 3;
                        printf(ROJO "La Arana te envenena!\n" RESET);
                    }
                } else if (strcmp(enemigo->nombre, "Bruja") == 0) {
                    if (rand_entre(1, 100) <= 35) {
                        p->debuff_dano_turnos = 3;
                        p->debuff_dano_cantidad = 3;
                        printf(MAGENTA "La Bruja debilita tu ataque!\n" RESET);
                    }
                }
            }

            pausar();

        } else if (accion == 'S') {
            ciclar_arma_siguiente(p);
            mostrar_arma_actual(p);
            pausar();
        } else if (accion == 'P') {
            ciclar_arma_anterior(p);
            mostrar_arma_actual(p);
            pausar();
        } else if (accion == 'H') {
            if (rand_entre(1, 100) <= 50) {
                printf(AMARILLO "Logras escapar!\n" RESET);
                pausar();
                return; // huiste: no hay oro ni loot
            } else {
                printf(ROJO "No pudiste huir.\n" RESET);
                {
                    int danio_enemigo = enemigo->fuerza;
                    p->vida -= danio_enemigo;
                    if (p->vida < 0) p->vida = 0;
                    printf("%s te causa " ROJO "%d" RESET " de dano.\n",
                           enemigo->nombre, danio_enemigo);
                }
                pausar();
            }
        }
    }

    if (enemigo->vida <= 0 && p->vida > 0) {
        printf("\n" VERDE "Derrotaste a %s!\n" RESET, enemigo->nombre);
    }
}

/* ==================== TIENDA ==================== */

void tienda(Personaje *p, TablaHashArmas *th) {
    int opcion;

    while (1) {
        limpiar_pantalla();
        printf("============== " BOLD "MERCADER" RESET " ==============\n");
        printf("Vida: " VERDE "%d/%d" RESET "  |  Oro: " AMARILLO "%d" RESET "\n",
               p->vida, p->vida_max, p->oro);
        printf("======================================\n\n");

        printf("1. Pocion de curacion (20 oro) - +30 HP\n");
        printf("2. Pocion de fuerza (30 oro) - +10 de dano temporal\n");
        printf("3. Pocion de critico (35 oro) - +15%% critico temporal\n");
        printf("4. Ver armas de la tienda\n");
        printf("5. Salir de la tienda\n\n");
        printf("Opcion: ");
        int entrada_valida = 0;
        while (!entrada_valida) {
            if (scanf("%d", &opcion) == 1 && opcion >= 1 && opcion <= 5) {
                entrada_valida = 1;
            } else {
                printf(ROJO "Opcion invalida. Elige 1-5: " RESET);
            }
            while(getchar() != '\n'); // limpiar buffer
        }

        if (opcion == 1 && p->oro >= 20) {
            p->oro -= 20;
            p->vida += 30;
            if (p->vida > p->vida_max) p->vida = p->vida_max;
            printf(VERDE "Te curaste.\n" RESET);
            pausar();
        } else if (opcion == 2 && p->oro >= 30) {
            p->oro -= 30;
            p->danio_temporal += 10;
            printf(VERDE "Dano aumentado temporalmente.\n" RESET);
            pausar();
        } else if (opcion == 3 && p->oro >= 35) {
            p->oro -= 35;
            p->critico_temporal += 0.15f;
            printf(VERDE "Critico aumentado temporalmente.\n" RESET);
            pausar();
        } else if (opcion == 4) {
            mostrar_inventario_hash(th, p);
        } else if (opcion == 5) {
            break;
        } else {
            printf(ROJO "Opcion invalida o sin oro suficiente.\n" RESET);
            pausar();
        }
    }

    /* Al salir de la tienda: opcion de ir al inventario antes de seguir */
    printf("\nQuieres revisar tu inventario antes de seguir descendiendo?\n");
    printf("1. Si\n");
    printf("2. No\n");
    printf("Opcion: ");
    {
        int opInv;
        while(1){
            scanf("%d", &opInv);
            getchar();
            if(opInv==1||opInv==2){
                break;
            }
        }
        if (opInv == 1) {
            menu_inventario(p);
        }
    }
}

/* ==================== MENU INVENTARIO ==================== */

void menu_inventario(Personaje *p) {
    int opcion = 0;
    while (1) {
        limpiar_pantalla();
        printf("============== " BOLD "INVENTARIO" RESET " ==============\n");
        printf("Personaje: %s\n", p->nombre);
        printf("Vida: " VERDE "%d/%d" RESET "  |  Oro: " AMARILLO "%d" RESET "\n",
               p->vida, p->vida_max, p->oro);
        printf("Fuerza: %d\n", p->fuerza);
        printf("========================================\n\n");

        /* listar armas equipadas */
        printf(BOLD "Armas equipadas (%d/3):\n" RESET, p->num_armas_equipadas);
        NodoArmaEquipada *actual_eq = p->armas_equipadas;
        NodoArmaEquipada *array_eq[MAX_ARMAS_EQUIPADAS];
        int i_eq = 0;

        if (actual_eq) {
            int i = 1;
            do {
                printf("%d. %s (Dano: %d, Usos: %d/%d)\n", i,
                       actual_eq->arma.nombre, actual_eq->arma.danio,
                       actual_eq->arma.usos, actual_eq->arma.usos_max);
                array_eq[i_eq++] = actual_eq;
                actual_eq = actual_eq->siguiente;
                i++;
            } while (actual_eq != p->armas_equipadas && i_eq < MAX_ARMAS_EQUIPADAS);
        } else {
            printf("  (sin armas equipadas)\n");
        }

        /* listar armas en mochila */
        printf("\n" BOLD "Armas en mochila (todas las que has obtenido):\n" RESET);
        NodoArmaInventario *act_inv = p->inventario_armas;
        NodoArmaInventario *array_inv[100];
        int i_inv = 0;
        int idx = 1;

        if (act_inv) {
            while (act_inv && i_inv < 100) {
                printf("%d. %s (Dano: %d, Usos max: %d)\n",
                       idx, act_inv->arma.nombre, act_inv->arma.danio,
                       act_inv->arma.usos_max);
                array_inv[i_inv++] = act_inv;
                act_inv = act_inv->siguiente;
                idx++;
            }
        } else {
            printf("  (no tienes armas en la mochila aun)\n");
        }

        if (p->tiene_amuleto) {
            if (p->amuleto_equipado) {
                printf("\nAmuleto equipado: %s\n", p->amuleto.nombre);
                printf("  Efecto: %s (+%d de dano)\n",
                       p->amuleto.efecto, p->amuleto.bonus_danio);
            } else {
                printf("\nAmuleto en mochila (NO equipado): %s\n", p->amuleto.nombre);
                printf("  Efecto: %s (+%d de dano)\n",
                       p->amuleto.efecto, p->amuleto.bonus_danio);
            }
        } else {
            printf("\nNo tienes amuleto.\n");
        }

        printf("\nOpciones:\n");
        printf("1. Equipar arma de la mochila\n");
        printf("2. Desequipar arma\n");
        printf("3. Ver arma actual\n");
        printf("4. Equipar / desequipar amuleto\n");
        printf("5. Salir del inventario\n");
        printf("Opcion: ");

        scanf("%d", &opcion);
        getchar();

        if (opcion == 1) {
            if (i_inv == 0) {
                printf("No hay armas en la mochila.\n");
                pausar();
                continue;
            }
            if (p->num_armas_equipadas >= MAX_ARMAS_EQUIPADAS) {
                printf("Ya tienes 3 armas equipadas. Desequipa una primero.\n");
                pausar();
                continue;
            }
            printf("Numero de arma de la mochila a equipar: ");
            int sel;
            scanf("%d", &sel);
            getchar();
            if (sel < 1 || sel > i_inv) {
                printf("Numero invalido.\n");
                pausar();
                continue;
            }
            insertar_arma_equipada(p, array_inv[sel - 1]->arma);
            printf(VERDE "Arma equipada desde la mochila.\n" RESET);
            pausar();

        } else if (opcion == 2) {
            if (i_eq == 0) {
                printf("No tienes armas equipadas.\n");
                pausar();
                continue;
            }
            printf("Numero de arma equipada a desequipar: ");
            int sel;
            scanf("%d", &sel);
            getchar();
            if (sel < 1 || sel > i_eq) {
                printf("Numero invalido.\n");
                pausar();
                continue;
            }
            {
                char nombre[MAX_NOMBRE];
                strcpy(nombre, array_eq[sel - 1]->arma.nombre);
                eliminar_arma_equipada(p, nombre);
                printf(AMARILLO "Arma desequipada.\n" RESET);
            }
            pausar();

        } else if (opcion == 3) {
            mostrar_arma_actual(p);
            pausar();

        } else if (opcion == 4) {
            quitar_amuleto(p);

        } else if (opcion == 5) {
            break;

        } else {
            printf("Opcion invalida.\n");
            pausar();
        }
    }
}

/* ==================== AMULETO ==================== */

void quitar_amuleto(Personaje *p) {
    if (!p->tiene_amuleto) {
        printf("No tienes ningun amuleto.\n");
        pausar();
        return;
    }

    if (p->amuleto_equipado) {
        printf("\nAmuleto actual: %s\n", p->amuleto.nombre);
        printf("Efecto: %s (+%d de dano)\n",
               p->amuleto.efecto, p->amuleto.bonus_danio);
        printf("Quieres DESEQUIPAR este amuleto? (s/n): ");

        char c;
        scanf(" %c", &c);
        getchar();

        if (c == 's' || c == 'S') {
            p->amuleto_equipado = 0;  // se guarda, pero no da bono
            printf(AMARILLO "Has desequipado tu amuleto. Sigue en tu mochila.\n" RESET);
        } else {
            printf("Mantienes el amuleto equipado.\n");
        }
    } else {
        printf("\nTienes este amuleto en tu mochila: %s\n", p->amuleto.nombre);
        printf("Efecto: %s (+%d de dano)\n",
               p->amuleto.efecto, p->amuleto.bonus_danio);
        printf("Quieres EQUIPAR este amuleto? (s/n): ");

        char c;
        scanf(" %c", &c);
        getchar();

        if (c == 's' || c == 'S') {
            p->amuleto_equipado = 1;
            printf(VERDE "Has equipado tu amuleto.\n" RESET);
        } else {
            printf("Sigues sin amuleto equipado.\n");
        }
    }

    pausar();
}

/* ==================== TIENDA: ARMAS POR NUMERO ==================== */

void mostrar_inventario_hash(TablaHashArmas *th, Personaje *p) {
    /* armar lista de armas y ordenarlas por precio */
    Arma *armas_lista[200];
    int total_armas = 0;
    int i;

    for (i = 0; i < HASH_SIZE; i++) {
        Arma *arma = th->tabla[i];
        while (arma && total_armas < 200) {
            armas_lista[total_armas++] = arma;
            arma = arma->siguiente;
        }
    }

    /* ordenamiento sencillo por precio (burbuja) */
    for (i = 0; i < total_armas - 1; i++) {
        int j;
        for (j = i + 1; j < total_armas; j++) {
            if (armas_lista[j]->precio < armas_lista[i]->precio) {
                Arma *tmp = armas_lista[i];
                armas_lista[i] = armas_lista[j];
                armas_lista[j] = tmp;
            }
        }
    }

    if (total_armas == 0) {
        printf("No hay armas en la tienda.\n");
        pausar();
        return;
    }

    int pagina = 0;

    while (1) {
        limpiar_pantalla();
        printf(BOLD "=== TIENDA DE ARMAS" RESET " (Oro: " AMARILLO "%d" RESET ")\n\n",
               p->oro);

        int inicio = pagina * 5;
        int fin = inicio + 5;
        if (inicio >= total_armas) {
            pagina = 0;
            inicio = 0;
            fin = 5;
        }

        Arma *pagina_armas[5];
        int count = 0;
        int k;

        for (k = inicio; k < fin && k < total_armas; k++) {
            Arma *arma = armas_lista[k];
            pagina_armas[count] = arma;
            printf(CYAN "%d)" RESET " %s - Costo: %d | Dano: %d | Usos: %d | Critico: %.0f%%\n",
                   count + 1, arma->nombre, arma->precio,
                   arma->danio, arma->usos_max, arma->prob_critico * 100);
            printf("   Efecto: %s\n\n", arma->efecto);
            count++;
        }

        if (count == 0) {
            printf("(No hay armas en esta pagina)\n\n");
        }

        printf("[S]iguiente pagina | [A]tras | [C]omprar | [X] salir\n");
        printf("Opcion: ");

        char opcion;
        scanf(" %c", &opcion);
        getchar();

        if (opcion == 'S' || opcion == 's') {
            pagina++;
        } else if (opcion == 'A' || opcion == 'a') {
            if (pagina > 0) pagina--;
        } else if (opcion == 'X' || opcion == 'x') {
            break;
        } else if (opcion == 'C' || opcion == 'c') {
            if (count == 0) {
                printf("No hay armas para comprar en esta pagina.\n");
                pausar();
                continue;
            }
            printf("Numero de arma a comprar (1-%d): ", count);
            int sel;
            scanf("%d", &sel);
            getchar();
            if (sel < 1 || sel > count) {
                printf("Numero invalido.\n");
                pausar();
                continue;
            }

            Arma *arma = pagina_armas[sel - 1];

            if (p->oro < arma->precio) {
                printf(ROJO "No tienes suficiente oro para comprar esta arma.\n" RESET);
                pausar();
                continue;
            }
            int ya_equipada = 0;
            if (p->armas_equipadas != NULL) {
                NodoArmaEquipada *check = p->armas_equipadas;
                do{
                    if(strcmp(check->arma.nombre, arma->nombre) == 0) {
                        ya_equipada = 1;
                        break;
                    }
                    check = check->siguiente;
                } while (check != p->armas_equipadas);
            }

            /* comprar: la agregamos al inventario y, si hay espacio, la equipamos */
            p->oro -= arma->precio;
            Arma arma_nueva = *arma;
    arma_nueva.usos = arma_nueva.usos_max; // IMPORTANTE: usos completos
    agregar_arma_inventario(p, arma_nueva);

    if (p->num_armas_equipadas < MAX_ARMAS_EQUIPADAS && !ya_equipada) {
        insertar_arma_equipada(p, arma_nueva);
        printf(VERDE "Arma comprada y equipada.\n" RESET);
    } else if (ya_equipada) {
        printf(VERDE "Arma comprada y guardada en la mochila.\n" RESET);
        printf(AMARILLO "Ya tienes %s equipada.\n" RESET, arma->nombre);
    } else {
        printf(VERDE "Arma comprada y guardada en la mochila.\n" RESET);
        printf("Ve al inventario para equiparla.\n");
    }
    pausar();
        }
    }
}

/* ==================== AUXILIAR: DANO RECURSIVO ==================== */

int calcular_danio_recursivo(int danio_base, int profundidad) {
    if (profundidad == 0) return danio_base;
    {
        int variacion = rand_entre(-1, 1);
        return calcular_danio_recursivo(danio_base + variacion, profundidad - 1);
    }
}
