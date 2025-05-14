//============================================================================
// ----------- PRACTICAS DE FUNDAMENTOS DE REDES DE COMUNICACIONES -----------
// ---------------------------- CURSO 2023/24 --------------------------------
// ----------------------------- SESION2.CPP ---------------------------------
//============================================================================
// Autores: Manuel Alonso González
//         José Manuel de Torres Domínguez
// Grupo: 2

#include "UserInterface.h"

interface_t EscogerInterfaz()
{
    interface_t iface;
    pcap_if_t *avail_ifaces = NULL;

    avail_ifaces = GetAvailAdapters();

    // Mostramos todas las interfaces disponibles en ese momento
    printf("Interfaces disponibles:\n");

    int i = 0;
    while (avail_ifaces != NULL)
    {
        printf("[%d] %s\n", i, avail_ifaces->name);
        avail_ifaces = avail_ifaces->next;
        i++;
    }

    // Preguntamos por la interfaz a usar
    int choice;

    do
    {
        printf("Seleccione interfaz: ");
        scanf("%d", &choice);
    } while (choice < 0 && choice > i);

    pcap_if_t *if_choice = GetAvailAdapters();

    for (int j = 0; j < choice; j++)
    {
        if_choice = if_choice->next;
    }

    // Obtenemos la MAC de la interfaz elegida
    printf("Interfaz Elegida: %s\n", if_choice->name);
    setDeviceName(&iface, if_choice->name);
    GetMACAdapter(&iface);

    // Mostramos la MAC origen
    printf("La MAC origen es: ");
    for (int j = 0; j < sizeof(iface.MACaddr); j++)
    {
        printf("%02X", iface.MACaddr[j]);
        if (j < sizeof(iface.MACaddr) - 1)
            printf(":");
        else
            printf("\n");
    }

    return iface;
}

void rolMaestro(GestionTramas& gt)
{
    // Envía la señal a la red para que la detecte el esclavo
    gt.EnviarSignalMaestro();

    // Recibe la confirmación del esclavo
    unsigned char mac_escl[6];
    while (!gt.RecibirMaestro(mac_escl));

    // Una vez que la encuentra la muestra por pantalla
    printf("Estación encontrada. La MAC es: ");
    for (int j = 0; j < sizeof(mac_escl); j++)
    {
        printf("%02X", mac_escl[j]);
        if (j < sizeof(mac_escl) - 1)
            printf(":");
        else
            printf("\n");
    }
}

void rolEsclavo(GestionTramas& gt)
{
    // Recibe la señal del maestro
    unsigned char mac_recibida[6];
    while (!gt.RecibirEsclavo(mac_recibida));

    // Envía la señal al maestro
    gt.EnviarSignalEsclavo();

    // Una vez que la encuentra la muestra por pantalla
    printf("Estación encontrada. La MAC es: ");
    for (int j = 0; j < sizeof(mac_recibida); j++)
    {
        printf("%02X", mac_recibida[j]);
        if (j < sizeof(mac_recibida) - 1)
            printf(":");
        else
            printf("\n");
    }
}

void mostrarMenu(int modo)
{
    printf("\n");
    printf("Selección de modo:\n");
    printf("   [F1] - Envío de caracteres interactivos\n");
    if (modo == 1)
        printf("   [F2] - Envío de un fichero\n");
    printf("   [F3] - Protocolo paro y espera\n");
    printf("   [ESC] - Salir\n");
}
