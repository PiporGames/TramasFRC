//============================================================================
// ----------- PRACTICAS DE FUNDAMENTOS DE REDES DE COMUNICACIONES -----------
// ---------------------------- CURSO 2023/24 --------------------------------
// ----------------------------- SESION2.CPP ---------------------------------
//============================================================================
// Autores: Manuel Alonso González
//         José Manuel de Torres Domínguez
// Grupo: 2

#include <stdio.h>
#include <stdio_ext.h>
#include <iostream>
#include "linkLayer.h"
#include "UserInterface.h"

using namespace std;

int main() {
    // El usuario escoge la interfaz
    interface_t iface = EscogerInterfaz();
    // Abrimos el puerto
    // TODO: Aquí abrimos el adaptador antes de asignarlo a un GT.
    //       La apertura de adaptadores debería hacerse, idealmente, dentro del GT.
    //       Esto es un fallo estructural en términos de gestión y protección de objetos,
    //       pero realmente ahora mismo no nos importa para este proyecto.
    int Puerto = OpenAdapter(&iface);

    if (Puerto != 0) {
        printf("Error al abrir el puerto\n");
        getch();
        return (1);
    } else printf("Puerto abierto correctamente\n");
    __fpurge(stdin);

    // A través de esa interfaz, se elije el grupo en el que se encuentra
    printf("Introduzca el número de grupo:\n");
    int grupo;
    scanf("%d", &grupo);

    // Preguntamos si se trata del maestro o el esclavo
    printf("Selección del modo de la estación:\n\t[1] Modo Maestra\n\t[2] Modo Esclava\n");
    int modo;
    scanf("%d", &modo);

    unsigned char mac_dst[6];
    unsigned char type[2] = {(unsigned char)(48 + grupo), 0x00};

    //Creamos el objeto de GestionTramas
    GestionTramas gt(&iface, mac_dst, type, grupo);

    if (modo == 1) {
        // Manda el mensaje y espera a la otra estación
        printf("Esperando que se una la estación esclava\n");
        rolMaestro(gt);
    } else {
        // Recibe el mensaje y espera a la otra estación
        printf("Esperando que se una la estación maestra\n");
        rolEsclavo(gt);
    }

    // Bucle menú principal
    mostrarMenu(modo);

    unsigned char car = 0;
    unsigned char esc = 27;
    char inf[255];

    while (car != esc) {
        if (modo == 2) { // Si se trata del esclavo...
            int size = gt.RecibirInformacion(inf);
            if (size > 0) { // Procesar información
                printf("\nRecibido: ");
                for (int i = 0; i < size; i++)
                    printf("%c", inf[i]);

                printf("\nTamaño trama: %d Bytes\n", size);
            }
        }

        if (kbhit()) { // Si se ha pulsado una tecla...
            
            car = getch(); // Recibir letra
            if (car == esc) { // Si la letra es ESC...
                
                if (kbhit()) { // Comprobamos si se ha pulsado una tecla...
                    
                    car = getch(); // Recibimos la letra
                    if (car = 'O') { // Si la letra pulsada (luego del ESC) fue O
                        
                        if (kbhit()) { // Comprobamos si se ha pulsado una tecla...
                            car = getch(); // Recibimos la letra
                            
                            switch (car) { // Si la tecla fue...

                                case 'P': // P, entonces ESC + O + P = F1
                                    printf("\n");
                                    gt.EstConexCarac();
                                    mostrarMenu(modo);
                                    break;
                                
                                case 'Q': // Q, entonces ESC + O + Q = F2
                                    if (modo == 1) {
                                        printf("Enviando fichero...\n");
                                        gt.EnviarFichTxt();
                                        mostrarMenu(modo);
                                    }
                                    break;
                                
                                case 'R': // R, entonces ESC + O + R = F3
                                    bool maestro;
                                    maestro = (modo == 1);
                                    if (!maestro) { printf("Estás en modo esclavo");}
                                    gt.ProtParoEspera(maestro);
                                    mostrarMenu(modo);
                                    break;

                                default:
                                    break;
                            
                            }
                        }
                    }
                }
            }
        }
    }

    // Cerramos el puerto:
    // TODO: De nuevo. La gestión debería de realizarse dentro del GT.
    //       Sin embargo, no es importante ahora.
    CloseAdapter(gt.getInterface());

    printf("Puerto cerrado\n");

    return 0;
}
