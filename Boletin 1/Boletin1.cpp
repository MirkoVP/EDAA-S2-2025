#include <iostream>   
#include <vector>
using namespace std;

int BusquedaLineal(const vector<int>& vector, int valor) {
    for (size_t i = 0; i < vector.size(); ++i) {
        if (vector[i] == valor) {
            return static_cast<int>(i); 
        }
    }
    return -1;
}

int BusquedaBinaria(const vector<int>& vector, int valor, int inicio = 0, int fin = -1){
    if(fin == -1) fin = static_cast<int>(vector.size())-1;
    while (inicio <= fin){
        int medio = inicio + (fin - inicio)/2;
        if(vector[medio] == valor) return medio;
        if(vector[medio] < valor) inicio = medio + 1;
        else fin = medio -1;
    }
    return -1;
}

int BusquedaGalopante(const vector<int>& vector, int valor){
    int n = static_cast<int>(vector.size());
    if(n == 0) return -1;
    if(vector[0] == valor) return 0;
    int i = 1;
    while(i < n && vector[i] <= valor){
        i *= 2;
    }
    int inicio = i/2;
    int fin = min(i, n -1);
    return BusquedaBinaria(vector, valor, inicio, fin);
}