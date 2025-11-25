#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum {
    EFECTO_NINGUNO,
    EFECTO_VENENO,
    EFECTO_PARALISIS,
    EFECTO_DRENA_VIDA
} TipoEfectoArma;

// Tipos de amuletos
typedef enum {
    AMU_NINGUNO,
    AMU_MAS_DANIO,
    AMU_MAS_VIDA,
    AMU_REGENERACION,
    AMU_PRIMER_GOLPE,
    AMU_MAS_CRITICO
} TipoAmuleto;

//ESTRUCTURAS PRINCIPALES
//ESTRUCTURAS PRINCIPALES
//ESTRUCTURAS PRINCIPALES

// Estructura para armas
typedef struct{
    char nombre[30];        // Nombre del arma
    int danoBase;          // Daño base
    float probCritico;      // Probabilidad de crítico (0.0 a 1.0)
    int usosMaximos;        // Cuántos usos tiene como máximo
    int usosRestantes;      // Cuántos usos le quedan
    TipoEfectoArma efecto;  // Efecto especial del arma
} arma;

// Estructura para amuletos
typedef struct {
    char nombre[30];   // Nombre del amuleto
    TipoAmuleto tipo;  // Tipo de efecto
    int valor;         // Intensidad del efecto (por ejemplo +10 vida, +5 daño, etc.)
} amuleto;

// Estructura para el personaje
typedef struct {
    char nombre[30];
    int vidaMax;
    int vidaActual;
    int fuerza;
    int velocidad;
    int pisoActual;
    int oro;
    amuleto *amuletoEquipado;  // Apunta al amuleto que trae puesto (o NULL)
} Personaje;

//PROTOTIPOS
//PROTOTIPOS
//PROTOTIPOS

Personaje crearPersonajeInicial(void);
void mostrarPersonaje(Personaje p);

//MAIN
int main(void) {
    Personaje jugador;
    jugador = crearPersonajeInicial(); //se crea el personaje
    
    mostrarPersonaje(jugador); //se muestra el personaje


    return 0;
}



Personaje crearPersonajeInicial(void){//Crea un personaje con valores iniciales
    Personaje p;
    char nombre[30];
    printf("Nombre del personaje: "); scanf(" %s", nombre);

    strcpy(p.nombre, nombre);

    p.vidaMax     = 100;
    p.vidaActual  = 100;
    p.fuerza      = 10;
    p.velocidad   = 5;
    p.pisoActual  = 1;
    p.oro         = 0;
    p.amuletoEquipado = NULL;  // Todavía no tiene amuleto

    return p;
}

//Muestra en pantalla la información básica del personaje
void mostrarPersonaje(Personaje p){
    printf("\n");
    printf("===== DATOS DEL PERSONAJE =====\n");
    printf("Nombre:      %s\n", p.nombre);
    printf("Vida:        %d / %d\n", p.vidaActual, p.vidaMax);
    printf("Fuerza:      %d\n", p.fuerza);
    printf("Velocidad:   %d\n", p.velocidad);
    printf("Piso actual: %d\n", p.pisoActual);
    printf("Oro:         %d\n", p.oro);

    if (p.amuletoEquipado == NULL) {
        printf("Amuleto:     Ninguno\n");
    } else {
        printf("Amuleto:     %s\n", p.amuletoEquipado->nombre);
    }

    printf("===============================\n");
}
