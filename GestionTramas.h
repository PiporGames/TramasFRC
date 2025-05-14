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
#include <fstream>
#include "linkLayer.h"

using namespace std;

enum DIRECCION : unsigned char {
    SELECCION = 'R',
    SONDEO = 'T'
};

enum CONTROL : unsigned char {
    ACK = 0x6,
    NACK = 0x15,
    ENQ = 0x5,
    EOT = 0x4,
    STX = 0x2,
    NUL = 0x0 // CONTROL NUL reservado para control de tramas
};

class GestionTramas {
    private:

        interface_t *iface; //Interfaz de red (incluye la MAC maestro/src)
        unsigned char *mac_dst; //MAC destino
        unsigned char *protocolCode; //Código del protocolo
        int grupo; //Número del grupo
        
        // Modo de dirección para el modo F3.
        //Si no se especifica en el constructor, se establecerá por defecto a SONDEO.
        DIRECCION modo_direccion;


        // -- PRÁCTICA 4 -- //
        
        // PRE: DIRECCION::direccion (tipo DIRECCION, e/s), control (tipo CONTROL, e/s), num_trama (número trama, e/s), longitud (longitud de los datos, e/s), char * datos reservado con 254 bytes de memoria
        // DESC: Función interna que sirve como herramienta para simplificar el proceso de recibir una trama.
        // POST: direccion, control, num_trama, longitud y datos por medio de los parámetros entrada/salida.
        //       Si la trama tiene un control distinto a CONTROL::STX (0x2), "datos" no se modificará.
        //          Asimismo, si se recibe "nullptr" como parámetro de entrada en "datos", tampoco se guardarán los datos.
        //       Si no se recibe una trama (el buffer está vacío/no se ha enviado ninguna trama) control pasará a valer CONTROL::NUL y el resto de parámetros no se modificarán.
        //          Asimismo, si internamente existe una disparidad en el cálculo del BCE, control tambien pasará a valer CONTROL::NUL y el resto de parámetros no se modificarán.
        void RecibirTrama(DIRECCION & direccion, CONTROL &control, unsigned char & num_trama, unsigned char &longitud, unsigned char * datos);
        // PRE: direccion (tipo DIRECCION), control (tipo CONTROL), num_trama (número de la trama), longitud (longitud de los datos), char * datos con hasta 254 bytes como máximo, err (Introducir error)
        // DESC: Función interna que sirve como herramienta para simplificar el proceso de enviar una trama.
        //       Si se pasa por parámetro un control distinto a CONTROL::STX (0x2), el campo de "longitud" y "datos" no se leerá.
        //              En tal caso, se recomienda asignar "0" y "nullptr" al campo de "longitud" y "datos", respectivamente, en la llamada al método.
        //       Si se desea emular un error, debes pasar "true" en el campo "err", o "false" en caso de no querer emularlo.
        void EnviarTrama(DIRECCION direccion, CONTROL control, unsigned char num_trama, unsigned char longitud, unsigned char * datos, bool err);
        
        // Método que contiene la lógica del envío de un fichero, siguiendo las reglas PyP.
        void EnviarFichPyP(bool maestro);

        // Método que contiene la lógica de la recepción de un fichero, siguiendo las reglas PyP.
        void RecibirFichPyP(bool maestro);

    public:
    
        // Constructores
        GestionTramas(interface_t *_iface, unsigned char *_mac_dst, unsigned char *_protocolCode, int _grupo);
        GestionTramas(interface_t *_iface, unsigned char *_mac_dst, unsigned char *_protocolCode, int _grupo, DIRECCION _direccion);


        // Setters && Getters
        // Devuelve la intefaz del GT
        interface_t* getInterface();
        // Devuelve el puntero de la MacDst. Nótese que esto permite al receptor modificar la MAC.
        unsigned char * getMacDst();
        // Establece un nuevo modo de dirección
        void setDireccion(DIRECCION dir);



        // Métodos
        // Envía un carácter al destino.
        void EnviarCaracter(unsigned char car);

        // Envía la señal para detectar al esclavo.
        void EnviarSignalMaestro();

        // Devuelve 0 si no es el mensaje del esclavo.
        int RecibirMaestro(unsigned char * mac_recibida);

        // Envía la señal para notificar al maestro.
        void EnviarSignalEsclavo();

        // Devuelve la MAC del maestro por parámetro.
        int RecibirEsclavo(unsigned char * mac_recibida);

        // Recibe un carácter en un modo que no sea el F3.
        // Devuelve el carácter leido.
        char RecibirCaracter(int &size);
        
        // Método para recibir cualquier tipo de información en un modo que no sea el F3.
        // Devuelve 1 si se ha recibido algo, 0 si no.
        int RecibirInformacion(char *vec);
        
        // Método para gestionar el envío de caracteres interactivo.
        void EstConexCarac();

        // Envía un fichero.
        void EnviarFichTxt();


        //-- PRÁCTICA 4 --//

        // Método de control del protocolo paro y espera.
        void ProtParoEspera(bool maestro);
        
        // PRE: char * datos con la cadena de < 255 carácteres a procesar, unsigned char longitud (de la cadena).
        // DESC: Calcula el BCE de la cadena dada.
        // POST: El valor del BCE.
        static unsigned char CalcularBCE(unsigned char * datos, unsigned char longitud);


        // RECIBIR: Recibir una trama guardada en el buffer, devolviendo el número de la trama recibido si la trama recibida no está vacía.
        //          Por lo tanto, es conveniente hacer la comprobación del número de la trama en código externo.
        // ENVIAR: Envía un frame con los datos necesarios de cada estructura a la MAC guardada en GestionTramas.
        //         En todas se deberá introducir el número de la trama (La dirección se hereda de GestionTramas, el control es implícito al método).
        //         En STX, además, se debe de introducir la longitud de los datos y los datos (El BCE se calcula solo).
        unsigned char RecibirACK();
        void EnviarACK(unsigned char num_trama); // ACKNOWLEDGMENT
        
        unsigned char RecibirNACK();
        void EnviarNACK(unsigned char num_trama); // NOT ACKNOWLEDGMENT

        // Además, RecibirENQ devuelve el tipo de direccion como parámetro e/s.
        unsigned char RecibirENQ(DIRECCION &direccion);
        void EnviarENQ(unsigned char num_trama); // PETICIÓN DE SONDEO

        unsigned char RecibirEOT();
        void EnviarEOT(unsigned char num_trama); // LIBERACIÓN

        // Además, RecibirSTX devuelve los datos y su longitud como parámetro e/s.        
        unsigned char RecibirSTX(unsigned char &longitud, unsigned char *datos);
        void EnviarSTX(unsigned char num_trama, unsigned char longitud, unsigned char *datos, bool err); // PAQUETE DE DATOS

 
        // Destructores
};