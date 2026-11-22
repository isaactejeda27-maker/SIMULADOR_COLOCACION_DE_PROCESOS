    #include <iostream>
    #include <string>
    #include <iomanip>
    #include <list>
    #include <iterator>
    #include <sstream>
    #include <deque>
    #include <random>
    using namespace std;

    struct Bloque
    {
        int ID;
        int tamanio;
        int cuanto;

        bool esHueco() { return ID == 0; }
    };

    enum class Metodo { MEJOR_AJUSTE, PEOR_AJUSTE };

    class Memoria
    {
        private:
            using Iter = list<Bloque>::iterator;

            list<Bloque> bloques;
            int tam;
            Metodo tipo;
            int min;

            Iter elegirHueco(int tam){
                Iter elegido = bloques.end();

                for(Iter it=bloques.begin(); it != bloques.end(); ++it){
                    if(!it->esHueco() || it->tamanio < tam)
                    {
                        continue;
                    }

                    if(elegido==bloques.end())
                    {
                        elegido = it;
                    }
                    else if(tipo == Metodo::MEJOR_AJUSTE && it->tamanio < elegido->tamanio)
                    {
                        elegido = it;
                    }
                    else if(tipo == Metodo::PEOR_AJUSTE && it->tamanio > elegido->tamanio)
                    {
                        elegido = it;
                    }
                }

                return elegido;
            }

            Iter buscarProceso(int id)
            {
                for(Iter it=bloques.begin(); it != bloques.end(); it++)
                {
                    if(!it->esHueco() && it->ID == id) return it;
                }
                return bloques.end();
            }

        public:

            Memoria(int tamTotalKB, Metodo metodo, int minSplit) : tam(tamTotalKB), tipo(metodo), min(minSplit)
            {
                bloques.push_back(Bloque{0, tamTotalKB, 0});
            }

            bool asignar(int id, int t, int quantum)
            {
                if(id <= 0 || t <= 0)
                {
                    return false;
                }
                Iter it = elegirHueco(t);

                if(it == bloques.end()) return false;

                int sobrante = it -> tamanio - t;

                it -> ID = id;
                it -> cuanto = quantum;

                if(sobrante >= min)
                {
                    it->tamanio = t;
                    bloques.insert(next(it), Bloque{0, sobrante, 0});
                }

                return true;

            }

            bool liberar(int id)
            {
                Iter it = buscarProceso(id);
                if(it == bloques.end()) return false;
                it->ID = 0;
                it->cuanto = 0;
                return true;
            }

            void condensar()
            {
                Iter it = bloques.begin();

                while(it != bloques.end())
                {
                    Iter sig = next(it);
                    if(sig!=bloques.end() && it->esHueco() && sig->esHueco())
                    {
                        it->tamanio += sig->tamanio;
                        bloques.erase(sig);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            bool actualizarcuanto(int id, int cuantoSistema)
            {
                Iter it = buscarProceso(id);
                if(it == bloques.end()){ return false; }
                it->cuanto -= cuantoSistema;
                return true;
            }

            string aTexto() const
            {
                ostringstream ss;

                for(const Bloque& b : bloques)
                {
                    ss << "[" << b.ID << "," << b.tamanio << "," <<  b.cuanto << "]"; // CORREGIDO: sin espacios, como pide el PDF
                }
                return ss.str();
            }

            int tamTotal()
            {
                return tam;
            }

            int ocupacionKB() const
            {
                int suma = 0;
                for (const Bloque& b : bloques)
                {
                    if (b.ID != 0) suma += b.tamanio;
                }
                return suma;
            }

            double ocupacionPorcentaje()
            {
                return 100.0 * ocupacionKB() / tamTotal(); 
            }

            int cantidadHuecos() const
            {
                int n = 0;
                for (const Bloque& b : bloques) {
                    if (b.ID == 0) n++;
                }
                return n;
            }

    };

    struct PCB
    {
        int id;       
        int tamanio;  
        int cuanto;   

        string aTexto() const
        {
            return "[" + to_string(id) + "," + to_string(tamanio) + "," + to_string(cuanto) + "]";
        }
    };

    class GeneradorProcesos
    {
    private:
        int tamMin;
        int tamMax;
        int cuantoMax;
        int proximoId;
        mt19937 motor;
        PCB listo;

        PCB generar()
        {
            uniform_int_distribution<int> distTam(tamMin, tamMax);
            uniform_int_distribution<int> distCuanto(1, cuantoMax);
            PCB p{proximoId, distTam(motor), distCuanto(motor)};
            proximoId++;
            return p;
        }

    public:
        GeneradorProcesos(int tamMaximo, int cuantoMaximo, int tamMinimo = 32,
            unsigned semilla = random_device{}()) : tamMin(tamMinimo), tamMax(tamMaximo), cuantoMax(cuantoMaximo),
            proximoId(1), motor(semilla)
            {
            if (tamMin < 1) tamMin = 1;
            if (tamMax < tamMin) tamMax = tamMin;
            if (cuantoMax < 1) cuantoMax = 1;
            listo = generar();
            }

        PCB verListo() const { return listo; }

        PCB tomarListo()
        {
            PCB salida = listo;
            listo = generar();
            return salida;
        }
    };

    struct Turno
    {
        PCB antes;       
        PCB despues;      
        int consumido;   
        bool termino;    
    };

    class PlanificadorRR
    {
    private:
        deque<PCB> cola;
        int cuantoSistema;

    public:
        PlanificadorRR(int cuantoDelSistema) : cuantoSistema(max(1, cuantoDelSistema)) {}

        void agregar(const PCB& p) { cola.push_back(p); }
        bool vacia() const { return cola.empty(); }

        Turno atender()
        {
            if (cola.empty()) return Turno{PCB{0, 0, 0}, PCB{0, 0, 0}, 0, false};

            PCB p = cola.front();
            cola.pop_front();

            Turno t;
            t.antes = p;
            t.consumido = min(cuantoSistema, p.cuanto);
            p.cuanto -= t.consumido;
            t.despues = p;
            t.termino = (p.cuanto == 0);

            if (!t.termino) cola.push_back(p); 
            return t;
        }

        string listaTexto() const
        {
            string s;
            for (size_t i = 0; i < cola.size(); i++)
            {
                if (i > 0) s += " ";
                s += cola[i].aTexto();
            }
            return s;
        }
    };
