#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

// Códigos ANSI para cores
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

// Classe que encapsula o gerenciamento da fila de entrevistas
class InterviewQueue {
private:
    std::queue<int> interviewQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    bool poisonPillSent = false;

public:
    // Adiciona uma entrevista à fila
    void addInterview(int duration) {
        std::lock_guard<std::mutex> lock(queueMutex);
        interviewQueue.push(duration);
        queueCV.notify_one(); // Notifica o consumidor
    }

    // Adiciona o Poison Pill à fila
    void addPoisonPill() {
        std::lock_guard<std::mutex> lock(queueMutex);
        interviewQueue.push(-1);  // -1 é o valor que representa a Poison Pill
        queueCV.notify_one();
    }

    // Retira uma entrevista da fila, aguardando se necessário
    bool getInterview(int &duration) {
        std::unique_lock<std::mutex> lock(queueMutex);
        // Espera até que haja algo na fila
        queueCV.wait(lock, [this] { return !interviewQueue.empty(); });

        duration = interviewQueue.front();
        interviewQueue.pop();
        return true;
    }

    // Verifica se a Poison Pill foi enviada
    bool isPoisonPill(int value) {
        return value == -1;
    }
};

// Gerador de números aleatórios
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(1, 3);  // Número de entrevistas
std::uniform_int_distribution<> durationDist(1, 5); // Duração da entrevista

// Função dos produtores (Reporters)
void reporter(InterviewQueue &queue, const std::string &name) {
    int numInterviews = dist(gen); // Quantidade de entrevistas
    for (int i = 0; i < numInterviews; ++i) {
        int duration = durationDist(gen); // Duração da entrevista
        std::cout << GREEN << name << " iniciando gravação...\n" << RESET;
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        std::cout << GREEN << name << " finalizei gravação. Duração: " << duration << " segundos.\n" << RESET;

        // Adiciona à fila
        queue.addInterview(duration);

        // Sleep após adicionar na fila
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep pequeno
    }
}

// Função do consumidor (Tela)
void screen(InterviewQueue &queue) {
    using namespace std::chrono;

    while (true) {
        int duration = 0;

        // Tenta pegar uma entrevista da fila
        queue.getInterview(duration);

        // Se for a Poison Pill, termina o loop
        if (queue.isPoisonPill(duration)) {
            std::cout << RED << "Recebido Poison Pill. Finalizando consumidor...\n" << RESET;
            break;
        }

        std::cout << BLUE << "Tela: Iniciando transmissão. Duração: " << duration << " segundos.\n" << RESET;
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        std::cout << BLUE << "Tela: Finalizei transmissão. Duração: " << duration << " segundos.\n" << RESET;
    }
}

int main() {
    InterviewQueue queue; // Instância da classe InterviewQueue

    // Cria as threads dos produtores
    std::thread reporter1(reporter, std::ref(queue), "Reporter 1");
    std::thread reporter2(reporter, std::ref(queue), "Reporter 2");
    std::thread reporter3(reporter, std::ref(queue), "Reporter 3");

    // Cria a thread do consumidor
    std::thread screenThread(screen, std::ref(queue));

    // Aguarda os produtores terminarem
    reporter1.join();
    reporter2.join();
    reporter3.join();

    // Envia a Poison Pill para o consumidor
    queue.addPoisonPill();

    // Aguarda a thread do consumidor terminar
    screenThread.join();

    return 0;
}
