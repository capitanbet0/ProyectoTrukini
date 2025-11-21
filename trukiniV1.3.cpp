#include <stdio.h>
#include <stdlib.h> //nuevo
#include <time.h> //nuevo
#include <string.h>

// === CONSTANTES Y ENUMS ===
#define NUM_CARTAS 40
#define CARTAS_POR_JUGADOR 3
#define PUNTOS_FINALES 15 // Puntos para ganar la partida, si se desea se puede cambiar por 30

// Enum para los Palos
typedef enum {
    ORO, COPA, ESPADA, BASTO, NUM_PALOS
} Palo;


//Enum para el estado del truco (v1.1)
typedef enum{
	TRUCO_NINGUNO = 0,
	TRUCO_CANTADO = 1,
	RETRUCO_CANTADO = 2,
	VALE4_CANTADO = 3
}EstadoTruco;

EstadoTruco estadoTruco = TRUCO_NINGUNO;

// ESTRUCTURAS

// Estructura de la Carta
typedef struct {
    Palo palo;
    int numero;         // 1, 2, 3, 4, 5, 6, 7, 10, 11, 12
    int valor_truco;    // Jerarquía de la carta 
    int valor_envido;   // Valor para el Envido (1-7, 0 para figuras)
} Carta;

// Estructura del Jugador
typedef struct {
    int id;
    char nombre[20];
    Carta mano[CARTAS_POR_JUGADOR];
    int puntos_partida;
} Jugador;

// ==- DECLARACIONES DE FUNCIONES -==
void inicializar_mazo(Carta mazo[]);
void asignar_valores_carta(Carta *c);
void barajar(Carta mazo[], int n);
void repartir(Carta mazo[], Jugador *j1, Jugador *j2);
void imprimir_carta(Carta c);
void imprimir_mano(Jugador *j);
int obtener_valor_envido_mano(Jugador *j);
void jugar_mano(Jugador *j1, Jugador *j2, int *puntos_truco, int *puntos_envido);
void imprimir_puntos_totales(Jugador *j1, Jugador *j2);

// --- UTILIDADES DE IMPRESION ---

const char *nombre_palo[] = {"O", "C", "E", "B"};

void imprimir_carta(Carta c) {
    // Muestra el numero real y la inicial del palo
    printf("(%d de %s)", c.numero, nombre_palo[c.palo]);
}

void imprimir_mano(Jugador *j) {

    if (strcmp(j->nombre, "CPU") == 0) {
        printf("Mano de CPU:\n");
        for (int i = 0; i < CARTAS_POR_JUGADOR; i++) {
            printf(" [%d]: (X)\n", i+1);
        }
        return;
    }

    // Mano del jugador real
    printf("Mano de %s:\n", j->nombre);
    for (int i = 0; i < CARTAS_POR_JUGADOR; i++) {
        printf(" [%d]: ", i + 1);
        imprimir_carta(j->mano[i]);
        printf(" (T:%d | E:%d)\n",
            j->mano[i].valor_truco,
            j->mano[i].valor_envido
        );
    }
}

void imprimir_puntos_totales(Jugador *j1, Jugador *j2) {
    printf("\n--- PUNTUACION DE LA PARTIDA ---\n");
    printf("+----------------------+----------------------+\n");
    printf("| %-20s | %-20s |\n", j1->nombre, j2->nombre);
    printf("+----------------------+----------------------+\n");
    printf("| %-20d | %-20d |\n", j1->puntos_partida, j2->puntos_partida);
    printf("+----------------------+----------------------+\n");
}

// ==- LOGICA DE CARTAS -==

// Asigna los valores de Truco y Envido a una carta
void asignar_valores_carta(Carta *c) {
    // 1. Asignar valor de ENVIDO
    if (c->numero >= 10) { // 10, 11, 12 (figuras) valen 0
        c->valor_envido = 0;
    } else {
        c->valor_envido = c->numero;
    }

    // 2. Asignar valor de TRUCO (Jerarquia)
    int num = c->numero;
    Palo p = c->palo;

    if (num == 1 && p == ESPADA) c->valor_truco = 14; // 1 E (Macho)
    else if (num == 1 && p == BASTO) c->valor_truco = 13; // 1 B (hembra)
    else if (num == 7 && p == ESPADA) c->valor_truco = 12; // 7 E
    else if (num == 7 && p == ORO) c->valor_truco = 11; // 7 O
    else if (num == 3) c->valor_truco = 10;
    else if (num == 2) c->valor_truco = 9;
    else if (num == 1) c->valor_truco = 8; // 1 de Copa y Oro
    else if (num == 12) c->valor_truco = 7; // Reyes
    else if (num == 11) c->valor_truco = 6; // Caballos
    else if (num == 10) c->valor_truco = 5; // Sotas
    else if (num == 7) c->valor_truco = 4; // 7 de Copa y Basto
    else if (num == 6) c->valor_truco = 3; //6
    else if (num == 5) c->valor_truco = 2;//5
    else if (num == 4) c->valor_truco = 1; //4
}

// Inicializa las 40 cartas del mazo
void inicializar_mazo(Carta mazo[]) {
    int indice = 0;
    for (int p = ORO; p < NUM_PALOS; p++) {
        for (int n = 1; n <= 12; n++) {
            // Se saltan los números 8 y 9
            if (n != 8 && n != 9) {
                mazo[indice].palo = (Palo)p;
                mazo[indice].numero = n;
                asignar_valores_carta(&mazo[indice]);
                indice++;
            }
        }
    }
}

// ==- LOGICA DE REPARTO Y MEZCLA -==

// Algoritmo de Fisher-Yates para barajar, aqui hago un cambio para que solo baraje y no siembre. Porque puede producir malas mezclas
void barajar(Carta mazo[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Carta temp = mazo[i];
        mazo[i] = mazo[j];
        mazo[j] = temp;
    }
}

// Reparte las primeras 6 cartas del mazo a los jugadores
void repartir(Carta mazo[], Jugador *j1, Jugador *j2) {
    int idx = 0;
    for (int i = 0; i < CARTAS_POR_JUGADOR; i++) {
        j1->mano[i] = mazo[idx++]; // carta para j1
        j2->mano[i] = mazo[idx++]; // carta para j2
    }
}

//  LOGICA DE JUEGO (ENVIDO) 

// Calcula el puntaje de Envido de una mano
int obtener_valor_envido_mano(Jugador *j) {
    int flor = 0;
    int envido = 0;

    // Detectar Flor: las 3 cartas del mismo palo
    if (j->mano[0].palo == j->mano[1].palo &&
        j->mano[0].palo == j->mano[2].palo) {

        flor = 20 +
               j->mano[0].valor_envido +
               j->mano[1].valor_envido +
               j->mano[2].valor_envido;

        return flor;  // Flor tiene prioridad
    }

    // Si no hay Flor, calcular Envido normal
    int max_pareja = 0;

    for (int i = 0; i < 3; i++) {
        for (int k = i + 1; k < 3; k++) {
            if (j->mano[i].palo == j->mano[k].palo) {
                int envido_pareja =
                    20 + j->mano[i].valor_envido + j->mano[k].valor_envido;

                if (envido_pareja > max_pareja)
                    max_pareja = envido_pareja;
            }
        }
    }

    // si no hubo pareja, el envido es la carta más alta
    if (max_pareja == 0) {
        int max_carta = 0;
        for (int i = 0; i < 3; i++)
            if (j->mano[i].valor_envido > max_carta)
                max_carta = j->mano[i].valor_envido;

        return max_carta;
    }

    return max_pareja;
}

// ==- LOGICA DE JUEGO (TRUCO) -==

// Determina el ganador de una baza (una carta contra otra)
// Devuelve 1 si gana j1, 2 si gana j2, 0 si es empate.
int determinar_ganador_baza(Carta c1, Carta c2) {
    if (c1.valor_truco > c2.valor_truco) {
        return 1;
    } else if (c2.valor_truco > c1.valor_truco) {
        return 2;
    } else {
        return 0; // Empate
    }
}

//MEJORA V1.3: JUGAR BAZAS DE MANERA MANUAL VS. LA CPU:

void eliminar_carta(Carta mano[], int index){
	for(int i = index; i < CARTAS_POR_JUGADOR - 1; i++){
		mano[i] = mano[i + 1];
	}
}

// Bucle principal de la mano V1.3: (((MANUAL)))
void jugar_mano(Jugador *j1, Jugador *j2, int *puntos_truco, int *puntos_envido) {
	int cartas_restantes = 3;
    int bazas_j1 = 0;
    int bazas_j2 = 0;
    int primera_ganada = 0;
    
    while (cartas_restantes > 0){
    	
    	printf("\n=== BAZA %d ===\n", 4 - cartas_restantes);
    	
    	//eleccion jugadro
    	printf("\nTu mano:\n");
    	for(int i = 0; i < cartas_restantes; i++){
    		printf("[%d] ", i+1);
    		imprimir_carta(j1->mano[i]);
    		printf("\n");
    	}
    	
    	int eleccion;
    	do{
    		printf("Elegi una carta (1-%d): ", cartas_restantes);
    		scanf ("%d", &eleccion);
    		getchar();
    	} while (eleccion < 1 || eleccion > cartas_restantes);
    	
    	Carta carta_j1 = j1->mano[eleccion - 1];
    	eliminar_carta(j1->mano, eleccion -1);
    	
    	//Como primera medida voy a hacer que la CPU juegue las cartas al azar, despues veo como configurarla para que sea inteligente.
    	
    	int idx_cpu = rand() % cartas_restantes;
    	Carta carta_j2 = j2->mano[idx_cpu];
    	eliminar_carta(j2->mano, idx_cpu);
    	
    	cartas_restantes--;
    	
    	printf("\n%s juega ", j1->nombre); imprimir_carta(carta_j1);
    	printf(" vs CPU juega ");
    	imprimir_carta(carta_j2);
    	printf("\n");
    	
    	int ganador = determinar_ganador_baza(carta_j1, carta_j2);
    	
    	if (ganador == 1){
    		bazas_j1++;
    		printf("Ganaste la baza.\n");
    		if(primera_ganada == 0) primera_ganada = 1;
    	} else if (ganador == 2){
    		bazas_j2++;
    		printf("La CPU gana la baza.\n");
    		if(primera_ganada == 0) primera_ganada = 2;
    	} else {
    		printf("Baza empatada.\n");
    	}
    	
    	if(bazas_j1 == 2 || bazas_j2 == 2) break;
    }
    
    // ==- RESOLUCION DEL TRUCO -==
    printf("\n*** RESOLUCION DEL TRUCO ***\n");
    
    if (bazas_j1 > bazas_j2){
    	printf("%s gana la mano y suma %d puntos.\n", j1->nombre, *puntos_truco);
    	j1->puntos_partida += *puntos_truco;
    }else if (bazas_j2 > bazas_j1){
    	printf("La CPU gana la mano y suma %d puntos.\n", *puntos_truco);
    	j2->puntos_partida += *puntos_truco;
    }else{
    	//Empate -> gana la primera baza
    	if(primera_ganada == 1){
    		printf("Empate. Ganas por la primera baza. +%d puntos.\n", *puntos_truco);
    		j1->puntos_partida += *puntos_truco;
    	} else {
    		printf("Empate. Gana CPU por primera baza. +%d puntos.\n", *puntos_truco);
    		j2->puntos_partida += *puntos_truco;
    	}
    }
    
}

//variable global para alternar manos entre la CPU y el jugadror
int mano = 1;

//MECANICA V1.3 de cantos de TRUCO

int cantar_truco(Jugador *j1, Jugador *j2) {

    int turno = mano;  // 1 = jugador, 2 = CPU

    // Si la CPU es mano -> puede cantar Truco antes del jugador.
    // Si el jugador es mano -> primero se ofrece Truco al jugador.

    int opcion;

    if (turno == 2) {
        // CPU decide si cantar Truco
        int prob = rand() % 100;
        if (prob < 40) {
            printf("\nCPU: Le canto TRUCO COMPA!\n");
            estadoTruco = TRUCO_CANTADO;

            printf("\nOpciones:\n");
            printf("1) Quiero\n");
            printf("2) No quiero\n");
            printf("3) Quiero RETRUCO\n");
            printf("Elegi: ");
            scanf("%d", &opcion); getchar();

            // JUGADOR RESPONDE
            if (opcion == 1) return 2; // quiero ? vale 2
            if (opcion == 2) return -1; // no quiero ? CPU gana 1
            if (opcion == 3) {
                estadoTruco = RETRUCO_CANTADO;
                printf("\nCPU pensando...\n");

                int prob2 = rand() % 100;

                if (prob2 < 50) {
                    printf("CPU: Quiero RETRUCO!\n");
                    return 3;
                } else if (prob2 < 80) {
                    printf("CPU: No quiero.\n");
                    return -1;
                } else {
                    printf("CPU: QUIERO VALE 4!!!\n");
                    estadoTruco = VALE4_CANTADO;

                    printf("\nOpciones:\n1) Quiero\n2) No quiero\n");
                    printf("Elegi: ");
                    scanf("%d", &opcion); getchar();

                    if (opcion == 1) return 4;
                    else return -1;
                }
            }
        }
    }

    // OFRECER AL JUGADOR CANTAR TRUCO
    printf("\nQueres cantar TRUCO?\n1) No\n2) Truco\nElegi: ");
    scanf("%d", &opcion); getchar();

    if (opcion == 1) return 1; // vale 1

    // JUGADOR CANTA TRUCO
    printf("\n%s: LE CANTO TRUCO!\n", j1->nombre);
    estadoTruco = TRUCO_CANTADO;

    // CPU RESPONDE
    int prob = rand() % 100;

    if (prob < 50) {
        printf("CPU: Quiero.\n");
        return 2;
    }
    else if (prob < 80) {
        printf("CPU: No quiero.\n");
        return -1;
    }
    else {
        printf("CPU: QUIERO RETRUCO!\n");
        estadoTruco = RETRUCO_CANTADO;

        printf("\nOpciones:\n1) Quiero\n2) No quiero\n3) Quiero VALE!!!!! 4\n");
        printf("Elige: ");
        scanf("%d", &opcion); getchar();

        if (opcion == 1) return 3;     // quiero retruco ? vale 3
        if (opcion == 2) return -1;    // no quiero ? CPU gana 2

        // jugador quiere VALE 4
        estadoTruco = VALE4_CANTADO;
        printf("\nCPU pensando...\n");

        int prob2 = rand() % 100;

        if (prob2 < 60) {
            printf("CPU: No me le voy a achicar, QUIERO!!!\n");
            return 4;
        } else {
            printf("CPU: No quiero.\n");
            return -1;
        }
    }

    return 1;
}


// --- FUNCION PRINCIPAL ---

int main() {
    Carta mazo[NUM_CARTAS];
    Jugador jugador1 = {1, "", {}, 0};
    Jugador jugador2 = {2, "CPU", {}, 0};
    
    //Pedir nombre al jugador
    printf("Ingresa tu nombre: ");
    fgets(jugador1.nombre, 20, stdin);
    jugador1.nombre[strcspn(jugador1.nombre, "\n")] = '\0'; //sacar salto de linea
    
    // Inicialización
    inicializar_mazo(mazo);
    srand((unsigned) time(NULL)); //<<--- aca movi el srand, para sembrar una sola vez
    printf("¡Trukini iniciado!\n");
    printf("Objetivo: %d puntos.\n\n", PUNTOS_FINALES);
    
    // Bucle principal del juego
    while (jugador1.puntos_partida < PUNTOS_FINALES && jugador2.puntos_partida < PUNTOS_FINALES) {
        
        //LIMPIAR CONSOLA
        
        #ifdef _WIN32
              system("cls");
              #else
              system("clear");
        #endif
        
        printf("===========================================\n");
        printf("              NUEVA RONDA\n");
        printf("===========================================\n");
        
        barajar(mazo, NUM_CARTAS);
        repartir(mazo, &jugador1, &jugador2);
           
      // Alternar mano
        if (mano == 1){
            mano = 2;
		} else {
		    mano = 1;
		}
        printf("La mano es de: %s\n", (mano == 1 ? jugador1.nombre : jugador2.nombre));

        
        // Puntuacion de la ronda actual
        int puntos_truco_ronda = 1; // La mano vale 1 punto (o mas si se canta Truco)
        int puntos_envido_ronda = 2; // Envido vale 2 si se acepta
        
        // ==- 1. MOSTRAR MANO -==
        imprimir_mano(&jugador1);
        printf("\n");
        imprimir_mano(&jugador2);
        printf("\n");

        // === 2. FASE DE APUESTAS (Envido y Truco) ===

        // Camtar Envido
        int opcion;
        int puntos_envido = 0;
        int estadoEnvido = 0;
        int envido_aceptado = 0; //Fix bug de la finalizacion de ronda en el envido y suma de puntos
        
        printf("\nDeseas cantar envido?\n");
        
        //CPU canta ENVIDO
        
        if(mano == 2){
        	int envido_cpu = obtener_valor_envido_mano(&jugador2);
        	int prob_envido = 0;
        	
        	if (envido_cpu >= 31) prob_envido = 80;
        	else if (envido_cpu >= 27) prob_envido = 55;
        	else if (envido_cpu >= 23) prob_envido = 35;
        	else prob_envido = 10;
        	
        	int decision = rand() % 100;
        	
        	if (decision < prob_envido){
        		printf("\nCPU: ENVIDO!\n");
        		estadoEnvido = 1;
        		puntos_envido = 2;
        		
        		int aceptar;
        		printf ("Aceptas? (1=SI, 2=NO): ");
        		scanf("%d", &aceptar);
        		getchar();
        		
        		if (aceptar == 1){
        			envido_aceptado = 1;
        		} else {
        			printf ("No queres. CPU suma 1 punto.\n");
        			jugador2.puntos_partida += 1;
        		}
        	}
        }
        
        
        /*
		Version 1.2:
		printf("1) No\n");
        printf("2) Envido\n"); 
        printf("3) Real Envido\n");
        printf("4) Falta Envido\n");
        */
        
        
        //V1.3:
        
        printf("\nOpciones:\n");
        printf("1) No\n");
        
        if (mano == 1){
        	printf("2) Envido\n");
        	printf("3) Real Envido\n");
        	printf("4) Falta Envido\n");
        } else {
        	printf("(No podes cantar envido primero porque NO sos mano)\n");
        }
        printf("Elige: ");
        scanf("%d", &opcion);
        getchar();
        //agregar el if anidado de envido,envido,real envido, falta envido
        
        if(opcion == 2){
        	printf("\n%s dice: le canto Envido compa!.\n", jugador1.nombre);
        	estadoEnvido = 1;
        	puntos_envido = 2;
        	
        	int prob = rand() % 100;
        	if (prob < 70){
        		printf("El rival dice: Quiero!\n");
        		envido_aceptado = 1;
        	} else {
        		printf("El rival dice: NO quiero...\n");
        	    jugador1.puntos_partida +=1;
				printf("%s suma 1 punto.\n", jugador1.nombre); 
				printf("\nPresiona ENTER para la siguiente ronda...\n");
				getchar();
        	}
        } else if (opcion == 3){
        	printf("\nREAL ENVIDO hermano!\n");
        	estadoEnvido = 3;
        	puntos_envido = 3;
        	
        	int prob = rand() % 100;
        	if(prob < 50){
        		printf("El rival dice: QUIERO!!\n");
        		envido_aceptado = 1;
        	} else {
        		printf("El rival dice: NO quiero...\n");
        		jugador1.puntos_partida += 1; //Real envido directo no querido = 1
        		printf("%s suma 1 punto.\n", jugador1.nombre);
        		printf("\nPresiona ENTER para la siguiente ronda...\n");
        		getchar();
        		
        	}
        } else if (opcion == 4){
        	printf("\nAchica que te canto la FALTA ENVIDO!!\n");
			estadoEnvido = 4;
			
			int prob = rand() % 100;
			if (prob < 45){
				printf("El rival dice: Oa TATA, QUIERO que p*ta no voy a querer!!\n");
				envido_aceptado = 1;
				puntos_envido = 30 - jugador1.puntos_partida;
			} else {
				printf("El rival dice: me achico nomas... NO quiero!\n");
				jugador1.puntos_partida += 1;
				printf("%s suma 1 punto. \n", jugador1.nombre);
				printf("\nPresiona ENTER para la siguiente ronda...\n");
				getchar();
			}	
        }
        
        if (envido_aceptado == 1){
        	int puntaje_j1 = obtener_valor_envido_mano(&jugador1);
        	int puntaje_j2 = obtener_valor_envido_mano(&jugador2);
        	
        	printf("\nTu envido es: %d\n", puntaje_j1);
        	printf("Envido del rival: %d\n", puntaje_j2);
        	
        	if (puntaje_j1 > puntaje_j2){
        		jugador1.puntos_partida += puntos_envido;
        		printf("\nGANASTE el Envido! +%d puntos.\n", puntos_envido);
        	} else if (puntaje_j2 > puntaje_j1){
        		jugador2.puntos_partida += puntos_envido;
        		printf("\n La CPU gano el Envido, gana +%d puntos.\n", puntos_envido);
        	}else {
        		if (mano == 1){
        			jugador1.puntos_partida += puntos_envido;
        			printf("Empate de envido. Gana %s por ser mano.\n", jugador1.nombre);
        		} else {
        			jugador2.puntos_partida += puntos_envido;
        			printf("Empate de envido. Gana %s por ser mano.\n", jugador2.nombre);
        		}
        	}
        	printf("\nPresiona ENTER para continuar...\n");
        	getchar();
        }
        
        //============= OPCION DE CANTAR TRUCO =============
        int valor_truco = cantar_truco(&jugador1, &jugador2);
        
        if(valor_truco == -1){
        	if(estadoTruco == TRUCO_CANTADO){
        		if(mano == 1) jugador2.puntos_partida +=1;
        		else jugador1.puntos_partida +=1;
        	}
        	estadoTruco = TRUCO_NINGUNO;
        	continue;
        }
        
        puntos_truco_ronda = valor_truco;
             
        // --- 3. FASE DE JUEGO DE CARTAS ---
        jugar_mano(&jugador1, &jugador2, &puntos_truco_ronda, &puntos_envido_ronda);
        
        // -_-_- 4. MOSTRAR PUNTUACIÓN -_-_-
        imprimir_puntos_totales(&jugador1, &jugador2);
        
        printf("\nPresiona ENTER para la siguiente ronda...\n");
        // Limpiar el buffer de entrada para esperar un ENTER
        fflush(stdout);
        getchar();
      
    }
    
    // ==- 5. RESULTADO FINAL -==
    printf("\n\n###########################################\n");
    if (jugador1.puntos_partida >= PUNTOS_FINALES) {
        printf("GANADOR DE LA PARTIDA: %s !!!\n", jugador1.nombre);
    } else {
        printf("GANADOR DE LA PARTIDA: %s !!!\n", jugador2.nombre);
    }
    printf("###########################################\n");

    return 0;
}









