//Developer: Edgar Alejandro Esparza Reyes (cada dia mas cerca de un senior)
#include<iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <array>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include <list>
#include <iterator>
using namespace std;

struct Bloque{
    int ID;
    int tamanio;
    int cuanto;

    bool esHueco() { return ID==0; }
};

enum class Metodo { MEJOR_AJUSTE, PEOR_AJUSTE };

class Memoria{
    private:
        using Iter = list<Bloque>::iterator;


        list<Bloque> bloques;
        int tam;
        Metodo tipo;
        int min;

        
        Iter elegirHueco(int tam){
            Iter elegido = bloques.end(); //bloques =[1,543, 7] [0, 7423, 0] 

            for(Iter it=bloques.begin(); it != bloques.end(); ++it){
                if(!it->esHueco() || it->tamanio < tam) continue; //salta el ciclo, es otro proceso o no cabe en el hueco

                if(elegido==bloques.end()){
                    elegido = it;
                }else if(tipo == Metodo::MEJOR_AJUSTE && it->tamanio < elegido->tamanio){
                    elegido = it;
                }else if(tipo == Metodo::PEOR_AJUSTE && it->tamanio > elegido->tamanio){
                    elegido = it;
                }
            }

            return elegido;
        }

        Iter buscarProceso(int id){
            for(Iter it=bloques.begin(); it != bloques.end(); it++){
                if(!it->esHueco() && it->ID == id) return it;
            }
            return bloques.end();
        }

    public:

        //El constructor creara el primer bloque con ID = 0 para simbolizar que no es un proceso, sino un hueco, y se le va a asignar el tamanio de tamTotalKB 
        Memoria(int tamTotalKB, Metodo metodo, int minSplit) : tam(tamTotalKB), tipo(metodo), min(minSplit) {
            bloques.push_back(Bloque{0, tamTotalKB, 0});
        }      

        void setTipo(Metodo metodo){
            tipo=metodo;
        }

        Metodo getTipo(){
            return tipo;
        }


        bool asignar(int id, int t, int quantum){
            if(id<=0 || t<=0){ 
                return false;
            }
            Iter it = elegirHueco(t);

            if(it->tamanio < t || !it->esHueco()) return false; el 
        }

};