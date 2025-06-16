#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <set>
#include <algorithm>
#include <limits> // Para std::numeric_limits

// Incluir el archivo de pre-procesamiento. Esto es necesario según la estructura
// de archivos que solicitaste, aunque no es una práctica estándar en C++.
#include "estructurasIniciales/arregloPersonas.cpp"

// Constantes (basadas en las descripciones del problema)
const int NUM_SIMILAR_USERS = 10;
const int MIN_LIKES_IN_CIRCLE = 5; // Minimo de personas en el circulo que les guste una cancion candidata
const int NUM_TOP_GENERAL_SONGS = 50;
const int NUM_RECOMMENDATIONS = 10;
// Denominador de P(B) según la descripción del problema
const double PB_DENOMINATOR = 12.0;

// --- Funciones de procesamiento ---

// Calcula la distancia euclidiana entre los vectores sentimiento de dos personas
double calculateEuclideanDistance(int pIndex1, int pIndex2) {
    if (pIndex1 < 0 || pIndex1 >= personas.size() || pIndex2 < 0 || pIndex2 >= personas.size()) {
        return std::numeric_limits<double>::infinity(); // Indices invalidos
    }

    const auto& v1 = personas[pIndex1].vectorSentimiento;
    const auto& v2 = personas[pIndex2].vectorSentimiento;

    if (v1.size() != v2.size()) {
        // Esto no deberia ocurrir si loadCSV funciona correctamente
        std::cerr << "Error: Los vectores de sentimiento tienen tamanos diferentes." << std::endl;
        return std::numeric_limits<double>::infinity();
    }

    double sum_sq = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        sum_sq += std::pow(v1[i] - v2[i], 2);
    }
    return std::sqrt(sum_sq);
}

// Encuentra los indices de las personas más parecidas al sujeto
std::vector<int> findSimilarUsers(int subjectIndex) {
    std::vector<std::pair<double, int>> distances; // pair: {distance, personIndex}

    for (size_t i = 0; i < personas.size(); ++i) {
        if (static_cast<int>(i) == subjectIndex) continue; // No comparar con uno mismo
        double dist = calculateEuclideanDistance(subjectIndex, i);
        distances.push_back({dist, static_cast<int>(i)});
    }

    // Ordenar por distancia (ascendente)
    std::sort(distances.begin(), distances.end());

    // Tomar los top N (NUM_SIMILAR_USERS)
    std::vector<int> similarUserIndices;
    for (int i = 0; i < NUM_SIMILAR_USERS && i < distances.size(); ++i) {
        similarUserIndices.push_back(distances[i].second);
    }

    return similarUserIndices;
}

// Identifica las canciones candidatas para la recomendación
std::set<int> getCandidateSongs(const std::vector<int>& similarUserIndices) {
    std::set<int> candidateSongIndices;

    // Candidatas del circulo de parecidos (canciones que les gustan a >= MIN_LIKES_IN_CIRCLE)
    std::map<int, int> songLikedCountInCircle; // map: {songIndex, count}
    for (int personIndex : similarUserIndices) {
        // Iterar por las canciones que le gustan a cada persona parecida
        for (int songIndex : personas[personIndex].cancionesGustar) {
            songLikedCountInCircle[songIndex]++;
        }
    }

    for (const auto& pair : songLikedCountInCircle) {
        if (pair.second >= MIN_LIKES_IN_CIRCLE) {
            candidateSongIndices.insert(pair.first);
        }
    }

    // Candidatas de las top canciones generales (por probGustar)
    std::vector<std::pair<double, int>> songsByProbGustar; // pair: {probGustar, songIndex}
    for (size_t i = 0; i < canciones.size(); ++i) {
        songsByProbGustar.push_back({canciones[i].probGustar, static_cast<int>(i)});
    }

    // Ordenar por probGustar (descendente)
    std::sort(songsByProbGustar.begin(), songsByProbGustar.end(),
              [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                  return a.first > b.first; // Orden descendente
              });

    // Tomar las top N (NUM_TOP_GENERAL_SONGS)
    for (int i = 0; i < NUM_TOP_GENERAL_SONGS && i < songsByProbGustar.size(); ++i) {
        candidateSongIndices.insert(songsByProbGustar[i].second);
    }

    return candidateSongIndices;
}

// Calcula la probabilidad final para cada cancion candidata para el sujeto
std::vector<std::pair<double, int>> calculateRecommendationProbabilities(
    int subjectIndex,
    const std::set<int>& candidateSongIndices,
    const std::vector<int>& similarUserIndices)
{
    std::vector<std::pair<double, int>> recommendationProbabilities; // pair: {probability, songIndex}

    for (int songIndex : candidateSongIndices) {
        if (songIndex < 0 || songIndex >= canciones.size()) {
             std::cerr << "Advertencia: Indice de cancion candidata invalido: " << songIndex << std::endl;
             continue;
        }

        // P(A) = probGustar general de la cancion
        double pA = canciones[songIndex].probGustar;

        // P(B) = Probabilidad basada en el circulo de parecidos (usando Rule of Succession)
        int aciertosInCircle = 0; // Cuantos en el circulo le dieron > 4
        // int participacionesInCircle = 0; // Cuantos en el circulo calificaron la cancion (No necesario con el denominador fijo de 12)

        for (int simPersonIndex : similarUserIndices) {
            if (simPersonIndex < 0 || simPersonIndex >= personas.size()) continue; // Sim person index invalid

            // Verificar si esta persona parecida calificó la cancion
            // En nuestro vectorSentimiento, 2.5 es el valor por defecto para "no calificado"
            if (personas[simPersonIndex].vectorSentimiento.size() > songIndex &&
                personas[simPersonIndex].vectorSentimiento[songIndex] != 2.5)
            {
                // participacionesInCircle++; // Contar si calificó

                // Verificar si le gustó (> 4)
                if (personas[simPersonIndex].vectorSentimiento[songIndex] > 4.0) {
                    aciertosInCircle++;
                }
            }
        }

        // P(B) = (aciertos en circulo + 1 / 12) según la descripción
        double pB = (static_cast<double>(aciertosInCircle) + 1.0) / PB_DENOMINATOR;

        // Probabilidad final = 0.5 * (P(A) + P(B))
        double finalProbability = 0.5 * (pA + pB);

        recommendationProbabilities.push_back({finalProbability, songIndex});
    }

    return recommendationProbabilities;
}

int main() {
    std::cout << "Cargando datos desde data.csv..." << std::endl;
    loadCSV("./estructurasIniciales/data.csv");

    std::string subjectId;
    std::cout << "\nIngrese el ID de la persona para la cual desea recomendaciones: ";
    std::cin >> subjectId;

    // Encontrar el indice del sujeto
    auto it = personaMap.find(subjectId);
    if (it == personaMap.end()) {
        std::cerr << "Error: No se encontro ninguna persona con el ID '" << subjectId << "'." << std::endl;
        return 1; // Salir si el sujeto no es encontrado
    }
    int subjectIndex = it->second;

    std::cout << "Sujeto encontrado. ID: " << personas[subjectIndex].id << std::endl;

    // 1. Hallar el círculo de parecidos
    std::cout << "Buscando usuarios similares..." << std::endl;
    std::vector<int> similarUserIndices = findSimilarUsers(subjectIndex);

    std::cout << "Encontrados " << similarUserIndices.size() << " usuarios similares." << std::endl;
    // Opcional: Imprimir IDs de usuarios similares
    // std::cout << "IDs de usuarios similares: ";
    // for(int idx : similarUserIndices) std::cout << personas[idx].id << " ";
    // std::cout << std::endl;


    // 2. Decidir canciones candidatas
    std::cout << "Identificando canciones candidatas..." << std::endl;
    std::set<int> candidateSongIndices = getCandidateSongs(similarUserIndices);

    std::cout << "Encontradas " << candidateSongIndices.size() << " canciones candidatas." << std::endl;

    // 3. Calcular probabilidad para cada candidata
    std::cout << "Calculando probabilidades de recomendacion para las candidatas..." << std::endl;
    std::vector<std::pair<double, int>> recommendationProbabilities =
        calculateRecommendationProbabilities(subjectIndex, candidateSongIndices, similarUserIndices);

    std::cout << "Calculadas probabilidades para " << recommendationProbabilities.size() << " canciones candidatas." << std::endl;

    // 4. Ordenar y seleccionar las 10 mejores
    std::cout << "Seleccionando las mejores " << NUM_RECOMMENDATIONS << " recomendaciones..." << std::endl;
    std::sort(recommendationProbabilities.begin(), recommendationProbabilities.end(),
              [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                  return a.first > b.first; // Orden descendente por probabilidad
              });

    // 5. Mostrar las recomendaciones
    std::cout << "\n--- Top " << NUM_RECOMMENDATIONS << " Recomendaciones para " << personas[subjectIndex].id << " ---" << std::endl;
    int count = 0;
    for (const auto& rec : recommendationProbabilities) {
        if (count < NUM_RECOMMENDATIONS) {
            int songIndex = rec.second;
            double probability = rec.first;
            // Asegurarse de que el indice de la cancion es valido antes de acceder
            if (songIndex >= 0 && songIndex < canciones.size()) {
                 std::cout << (count + 1) << ". " << canciones[songIndex].id
                          << " (Probabilidad: " << probability << ", Popularidad General: " << canciones[songIndex].popularidad
                          << "%, Disfrute Promedio: " << canciones[songIndex].disfrute << ")" << std::endl;
                 count++;
            } else {
                 std::cerr << "Advertencia: Cancion recomendada con indice invalido: " << songIndex << std::endl;
            }
        } else {
            break; // Ya tenemos las 10 recomendaciones
        }
    }

    if (count == 0) {
        std::cout << "No se pudieron generar recomendaciones suficientes." << std::endl;
    }

    return 0;
}