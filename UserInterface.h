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
#include "GestionTramas.h"

using namespace std;

// Devuelve la interfaz escogida por el usuario
interface_t EscogerInterfaz();

// Actualiza el objeto GestionTramas con la MAC del esclavo, en caso de recibirla.
void rolMaestro(GestionTramas& gt);

// Actualiza el objeto GestionTramas con la MAC del maestro, en caso de recibirla.
void rolEsclavo(GestionTramas& gt);

void mostrarMenu(int modo);
