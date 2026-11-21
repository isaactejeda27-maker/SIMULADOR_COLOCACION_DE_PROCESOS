// main.cpp - Banco de pruebas que integra las partes 1, 2 y 3

// Compilar:  g++ -std=c++17 -Wall -Wextra main.cpp -o simulador
#include "modulos.hpp"

#include <chrono>
#include <thread>

// ---------------------------------------------------------------
// Estadisticas minimas, solo para poder comparar mejor vs peor ajuste
// (Persona 4 puede reemplazar esto por su modulo completo)
// ---------------------------------------------------------------
struct Estadisticas
{
    int ciclos = 0;
    int admitidos = 0;
    int terminados = 0;
    int ciclosEspera = 0;      // ciclos en los que el proceso listo no cupo
    double sumaOcupacion = 0;  // % de memoria ocupada, sumado por ciclo
    double sumaHuecos = 0;     // cantidad de huecos, sumada por ciclo
    double sumaFragExt = 0;    // KB libres que NO estan en el hueco mayor

    double ocupacionMedia() const { return ciclos ? sumaOcupacion / ciclos : 0; }
    double huecosMedios() const { return ciclos ? sumaHuecos / ciclos : 0; }
    double fragExtMedia() const { return ciclos ? sumaFragExt / ciclos : 0; }
};

// ---------------------------------------------------------------
// Parte 3: un ciclo completo
//   1. Entra proceso  2. Se atiende RR  3. Lista RR  4. Descarga y condensa
// Devuelve true si el proceso "nuevo" logro entrar a memoria.
// ---------------------------------------------------------------
bool ciclo(Memoria& mem, PlanificadorRR& rr, const PCB& nuevo, bool hayNuevo,
           const string& nombre, ostream& out, Estadisticas& est)
{
    out << "\n--- " << nombre << " ---\n";
    bool entro = false;

    // 1. Entrada a memoria (si no cabe, condensa y reintenta)
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

    // 2. Se atiende Round Robin
    if (rr.vacia())
    {
        out << "Cola RR vacia\n";
    }
    else
    {
        Turno t = rr.atender();
        out << "Atendiendo proceso " << t.antes.aTexto() << "\n";
        mem.actualizarcuanto(t.antes.id, t.consumido);  // Memoria RESTA el valor

        // 3. Lista RR (despues de reinsertar, igual que el PDF)
        string lista = rr.listaTexto();
        out << "Lista RR " << (lista.empty() ? "(vacia)" : lista) << "\n";

        // 4. Descarga si termino
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
            out << mem.aTexto() << "\n";  // impresion final de ciclo
        }
    }

    // Estadisticas de este ciclo
    est.ciclos++;
    est.sumaOcupacion += mem.ocupacionPorcentaje();
    est.sumaHuecos += mem.cantidadHuecos();
    est.sumaFragExt += mem.espacioLibre() - mem.huecoMayor();

    return entro;
}

const char* nombreMetodo(Metodo m)
{
    return m == Metodo::MEJOR_AJUSTE ? "Mejor ajuste" : "Peor ajuste";
}

// ---------------------------------------------------------------
// Simulacion aleatoria (usa el generador de la parte 2)
// ---------------------------------------------------------------
Estadisticas simular(Metodo metodo, int memKB, int quantum, int tamMax, int cuantoMax,
                     int ciclos, unsigned semilla, int msPausa, ostream& out)
{
    Memoria mem(memKB, metodo, 32);
    PlanificadorRR rr(quantum);
    GeneradorProcesos gen(tamMax, cuantoMax, 32, semilla);
    Estadisticas est;

    out << "Estado inicial\nLista RR (vacia)\n" << mem.aTexto() << "\n";

    for (int i = 1; i <= ciclos; i++)
    {
        PCB listo = gen.verListo();  // siempre hay un proceso listo para entrar
        bool entro = ciclo(mem, rr, listo, true, "CICLO " + to_string(i), out, est);
        if (entro) gen.tomarListo();  // si no entro, sigue esperando como "listo"

        if (msPausa > 0) this_thread::sleep_for(chrono::milliseconds(msPausa));
    }
    return est;
}

// ---------------------------------------------------------------
// Ejemplo del PDF (parte 3 original)
// ---------------------------------------------------------------
void ejemploPDF(Metodo metodo)
{
    const int QUANTUM = 2;
    Memoria mem(4096, metodo, 32);
    PlanificadorRR rr(QUANTUM);
    Estadisticas est;

    // Preparacion: [1,300,2][0,96,0][5,500,9][0,3200,0]
    mem.asignar(1, 300, 2);
    mem.asignar(2, 96, 1);
    mem.asignar(5, 500, 9);
    mem.liberar(2);
    rr.agregar(PCB{5, 500, 9});
    rr.agregar(PCB{1, 300, 2});

    cout << "\n=== Ejemplo del PDF (" << nombreMetodo(metodo)
         << ", cuanto del sistema = " << QUANTUM << ") ===\n";
    cout << "Estado inicial\nLista RR " << rr.listaTexto() << "\n" << mem.aTexto() << "\n";

    PCB llegadas[3] = {{6, 256, 10}, {7, 384, 7}, {8, 200, 4}};
    const char* nombres[3] = {"CICLO N", "CICLO N+1", "CICLO N+2"};
    for (int i = 0; i < 3; i++)
        ciclo(mem, rr, llegadas[i], true, nombres[i], cout, est);
}

// ---------------------------------------------------------------
// Utilidad para leer enteros con valor por defecto
// ---------------------------------------------------------------
int leerInt(const string& pregunta, int porDefecto)
{
    cout << pregunta << " [" << porDefecto << "]: ";
    string linea;
    getline(cin >> ws, linea);
    if (linea.empty()) return porDefecto;
    try { return stoi(linea); } catch (...) { return porDefecto; }
}

void imprimirFila(const char* etiqueta, const Estadisticas& e)
{
    cout << left << setw(14) << etiqueta
         << right << setw(10) << e.admitidos
         << setw(12) << e.terminados
         << setw(10) << e.ciclosEspera
         << setw(13) << fixed << setprecision(2) << e.ocupacionMedia()
         << setw(10) << e.huecosMedios()
         << setw(14) << e.fragExtMedia() << "\n";
}

int main()
{
    cout << "=== Simulador de memoria + Round Robin (banco de pruebas) ===\n"
         << "1) Ejemplo del PDF\n2) Simulacion aleatoria\n3) Comparar mejor vs peor ajuste\n";
    int modo = leerInt("Opcion", 1);

    if (modo == 1)
    {
        int op = leerInt("Algoritmo (1 = Mejor ajuste, 2 = Peor ajuste)", 1);
        ejemploPDF(op == 2 ? Metodo::PEOR_AJUSTE : Metodo::MEJOR_AJUSTE);
        return 0;
    }

    // Configuracion para los modos 2 y 3
    int memMB = leerInt("Memoria en MB (1, 4 u 8)", 4);
    if (memMB != 1 && memMB != 4 && memMB != 8) memMB = 4;
    int memKB = memMB * 1024;
    int quantum = max(1, leerInt("Cuanto del sistema (quantum)", 2));
    int tamMax = leerInt("Tamano maximo de proceso (KB)", 1024);
    tamMax = min(max(tamMax, 32), memKB);  // nunca mayor que la memoria total
    int cuantoMax = max(1, leerInt("Cuanto maximo de un proceso", 10));
    int ciclos = max(1, leerInt("Ciclos a simular", 20));
    unsigned semilla = (unsigned)leerInt("Semilla (0 = aleatoria)", 0);
    if (semilla == 0) semilla = random_device{}();

    if (modo == 2)
    {
        int op = leerInt("Algoritmo (1 = Mejor ajuste, 2 = Peor ajuste)", 1);
        int ms = max(0, leerInt("Pausa entre ciclos (ms, 0 = sin pausa)", 0));
        Metodo m = (op == 2) ? Metodo::PEOR_AJUSTE : Metodo::MEJOR_AJUSTE;
        cout << "\n=== " << nombreMetodo(m) << " | semilla " << semilla << " ===\n";
        Estadisticas e = simular(m, memKB, quantum, tamMax, cuantoMax, ciclos, semilla, ms, cout);
        cout << "\n--- Resumen ---\nAdmitidos: " << e.admitidos << "  Terminados: " << e.terminados
             << "  Ciclos en espera: " << e.ciclosEspera << "\nOcupacion media: " << fixed
             << setprecision(2) << e.ocupacionMedia() << "%\n";
        return 0;
    }

    // Modo 3: misma secuencia de procesos (misma semilla) para ambos algoritmos
    ostringstream descarte;  // la salida detallada no se muestra
    Estadisticas mejor = simular(Metodo::MEJOR_AJUSTE, memKB, quantum, tamMax, cuantoMax,
                                 ciclos, semilla, 0, descarte);
    Estadisticas peor = simular(Metodo::PEOR_AJUSTE, memKB, quantum, tamMax, cuantoMax,
                                ciclos, semilla, 0, descarte);

    cout << "\n=== Comparacion | " << memMB << " MB, quantum " << quantum << ", " << ciclos
         << " ciclos, semilla " << semilla << " ===\n";
    cout << left << setw(14) << "Algoritmo" << right << setw(10) << "Admitidos" << setw(12)
         << "Terminados" << setw(10) << "Esperas" << setw(13) << "Ocupacion %" << setw(10)
         << "Huecos" << setw(14) << "FragExt (KB)" << "\n";
    imprimirFila("Mejor ajuste", mejor);
    imprimirFila("Peor ajuste", peor);
    return 0;
}
