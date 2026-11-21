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
#include <sstream>
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


            if(it==bloques.end()) return false; //el programa retorna false porque regreso el ultimo espacio, que simplemente no tiene el tamanio suficiente o ni siquiera es un hueco

            int tamHueco = it->tamanio;
            int sobrante = it->tamanio-t;

            it->ID = id;
            it->cuanto = quantum;

            if(sobrante>=min){
                it->tamanio = t;
                bloques.insert(next(it), Bloque{0, sobrante, 0});
            }

            return true;

        }

        void condensar(){
            Iter it = bloques.begin();

            while(it != bloques.end()){
                Iter sig = next(it);
                if(sig!=bloques.end() && it->esHueco() && sig->esHueco()){
                    it->tamanio += sig->tamanio;
                    bloques.erase(sig);
                }else{
                    ++it;
                }
            }
        }

        bool actualizarcuanto(int id, int cuantoSistema){
            Iter it = buscarProceso(id);
            if(it == bloques.end()){ return false; }
            it->cuanto -= cuantoSistema;
            return true;
        }

        string aTexto() const{
            ostringstream ss;

            for(const Bloque& b : bloques){
                ss << "[" << b.ID << ", " << b.tamanio << ", " <<  b.cuanto << "]";
            }
            return ss.str();
        }

        void imprimir(){
            cout << aTexto() << endl;
        }

        int tamTotal(){
            return tam;
        }

        int ocupacionKB() const {
            int suma = 0;
            for (const Bloque& b : bloques) {
                if (b.ID != 0) suma += b.tamanio;
            }
            return suma;
        }

        double ocupacionPorcentaje(){
            return 100*ocupacionKB()/tamTotal();
        }

        int espacioLibre() const {
            return tam-ocupacionKB();
        }

        int huecoMayor() const{
            int mayor = 0;
            for (const Bloque& b : bloques) {
                if (b.ID == 0 && b.tamanio>mayor) mayor = b.tamanio;
            }
            return mayor;
        }

        int huecoMenor() const {
            int menor = 0;
            for (const Bloque& b : bloques) {
                if (b.ID == 0 && b.tamanio < menor) menor = b.tamanio;
            }
            return menor;
        }

        int cantidadHuecos() const{
            int n = 0;
            for (const Bloque& b : bloques) {
                if (b.ID == 0) n++;
            }
            return n;
        }

        void liberarID(int id){
            Iter it = buscarProceso(id);
            it->ID=0;
            it->cuanto=0;
        }
};
