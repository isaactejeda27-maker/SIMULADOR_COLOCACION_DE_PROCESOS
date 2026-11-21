#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <list>
#include <deque>
#include <iterator>
 using namespace std;

string fp(const Proceso& p) {
    return "[" + to_string(p.id) + "," + to_string(p.tam) + "," + to_string(p.cuanto) + "]";
}

string listaRR(const deque<Proceso>& rr) {
    string s;
    for (const Proceso& p : rr) s += fp(p) + " ";
    return s.empty() ? "(vacia)" : s;
}

// 1. Entra proceso  2. Atiende  3. Lista RR  4. Descarga y condensa si termino
void ciclo(Memoria& mem, deque<Proceso>& rr, const Proceso& nuevo, int quantum, const string& nombre) {
    cout << "\n--- " << nombre << " ---\n";

    // 1. Entrada a memoria (si no cabe, condensa y reintenta)
    bool entro = mem.asignar(nuevo.id, nuevo.tam, nuevo.cuanto);
    if (!entro) {
        int antes = mem.cantidadHuecos();
        mem.condensar();
        if (mem.cantidadHuecos() < antes) {
            cout << "Condensando huecos\n";
            mem.imprimir();
        }
        entro = mem.asignar(nuevo.id, nuevo.tam, nuevo.cuanto);
    }
    if (entro) {
        cout << "Entra Proceso " << fp(nuevo) << "\n";
        rr.push_back(nuevo);
        mem.imprimir();
    } else {
        cout << "Sin espacio para " << fp(nuevo) << ", espera\n";
    }

    // 2. Se atiende Round Robin
    if (rr.empty()) { cout << "Cola RR vacia\n"; return; }
    Proceso p = rr.front();
    rr.pop_front();
    cout << "Atendiendo proceso " << fp(p) << "\n";

    int usado = std::min(quantum, p.cuanto);   // no consume mas de lo que le queda
    p.cuanto -= usado;
    mem.actualizarcuanto(p.id, usado);         // la clase de la Persona 1 RESTA el valor
    if (p.cuanto > 0) rr.push_back(p);         // si no termino, vuelve al final de la cola

    // 3. Lista RR (despues de reinsertar, igual que el PDF)
    cout << "Lista RR " << listaRR(rr) << "\n";

    // 4. Descarga si termino
    if (p.cuanto == 0) {
        cout << "Descargando proceso " << fp(p) << "\n";
        mem.liberarID(p.id);
        mem.imprimir();
        int antes = mem.cantidadHuecos();
        mem.condensar();
        if (mem.cantidadHuecos() < antes) {
            cout << "Condensando huecos\n";
            mem.imprimir();
        }
    } else {
        mem.imprimir();                        // impresion final de ciclo
    }
}

int main() {
    const int MEMORIA_KB = 4096;   // 4 MB
    const int SPLIT_MIN  = 32;
    const int QUANTUM    = 2;

    int op = 0;
    cout << "Algoritmo (1 = Mejor ajuste, 2 = Peor ajuste): ";
    cin >> op;
    Metodo metodo = (op == 2) ? Metodo::PEOR_AJUSTE : Metodo::MEJOR_AJUSTE;

    Memoria mem(MEMORIA_KB, metodo, SPLIT_MIN);
    deque<Proceso> rr;

    // Preparacion (sin imprimir)
    //   [1,300,2][0,96,0][5,500,9][0,3200,0]
    // Se crea el proceso 2 de 96 KB y se libera, para que quede el hueco de 96.
    mem.asignar(1, 300, 2);
    mem.asignar(2, 96, 1);
    mem.asignar(5, 500, 9);
    mem.liberarID(2);
    rr.push_back(Proceso{5, 500, 9});
    rr.push_back(Proceso{1, 300, 2});

    cout << "\n=== Ejemplo (" << (op == 2 ? "Peor" : "Mejor")
         << " ajuste, cuanto del sistema = " << QUANTUM << ") ===\n";
    cout << "Estado inicial\n";
    cout << "Lista RR " << listaRR(rr) << "\n";
    mem.imprimir();

    Proceso llegadas[3] = { {6, 256, 10}, {7, 384, 7}, {8, 200, 4} };
    const char* nombres[3] = { "CICLO N", "CICLO N+1", "CICLO N+2" };

    for (int i = 0; i < 3; i++)
        ciclo(mem, rr, llegadas[i], QUANTUM, nombres[i]);

    return 0;
}