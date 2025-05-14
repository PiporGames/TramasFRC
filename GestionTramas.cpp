//============================================================================
// ----------- PRACTICAS DE FUNDAMENTOS DE REDES DE COMUNICACIONES -----------
// ---------------------------- CURSO 2023/24 --------------------------------
// ----------------------------- SESION2.CPP ---------------------------------
//============================================================================
// Autores: Manuel Alonso González
//         José Manuel de Torres Domínguez
// Grupo: 2

#include "GestionTramas.h"


GestionTramas::GestionTramas(interface_t *_iface, unsigned char *_mac_dst, unsigned char *_protocolCode, int _grupo) {
    iface = _iface;
    mac_dst = _mac_dst;
    protocolCode = _protocolCode;
    grupo = _grupo;
    modo_direccion = DIRECCION::SELECCION;
}
GestionTramas::GestionTramas(interface_t *_iface, unsigned char *_mac_dst, unsigned char *_protocolCode, int _grupo, DIRECCION _direccion) {
    iface = _iface;
    mac_dst = _mac_dst;
    protocolCode = _protocolCode;
    grupo = _grupo;
    modo_direccion = _direccion;
}
interface_t * GestionTramas::getInterface(){
    return iface;
}
unsigned char * GestionTramas::getMacDst(){
    return mac_dst;
}
void GestionTramas::setDireccion(DIRECCION dir) {
    modo_direccion = dir;
}


void GestionTramas::EnviarCaracter(unsigned char car)
{
  // Construir trama
  unsigned char inf[1] = {car};
  unsigned char *frame = BuildFrame(iface->MACaddr, mac_dst, protocolCode, inf);

  // Enviar trama
  SendFrame(iface, frame, sizeof(inf));

  // Liberar memoria
  free(frame);
}

void GestionTramas::EnviarSignalMaestro()
{
  // Formamos una parte del tipo
  int aux = grupo + 48;
  // Elegimos la MAC aporpiada para BroadCast
  unsigned char mac_dst[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  // Establecemos el tipo correcto
  unsigned char type[2] = {(unsigned char)aux, 0x01};
  // Construimos la trama
  unsigned char *frame = BuildHeader(iface->MACaddr, mac_dst, type);

  // Enviar trama
  SendFrame(iface, frame, 0);

  // Liberar memoria
  free(frame);
}

int GestionTramas::RecibirMaestro(unsigned char * mac_recibida)
{
  // Obtenemos el paquete de la trama
  apacket_t frame = ReceiveFrame(iface);
  const unsigned char *packet = frame.packet;

  // Comprobamos que tenga datos
  if (packet != nullptr)
  {
    // Calculamos el offset del tipo
    packet += sizeof(iface->MACaddr) * 2;
    // Comprobamos que el tipo sea el adecuado

    unsigned char type[2] = {(unsigned char)(48 + grupo), 0x02};

    if (packet[0] == grupo + 48 && packet[1] == 0x02)
    {
      // Devolver mac_dst del maestro
      packet -= sizeof(iface->MACaddr);
      for (int j = 0; j < sizeof(iface->MACaddr); j++)
      {
        mac_recibida[j] = packet[j];
        mac_dst[j] = packet[j];
      }
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

void GestionTramas::EnviarSignalEsclavo()
{
  // Formamos una parte del tipo
  int aux = grupo + 48;
  // Establecemos el tipo correcto
  unsigned char type[2] = {(unsigned char)aux, 0x02};

  // Construimos la trama
  unsigned char *frame = BuildHeader(iface->MACaddr, mac_dst, type);

  // Enviar trama
  SendFrame(iface, frame, 0);

  // Liberar memoria
  free(frame);
}

int GestionTramas::RecibirEsclavo(unsigned char * mac_recibida)
{
  // Obtenemos el paquete de la trama
  apacket_t frame = ReceiveFrame(iface);
  const unsigned char *packet = frame.packet;

  // Comprobamos que tenga datos
  if (packet != nullptr)
  {
    // Calculamos el offset del tipo
    packet += sizeof(iface->MACaddr) * 2;
    // Comprobamos que el tipo sea el adecuado
    if (packet[0] == grupo + 48 && packet[1] == 0x01)
    {
      // Devolver mac_dst del maestro
      packet -= sizeof(iface->MACaddr);
      for (int j = 0; j < sizeof(iface->MACaddr); j++)
      {
        mac_recibida[j] = packet[j];
        mac_dst[j] = packet[j];
      }
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

char GestionTramas::RecibirCaracter(int &size)
{
  // Obtenemos el paquete de la trama
  apacket_t frame = ReceiveFrame(iface);
  const unsigned char *packet = frame.packet;
  size = frame.header.len - sizeof(iface->MACaddr) * 2 - 2;

  // Comprobamos que tenga datos
  if (packet != nullptr)
  {
    // Calculamos el offset del campo de los datos
    packet += sizeof(iface->MACaddr) * 2;

    // Comprobamos el tipo
    if (packet[0] == 48 + grupo && packet[1] == 0) {
      packet += 2;
      return (char)*packet;
    }

    return 0;
  }
  else
  {
    return 0;
  }
}

int GestionTramas::RecibirInformacion(char *vec)
{
  // Obtenemos el paquete de la trama
  apacket_t frame = ReceiveFrame(iface);
  const unsigned char *packet = frame.packet;

  int size = 0;

  // Comprobamos que tenga datos
  if (packet != nullptr)
  {
    int payloadSize = 255;
    // Calculamos el offset del protocolo
    packet += sizeof(iface->MACaddr) * 2;

    // Comprobar grupo y modo
    if (packet[0] - 48 == grupo && packet[1] == 0)
    {
      // Avanzamos al campo de los datos en el paquete
      packet += 2;
      size = frame.header.len - sizeof(iface->MACaddr) * 2 - 2;
      // Copiamos los datos en una estructura de entrada y salida
      for (int i = 0; i < payloadSize; i++)
      {
        vec[i] = packet[i];
      }
    }
  }

  return size;
}

void GestionTramas::EstConexCarac()
{
  // Enviamos un carácter
  unsigned char car = 0;
  int size = 0;

  // Comprobamos que no sea ESC
  while (car != 27)
  {
    // Recibimos un caracter
    car = RecibirCaracter(size);
    if (car != 0)
      printf("\nRecibido: %c\nTamaño trama: %d Bytes\n", car, size);

    // Comprobamos si se presiona una tecla
    if (kbhit())
    {
      car = getch();
      if (car == 27) // Si la tecla no es ESC...
      {
        if (!kbhit()) break;
        else {
          getch();
          getch();
        }
      }
      else
      {
        EnviarCaracter(car);
      }
    }
  }
}

void GestionTramas::EnviarFichTxt()
{
  // Abrimos el archivo "envio.txt"
  ifstream ifs("envio.txt");

  if (ifs.is_open())
  {
    char info[254];
    int size;

    while (!ifs.eof())
    {
      // Leemos hasta 254 caractéres del fichero
      ifs.read(info, 254);
      // Comprobamos el tamaño de lo que hemos leído
      size = ifs.gcount();

      if(size > 0) {
        // Construir trama
        unsigned char *frame = BuildFrame(iface->MACaddr, mac_dst, protocolCode, (unsigned char *)info);

        // Enviar trama
        SendFrame(iface, frame, size);

        // Liberar memoria
        free(frame);
      }
    }

    ifs.close();
  }
}

void GestionTramas::EnviarFichPyP(bool maestro) {
  unsigned char num_trama = '0';
  // Abrimos el archivo "EProtoc.txt"
  ifstream ifs("EProtoc.txt");

  // Primera fase del emisor: Da igual quien seas, prepara las tramas y envíalas.
  if (ifs.is_open()) {
    char info[254]; // Espacio de memoria local (se borrará al salir del método) para guardar los datos
    int size;

    while (!ifs.eof()) {
      // Leemos hasta 254 caractéres del fichero
      ifs.read(info, 254);
      // Comprobamos el tamaño de lo que hemos leído
      size = ifs.gcount();

      if (size > 0) { // Si hemos leido algo...
      
        // Comprobamos si se ha pulsado una tecla
        bool err = false;
        
        while (kbhit() && !err) { // Mientras queden carácteres en el buffer
          char buff = getch();
          if (buff == 27 && kbhit()) { // Si se ha pulsado ESC...
            buff = getch();
            if (buff == 'O' && kbhit()) { // ... seguido de 0 ...
              buff = getch();
              if (buff == 'S') { // ... terminado en S.
                err = true; // Introducir error.
                printf("\nINTRODUCIENDO ERROR");
              }
            }
          }
        }

        EnviarSTX(num_trama, size, (unsigned char *)info, err); // Enviar info

        CONTROL control = CONTROL::NUL;

        while(control != CONTROL::ACK) {
          DIRECCION direccion;
          unsigned char aux;
          RecibirTrama(direccion, control, num_trama, aux, nullptr);
          if(control == CONTROL::NACK) EnviarSTX(num_trama, size, (unsigned char *)info, false);
        }

        if(num_trama == '0') num_trama = '1'; // Partiendo de que el anterior IF se ha realizado correctamente, ahora cambiaremos el num_trama que enviaremos.
        else num_trama = '0';
      }
    }

    ifs.close();
  }

  // Indica el inicio de una nueva fase
  num_trama = '0';
  
  // Segunda fase del emisor: Si eres maestro, automaticamente envía el cierre. Si no eres maestro (eres esclavo), pide por favor un cierre.
  if (maestro) {
    EnviarEOT(num_trama);
    if(RecibirACK() != num_trama) return; // Si el num_trama que recibimos es erroneo, cerrar protocolo.

  } else { 
    bool fin = false;

    while(!fin) { // Soy esclavo
      DIRECCION direccion;
      CONTROL control;
      unsigned char size;

      if(control != CONTROL::NUL) EnviarEOT(num_trama); // Si la trama recibida no es NUL (no se ha recibido trama), enviamos una EOT
      RecibirTrama(direccion, control, num_trama, size, nullptr); // Recibimos trama

      if(control == CONTROL::ACK) fin = true;

      if(num_trama == '0') num_trama = '1'; // Si el código de antes fue verdad, no nos importa esta parte. Si el maestro no mando ACK, entonces cambiaremos el num_trama para el siguiente intento.
      else num_trama = '0';
    }
  }
}

void GestionTramas::RecibirFichPyP(bool maestro) {
  unsigned char num_trama;
  DIRECCION direccion;
  CONTROL control;
  // Abrimos/creamos el archivo "RProtoc.txt"
  ofstream ofs("RProtoc.txt");

  if (ofs.is_open()) { // Abrimos RProtoc.txt
    unsigned char datos[254]; // Espacio de memoria local (se borrará al salir del método) para guardar los datos
    unsigned char longitud;
    bool eot = false;

    while(!eot) {
      RecibirTrama(direccion, control, num_trama, longitud, datos); // Recibimos una trama

      if (control == CONTROL::STX && datos != nullptr) { // Si recibimos una trama STX y los datos no son nullptr...
        ofs.write((const char *)datos, longitud);  // Escribimos
        EnviarACK(num_trama); // Enviamos confirmación (la trama ha llegado y ha sido leida)

      } else if (control == CONTROL::EOT) { // Si recibimos EOT...

        if (maestro) { // Si somos maestros
          // Dar a elegir
          int opt = 0;
          printf("\nAcepta el cierre de comunicación:\n  [1] Sí.\n  [2] No.\n");
          scanf("%d", &opt);

          if (opt == 1) {
            EnviarACK(num_trama); // Soy maestro y digo cerrar
            eot = true;
          } else if (opt == 2){
            EnviarNACK(num_trama); // Soy maestro y digo NO cerrar
          }
        } else {
          eot = true; // Dejamos de leer
          EnviarACK(num_trama); // Enviamos confirmación de que dejamos de leer (somos esclavos)
        }
      } 
      // Si no es un CONTROL::STX ni un CONTROL::EOT, entonces hemos recibido, lo más seguro, un control que no queremos, o CONTROL::NUL.
      // Para ambas situaciones el resultado es el mismo: Volver a recibir una trama.
    }

    ofs.close();
  }
}

void GestionTramas::ProtParoEspera(bool maestro) {
  unsigned char num_trama = '0';
  unsigned char dir;

  // Establecemos la conexión maestro-esclavo
  if(maestro) {
    int opcion = 0;

    while (opcion < 1 || opcion > 2) {
      printf("Seleccion el tipo de operación:\n\t[1] Operación selección.\n\t[2] Operación sondeo.\n\t[3] Salir.\n");
      cin >> opcion;

      if (opcion == 1) modo_direccion = DIRECCION::SELECCION;
      else if (opcion == 2) modo_direccion = DIRECCION::SONDEO;
    }

    EnviarENQ(num_trama);
    if(RecibirACK() != num_trama) return;

    if(modo_direccion == DIRECCION::SELECCION) {
      // Enviamos el fichero
      EnviarFichPyP(maestro);
    } else {
      // Recibimos el fichero
      RecibirFichPyP(maestro);
    }

  } else {
    num_trama = RecibirENQ(modo_direccion); // TODO ¿Qué es dirección? ---> RES: Puede ser Selección y Sondeo. || Guardamos en modo_direccion del GT.
    EnviarACK(num_trama);
    
    if(modo_direccion == DIRECCION::SELECCION) {
      // Recibimos el fichero
      RecibirFichPyP(maestro);
    } else {
      // Enviamos el fichero
      EnviarFichPyP(maestro);
    }

  }
}

void GestionTramas::RecibirTrama(DIRECCION &direccion, CONTROL &control, unsigned char &num_trama, unsigned char &longitud, unsigned char *datos){
    // Obtenemos el paquete de la trama
    apacket_t frame = ReceiveFrame(iface);
    const unsigned char *packet = frame.packet;

    // Comprobamos que tenga datos
    if (packet != nullptr) {
      // Calculamos el offset del protocolo
      packet += sizeof(iface->MACaddr) * 2;

      // Comprobar grupo y modo
      if (packet[0] - 48 == grupo && packet[1] == 0) {
        // Avanzamos al campo de los datos en el paquete
        packet += 2;

        direccion = (DIRECCION) packet[0];
        control = (CONTROL) packet[1];
        num_trama = packet[2];
        unsigned char calcBCE;
        unsigned char recbBCE;
        if (control == CONTROL::STX) { // Si el control es STX (datos)...
          longitud = packet[3];

          // Calculamos el BCE con el puntero al paquete y su longitud
          calcBCE = CalcularBCE((unsigned char*)(packet+4), longitud);
          recbBCE = packet[4+longitud];
          if (calcBCE == recbBCE) { // Si el BCE es correcto...
            if (datos != nullptr) { // Si el usuario quiere guardar los datos..
              //Guardar datos en el parámetro e/s
              for (int i = 0; i < longitud; i++){
                datos[i] = packet[4+i];
              }
            }
          } else {
            printf("\nR  %c  STX  %c  %d  %d", direccion, (char) num_trama, recbBCE, calcBCE);
            // Enviar NACK
            EnviarNACK(num_trama);
            control = CONTROL::NUL;
          }
        }

        switch (control) {
          case CONTROL::ACK:
            printf("\nR  %c  ACK  %c", direccion, (char) num_trama);
            break;
          case CONTROL::NACK:
            printf("\nR  %c  NCK  %c", direccion, (char) num_trama);
            break;   
          case CONTROL::ENQ:
            printf("\nR  %c  ENQ  %c", direccion, (char) num_trama);
            break;
          case CONTROL::EOT:
            printf("\nR  %c  EOT  %c", direccion, (char) num_trama);
            break;
          case CONTROL::STX:
            printf("\nR  %c  STX  %c  %d  %d", direccion, (char) num_trama, recbBCE, calcBCE);
            break;
        }

        //FIN
      } else {
        printf("\nSe ha recibido una trama inválida (grupo/protocolo incorrecto)\n");
      }
    } else {
      control = CONTROL::NUL;
    }
}

unsigned char GestionTramas::CalcularBCE(unsigned char * datos, unsigned char longitud) {
    unsigned char res =  *datos;
    for (int i = 1; i < longitud; i++){
      res = res xor datos[i];
    }

    return res;
}

unsigned char GestionTramas::RecibirACK() {
    DIRECCION direccion;
    CONTROL control;
    unsigned char num_trama_recibido;
    unsigned char longitud;
    unsigned char * datos = nullptr;
    while(true) {
      RecibirTrama(direccion, control, num_trama_recibido, longitud, datos);
      if ((control == CONTROL::ACK)) {
        return num_trama_recibido;
      } else {
        //printf("\nSe ha recibido una trama no esperada (%d != %d)", control, CONTROL::ACK);
      }
    }
};
unsigned char GestionTramas::RecibirNACK() {
    DIRECCION direccion;
    CONTROL control;
    unsigned char num_trama_recibido;
    unsigned char longitud;
    unsigned char * datos = nullptr;
    while(true) {
      RecibirTrama(direccion, control, num_trama_recibido, longitud, datos);
      if ((control == CONTROL::NACK)) {
        return num_trama_recibido;
      } else {
        printf("\nSe ha recibido una trama no esperada (%d != %d)", control, CONTROL::NACK);
      }
    }
};
unsigned char GestionTramas::RecibirENQ(DIRECCION &direccion) {
    CONTROL control;
    unsigned char num_trama_recibido;
    unsigned char longitud;
    unsigned char * datos = nullptr;
    while(true) {
      RecibirTrama(direccion, control, num_trama_recibido, longitud, datos);
      if ((control == CONTROL::ENQ)) {
        return num_trama_recibido;
      } else {
        //printf("\nSe ha recibido una trama no esperada (%d != %d)", control, CONTROL::ENQ);
      }
    }
};
unsigned char GestionTramas::RecibirEOT() {
    DIRECCION direccion;
    CONTROL control;
    unsigned char num_trama_recibido;
    unsigned char longitud;
    unsigned char * datos = nullptr;
    while(true) {
      RecibirTrama(direccion, control, num_trama_recibido, longitud, datos);
      if ((control == CONTROL::EOT)) {
        return num_trama_recibido;
      } else {
        printf("\nSe ha recibido una trama no esperada (%d != %d)", control, CONTROL::EOT);
      }
    }
};
unsigned char GestionTramas::RecibirSTX(unsigned char &longitud, unsigned char *datos) {
    DIRECCION direccion;
    CONTROL control;
    unsigned char num_trama_recibido;
    while(true) {
      RecibirTrama(direccion, control, num_trama_recibido, longitud, datos);
      if ((control == CONTROL::STX)) {
        return num_trama_recibido;
      } else {
        printf("\nSe ha recibido una trama no esperada (%d != %d)", control, CONTROL::STX);
      }
    }
};

void GestionTramas::EnviarTrama(DIRECCION direccion, CONTROL control, unsigned char num_trama, unsigned char longitud, unsigned char * datos, bool err) {
    int size;
    unsigned char *frame;
    if (control != CONTROL::STX) {
      // TRAMA SENCILLA -> dirección (1B) | control (1B) | num_trama (1B)
      unsigned char payload[3];
      size = sizeof(payload);
      payload[0] = direccion;
      payload[1] = control;
      payload[2] = num_trama;

      // Construir trama
      frame = BuildFrame(iface->MACaddr, mac_dst, protocolCode, (unsigned char *)payload);

    } else {
      // TRAMA COMPLEJA -> dirección (1B) | control (1B) | num_trama (1B) | longitud (1B) | datos (hasta 254B) | BCE (1B)
      unsigned char payload[254 + 5];
      size = longitud + 5;
      payload[0] = direccion;
      payload[1] = control;
      payload[2] = num_trama;
      payload[3] = longitud;
      for (int i = 0; i < longitud; i++) {
        payload[4+i] = *(datos+i);
      }

      // Calcular BCE
      payload[3+longitud+1] = CalcularBCE(datos, longitud);

      // Comprobar si se quiere introducir un error
      if (err) payload[4] = 184;

      // Construir trama
      frame = BuildFrame(iface->MACaddr, mac_dst, protocolCode, (unsigned char *)payload);
   }   

    // Enviar trama
    SendFrame(iface, frame, size);

    // Liberar memoria
    free(frame);
};

void GestionTramas::EnviarACK(unsigned char num_trama) {
    EnviarTrama(modo_direccion, CONTROL::ACK, num_trama, 0, nullptr, false);
    printf("\nE  %c  ACK  %c", modo_direccion, (char) num_trama);
};
void GestionTramas::EnviarNACK(unsigned char num_trama) {
    EnviarTrama(modo_direccion, CONTROL::NACK, num_trama, 0, nullptr, false);
    printf("\nE  %c  NCK  %c", modo_direccion, (char) num_trama);
};
void GestionTramas::EnviarENQ(unsigned char num_trama) {
    EnviarTrama(modo_direccion, CONTROL::ENQ, num_trama, 0, nullptr, false);
    printf("\nE  %c  ENQ  %c", modo_direccion, (char) num_trama);
};
void GestionTramas::EnviarEOT(unsigned char num_trama) {
    EnviarTrama(modo_direccion, CONTROL::EOT, num_trama, 0, nullptr, false);
    printf("\nE  %c  EOT  %c", modo_direccion, (char) num_trama);
};
void GestionTramas::EnviarSTX(unsigned char num_trama, unsigned char longitud, unsigned char * datos, bool err) {
    EnviarTrama(modo_direccion, CONTROL::STX, num_trama, longitud, datos, err);
    printf("\nE  %c  STX  %c  %d", modo_direccion, (char) num_trama, CalcularBCE(datos, longitud));
};
