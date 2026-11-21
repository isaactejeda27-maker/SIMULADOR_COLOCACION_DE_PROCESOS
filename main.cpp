// FUNCION PRINCIPAL DEL PROYECTO
//librerias
#include <iostream> //entradas y salidas
#include <stdio.h>
#include "funciones.hpp"

//Espacio de nombres
using namespace std;

//prototipos de funciones

//funcion principal
int main(){

    int seleccion;
    bool salir = false;

    do{
        system("pause");
        //Menu principal
        cout << "--| COLOCADOR DE PROCESOS |--" << endl;
        cout << "Seleccionar metodo de colocacion......1" << endl;
        cout << "Ajustar tamanio de memoria............2" << endl;
        cout << "Salir.................................3" << endl;
        cout << "-SELECCION: ";
        cin >> seleccion;

        switch (seleccion){
        case 1:
                tipo_metodo();
            break;
        case 2:
                //Ajuste de tamaño
            break;
        case 3:
            !salir; 
            break;
        default:
            cout << "Esta opcion no existe!!";
            system("pause");
            break;
        }

    
    }while(salir == false);

    cout << "Puedes cerrar esta ventana!!";

    return 0;
} 



