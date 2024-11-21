import threading
import time
import random
import queue

# Códigos ANSI para cores
RESET   = "\033[0m"
GREEN   = "\033[32m"
BLUE    = "\033[34m"
RED     = "\033[31m"

# Classe que encapsula o gerenciamento da fila de entrevistas
class InterviewQueue:
    def __init__(self):
        self.queue = queue.Queue()
        self.lock = threading.Lock()
        self.cv = threading.Condition(self.lock)

    # Adiciona uma entrevista à fila
    def add_interview(self, duration):
        with self.lock:
            self.queue.put(duration)
            self.cv.notify()  # Notifica o consumidor

    # Retira uma entrevista da fila, aguardando se necessário
    def get_interview(self, max_wait_seconds=10):
        with self.lock:
            # Espera até 10 segundos para que a fila tenha algo
            end_time = time.time() + max_wait_seconds
            while self.queue.empty() and time.time() < end_time:
                self.cv.wait(timeout=max_wait_seconds)

            if not self.queue.empty():
                return self.queue.get()
            else:
                return None  # Retorna None se a fila estiver vazia após o tempo de espera

# Função dos produtores (Reporters)
def reporter(queue, name):
    num_interviews = random.randint(1, 3)  # Número de entrevistas aleatórias
    for _ in range(num_interviews):
        duration = random.randint(1, 5)  # Duração aleatória da entrevista
        print(GREEN + f"{name} iniciando gravação...")
        time.sleep(duration)
        print(GREEN + f"{name} finalizei gravação. Duração: {duration} segundos.")

        # Adiciona à fila
        queue.add_interview(duration)

        # Sleep pequeno após adicionar na fila
        time.sleep(0.5)  # Simula um pequeno atraso após adicionar à fila

# Função do consumidor (Tela)
def screen(queue):
    while True:
        duration = queue.get_interview()

        if duration is None:
            print(RED + "Fila vazia por mais de 10 segundos. Finalizando...")
            break

        print(BLUE + f"Tela: Iniciando transmissão. Duração: {duration} segundos.")
        time.sleep(duration)
        print(BLUE + f"Tela: Finalizei transmissão. Duração: {duration} segundos.")

def main():
    interview_queue = InterviewQueue()

    # Cria as threads dos produtores
    reporter1 = threading.Thread(target=reporter, args=(interview_queue, "Reporter 1"))
    reporter2 = threading.Thread(target=reporter, args=(interview_queue, "Reporter 2"))
    reporter3 = threading.Thread(target=reporter, args=(interview_queue, "Reporter 3"))

    # Cria a thread do consumidor
    screen_thread = threading.Thread(target=screen, args=(interview_queue,))

    # Inicia as threads
    reporter1.start()
    reporter2.start()
    reporter3.start()
    screen_thread.start()

    # Aguarda os produtores terminarem
    reporter1.join()
    reporter2.join()
    reporter3.join()

    # Aguarda a thread do consumidor terminar
    screen_thread.join()

if __name__ == "__main__":
    main()
