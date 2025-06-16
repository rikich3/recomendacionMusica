      
#ifndef ARREGLO_PERSONAS_CPP
#define ARREGLO_PERSONAS_CPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>    // Para sqrt, pow
#include <limits>   // Para infinity
#include <algorithm>// Para sort, min
#include <set>      // Para canciones candidatas unicas

// Definición de estructuras (los campos se adaptan a std::vector para flexibilidad)

struct Cancion {
    std::string id;
    int totalRatings = 0;   // Cuantas personas la calificaron
    int successes = 0;      // Cuantas personas le dieron > 4
    double sumScores = 0.0; // Suma de todas las calificaciones
    double popularidad = 0.0; // % de personas que han rankeado
    double disfrute = 0.0;    // Promedio de calificación
    double probGustar = 0.0;  // Probabilidad general de gustar (Rule of Succession)
};

struct Persona {
    std::string id;
    std::vector<int> cancionesGustar; // Indices de canciones con score > 4
    std::vector<double> vectorSentimiento; // Puntuaciones para todas las canciones (2.5 si no calificó)
    // PersonasParecidas y CancionesPuedenGustar se calcularán y almacenarán en ejer3.cpp
};

// Estructuras de datos globales (para ser accesibles desde ejer3.cpp)
// Una mejor práctica sería encapsular esto en una clase o namespace.
std::vector<Persona> personas;
std::vector<Cancion> canciones;
std::map<std::string, int> personaMap; // Mapea personId a indice en el vector personas
std::map<std::string, int> cancionMap; // Mapea songId a indice en el vector canciones

// Estructura temporal para guardar las calificaciones mientras se cargan los IDs
struct Rating {
    int personIndex;
    int songIndex;
    double score;
};
std::vector<Rating> rawRatings; // Lista de todas las calificaciones leídas

// --- Funciones de pre-procesamiento ---

void loadCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo: " << filename << std::endl;
        exit(1); // Salir si no se puede abrir el archivo
    }

    std::string line;
    // Leer y descartar la primera línea (encabezados)
    std::getline(file, line);

    int personCount = 0;
    int songCount = 0;

    // Primera pasada: Identificar todas las personas y canciones y almacenar ratings crudos
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string personId, songId, scoreStr, randomData;
        double score;

        if (std::getline(ss, personId, ',') &&
            std::getline(ss, songId, ',') &&
            std::getline(ss, scoreStr, ',') &&
            std::getline(ss, randomData)) // Leer el resto de la línea como randomData
        {
            try {
                score = std::stod(scoreStr);

                // Obtener o asignar indice para personId
                int personIndex;
                if (personaMap.find(personId) == personaMap.end()) {
                    personIndex = personCount++;
                    personaMap[personId] = personIndex;
                    personas.push_back({personId}); // Agregar nueva persona
                } else {
                    personIndex = personaMap[personId];
                }

                // Obtener o asignar indice para songId
                int songIndex;
                if (cancionMap.find(songId) == cancionMap.end()) {
                    songIndex = songCount++;
                    cancionMap[songId] = songIndex;
                    canciones.push_back({songId}); // Agregar nueva cancion
                } else {
                    songIndex = cancionMap[songId];
                }

                // Almacenar la calificación cruda
                rawRatings.push_back({personIndex, songIndex, score});

            } catch (const std::invalid_argument& ia) {
                std::cerr << "Advertencia: Salteando fila con puntuacion no valida: " << scoreStr << " en linea: " << line << std::endl;
            } catch (const std::out_of_range& oor) {
                std::cerr << "Advertencia: Salteando fila con puntuacion fuera de rango: " << scoreStr << " en linea: " << line << std::endl;
            }
        } else {
             std::cerr << "Advertencia: Salteando linea con formato incorrecto: " << line << std::endl;
        }
    }

    file.close();

    std::cout << "Pre-procesamiento: Leidas " << rawRatings.size() << " calificaciones." << std::endl;
    std::cout << "Pre-procesamiento: Identificadas " << personas.size() << " personas y " << canciones.size() << " canciones." << std::endl;

    // Inicializar vectorSentimiento para todas las personas y canciones
    // Segunda pasada: Llenar vectorSentimiento y actualizar datos de canciones
    int numSongs = canciones.size();
    for (auto& p : personas) {
        p.vectorSentimiento.assign(numSongs, 2.5); // Inicializar con 2.5
    }

    for (const auto& rating : rawRatings) {
        int pIndex = rating.personIndex;
        int sIndex = rating.songIndex;
        double score = rating.score;

        // Asegurarse de que los indices son validos
        if (pIndex < personas.size() && sIndex < canciones.size()) {
             // Llenar el vectorSentimiento con la puntuacion real
            personas[pIndex].vectorSentimiento[sIndex] = score;

            // Actualizar datos de la cancion
            canciones[sIndex].totalRatings++;
            canciones[sIndex].sumScores += score;
            if (score > 4.0) {
                canciones[sIndex].successes++;
                 // Añadir a cancionesGustar (limite de 50 si se quiere imponer, pero con vector no es necesario el limite fijo)
                personas[pIndex].cancionesGustar.push_back(sIndex);
            }
        } else {
            std::cerr << "Error interno: Indice de persona o cancion fuera de rango." << std::endl;
        }
    }

    // Calcular popularidad, disfrute y probGustar para todas las canciones
    double totalPersonsDouble = static_cast<double>(personas.size());
    for (auto& c : canciones) {
        if (c.totalRatings > 0) {
            c.popularidad = (static_cast<double>(c.totalRatings) / totalPersonsDouble) * 100.0;
            c.disfrute = c.sumScores / static_cast<double>(c.totalRatings);
            // Rule of Succession para probGustar general
            c.probGustar = (static_cast<double>(c.successes) + 1.0) / (static_cast<double>(c.totalRatings) + 2.0);
        } else {
             // Si no hay ratings, las probabilidades son bajas o 0
            c.popularidad = 0.0;
            c.disfrute = 0.0; // O algun valor neutral
            c.probGustar = (0.0 + 1.0) / (0.0 + 2.0); // (1/2) = 0.5 con Rule of Succession sin datos
        }
    }

    std::cout << "Pre-procesamiento completado." << std::endl;
}

#endif // ARREGLO_PERSONAS_CPP

    