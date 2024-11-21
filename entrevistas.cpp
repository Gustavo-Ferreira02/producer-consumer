#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

// Fila protegida
std::queue<int> interviewQueue;
std::mutex queueMutex;
std::condition_variable queueCV;

// Gerador de números aleatórios
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(1, 3);  // Número de entrevistas
std::uniform_int_distribution<> durationDist(1, 5); // Duração da entrevista

// Função dos produtores (Reporters)
void reporter(const std::string &name) {
    int numInterviews = dist(gen); // Quantidade de entrevistas
    for (int i = 0; i < numInterviews; ++i) {
        int duration = durationDist(gen); // Duração da entrevista
        std::cout << name << " iniciando gravação...\n";
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        std::cout << name << " finalizei gravação. Duração: " << duration << " segundos.\n";

        // Adiciona à fila
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            interviewQueue.push(duration);
        }
        queueCV.notify_one();
    }
}

// Função do consumidor (Tela)
void screen() {
    using namespace std::chrono;

    auto lastInteraction = steady_clock::now();

    while (true) {
        int duration = 0;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Espera por um item na fila, mas com um tempo limite de 10 segundos
            if (queueCV.wait_for(lock, seconds(10), [] { return !interviewQueue.empty(); })) {
                duration = interviewQueue.front();
                interviewQueue.pop();
                lastInteraction = steady_clock::now();  // Reinicia o tempo de interação
            } else {
                std::cout << "Fila vazia por mais de 10 segundos. Finalizando...\n";
                break;
            }
        }

        if (duration > 0) {
            std::cout << "Tela: Iniciando transmissão. Duração: " << duration << " segundos.\n";
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            std::cout << "Tela: Finalizei transmissão. Duração: " << duration << " segundos.\n";
        }
    }
}

int main() {
    // Cria as threads dos produtores
    std::thread reporter1(reporter, "Reporter 1");
    std::thread reporter2(reporter, "Reporter 2");
    std::thread reporter3(reporter, "Reporter 3");

    // Cria a thread do consumidor
    std::thread screenThread(screen);

    // Aguarda os produtores terminarem
    reporter1.join();
    reporter2.join();
    reporter3.join();

    // Aguarda a thread do consumidor terminar
    screenThread.join();

    return 0;
}
