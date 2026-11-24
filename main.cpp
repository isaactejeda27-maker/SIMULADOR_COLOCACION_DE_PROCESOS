#include "modulos.hpp"

#include<conio.h>
#include <chrono>
#include <thread>

struct Estadisticas
{
    int ciclos = 0;
    int admitidos = 0;
    int terminados = 0;
    int ciclosEspera = 0;     
    double sumaOcupacion = 0; 

    double ocupacionMedia() const { return ciclos ? sumaOcupacion / ciclos : 0; }
};

void mostrarEstado (ostream& out, const Memoria& mem, const PlanificadorRR& rr, const PCB* ejecutando = nullptr)
{

    if(ejecutando)
        out << "Proceso en ejecucion: " << ejecutando->aTexto() << "\n";
    else
        out << "Proceso en ejecucion: Ninguno\n";

    out << "Lista RR: ";

    string lista = rr.listaTexto();

    if(lista.empty())
        out << "(vacia)\n";
    else
        out << lista << "\n";

    out << "Memoria:\n";
    out << mem.aTexto() << "\n";

}

bool ciclo(Memoria& mem, PlanificadorRR& rr, const PCB& nuevo, bool hayNuevo, const string& nombre, ostream& out, Estadisticas& est)
{
    out << "\n" << nombre << "\n";
    mostrarEstado(out, mem, rr, nullptr);
    bool entro = false;

    if (hayNuevo)
    {
        entro = mem.asignar(nuevo.id, nuevo.tamanio, nuevo.cuanto);
        if (!entro)
        {
            int antes = mem.cantidadHuecos();
            mem.condensar();
            if (mem.cantidadHuecos() < antes)
            {
                out << "Condensando huecos\n" << mem.aTexto() << "\n";
            }
            entro = mem.asignar(nuevo.id, nuevo.tamanio, nuevo.cuanto);
        }

        if (entro)
        {
            out << "Entra Proceso " << nuevo.aTexto() << "\n";
            rr.agregar(nuevo);
            out << mem.aTexto() << "\n";
            est.admitidos++;
        }
        else
        {
            out << "Sin espacio para " << nuevo.aTexto() << ", espera\n";
            est.ciclosEspera++;
        }
    }

    if (rr.vacia())
    {
        out << "Cola RR vacia\n";
    }
    else
    {
        Turno t = rr.atender();
        out << "Atendiendo proceso " << t.antes.aTexto() << "\n";
        mem.actualizarcuanto(t.antes.id, t.consumido);  
        mostrarEstado(out, mem, rr, &t.despues);
        string lista = rr.listaTexto();
        out << "Lista RR " << (lista.empty() ? "(vacia)" : lista) << "\n";

        if (t.termino)
        {
            out << "Descargando proceso " << t.despues.aTexto() << "\n";
            mem.liberar(t.despues.id);
            out << mem.aTexto() << "\n";

            int antes = mem.cantidadHuecos();
            mem.condensar();
            if (mem.cantidadHuecos() < antes)
            {
                out << "Condensando huecos\n" << mem.aTexto() << "\n";
            }
            est.terminados++;
        }
        else
        {
            out << mem.aTexto() << "\n"; 
        }
    }

    est.ciclos++;
    est.sumaOcupacion += mem.ocupacionPorcentaje();

    return entro;
}

const char* nombreMetodo(Metodo m)
{
    return m == Metodo::MEJOR_AJUSTE ? "Mejor ajuste" : "Peor ajuste";
}

Estadisticas simular(Metodo metodo, int memKB, int quantum, int tamMax, int cuantoMax, unsigned semilla, int msPausa, ostream& out)
{
    Memoria mem(memKB, metodo, 32);
    PlanificadorRR rr(quantum);
    GeneradorProcesos gen(tamMax, cuantoMax, 32, semilla);
    Estadisticas est;

    out << "Estado inicial\n";
    mostrarEstado(out, mem, rr, nullptr);

    int i = 1;

    char t;
    while (true)
    {
        if(_kbhit()){
            t=_getch();

            if(t=='-'){
                if(msPausa>1000){
                    msPausa = msPausa-1000;
                }else if(msPausa>100){
                    msPausa -= 100;
                }else{
                    msPausa=100;
                }
            }else if(t=='+'){
                if(msPausa<1000){
                    msPausa = msPausa+100;
                }else{
                    msPausa = msPausa+1000;
                }
            }else if(t=='e' || t == 'E'){
                break;
            }
        }

        PCB listo = gen.verListo();

        bool entro = ciclo(mem, rr, listo, true, "CICLO " + to_string(i), out, est);

        if (entro)
            gen.tomarListo();

        if (msPausa > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(msPausa));
        }
        i++;
    }
    return est;
}

int leerInt(const string& pregunta, int porDefecto)
{
    cout << pregunta << " [" << porDefecto << "]: ";
    string linea;
    getline(cin >> ws, linea);
    if (linea.empty()) return porDefecto;
    try { return stoi(linea); } catch (...) { return porDefecto; }
}

int main()
{
    cout << " Simulador\n" << "1. Mejor ajuste\n" << "2. Peor ajuste\n";
    int modo = leerInt("Opcion", 1);

    Metodo m;

    switch(modo)
    {
        case 1:
            m = Metodo::MEJOR_AJUSTE;
            break;

        case 2:
            m = Metodo::PEOR_AJUSTE;
            break;

        default:
            cout << "Opcion incorrecta\n";
            return 0;
    }

    int memMB;

    do
    {
        memMB = leerInt("Memoria en MB (1, 4 u 8)", 4);

        if(memMB != 1 && memMB != 4 && memMB != 8)
        {
            cout << "Valor incorrecto. Solo se permite 1, 4 u 8 MB.\n";
        }

    } while(memMB != 1 && memMB != 4 && memMB != 8);


    int memKB = memMB * 1024;

    int quantum;

    do
    {
        quantum = leerInt("Cuanto del sistema el quantum", 2);

        if(quantum < 1)
        {
            cout << "El quantum debe ser mayor que 0.\n";
        }

    } while(quantum < 1);

    int tamMax=1024;


    int cuantoMax;

    do
    {
        cuantoMax = leerInt("Cuanto maximo de un proceso", 10);

        if(cuantoMax < 1)
        {
            cout << "Debe ser mayor que 0.\n";
        }

    } while(cuantoMax < 1);


    int ms=5000;
    
    unsigned semilla = random_device{}();

    cout << nombreMetodo(m) << "\n" ;

    Estadisticas e = simular(
        m,
        memKB,
        quantum,
        tamMax,
        cuantoMax,
        semilla,
        ms,
        cout);

    return 0;
}
