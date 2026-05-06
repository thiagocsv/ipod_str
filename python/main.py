import time
import threading
import queue
import random
from dataclasses import dataclass, field



SIMULATION_TIME = 15.0  # segundos

AUDIO_BUFFER_MAX_SIZE = 20
AUDIO_BUFFER_MIN_LEVEL = 4

# Períodos das tarefas, em segundos
PERIOD_AUDIO_PLAYBACK = 0.010      # 10 ms
PERIOD_AUDIO_DECODE = 0.020        # 20 ms
PERIOD_INTERFACE = 0.500           # 500 ms
PERIOD_BUTTON_READER = 0.100       # 100 ms

# Deadlines, em segundos
DEADLINE_AUDIO_PLAYBACK = 0.010
DEADLINE_AUDIO_DECODE = 0.020
DEADLINE_INTERFACE = 0.500
DEADLINE_BUTTON_READER = 0.100


@dataclass
class PlayerState:
    playing: bool = True
    current_track: int = 1
    volume: int = 50
    elapsed_time: float = 0.0
    running: bool = True
    lock: threading.Lock = field(default_factory=threading.Lock)


player_state = PlayerState()

audio_buffer = queue.Queue(maxsize=AUDIO_BUFFER_MAX_SIZE)
command_queue = queue.Queue()

stats_lock = threading.Lock()

task_stats = {
    "audio_playback": {
        "executions": 0,
        "deadline_misses": 0,
        "max_execution_time": 0.0,
    },
    "audio_decode": {
        "executions": 0,
        "deadline_misses": 0,
        "max_execution_time": 0.0,
    },
    "interface": {
        "executions": 0,
        "deadline_misses": 0,
        "max_execution_time": 0.0,
    },
    "button_reader": {
        "executions": 0,
        "deadline_misses": 0,
        "max_execution_time": 0.0,
    },
    "command_handler": {
        "executions": 0,
        "deadline_misses": 0,
        "max_execution_time": 0.0,
    },
}


def update_stats(task_name, execution_time, deadline):

    # Atualiza métricas da tarefa: número de execuções; maior tempo de execução observado; número de deadlines perdidos.
    
    with stats_lock:
        task_stats[task_name]["executions"] += 1

        if execution_time > task_stats[task_name]["max_execution_time"]:
            task_stats[task_name]["max_execution_time"] = execution_time

        if execution_time > deadline:
            task_stats[task_name]["deadline_misses"] += 1


def busy_work(duration):
    # Pausa para simular o tempo de processamento;

    time.sleep(duration)


def periodic_task_loop(task_function, period, task_name):

    next_release = time.perf_counter()

    while player_state.running:
        start = time.perf_counter()

        task_function()

        finish = time.perf_counter()
        execution_time = finish - start

        next_release += period
        sleep_time = next_release - time.perf_counter()

        if sleep_time > 0:
            time.sleep(sleep_time)
        else:
            # Se sleep_time <= 0, a tarefa atrasou o próximo ciclo.
            # A simulação continua, mas isso quer dizer que atrasou. ??? VERIFICAR!
            next_release = time.perf_counter()


def audio_playback_task():
    
    # Tarefa periódica de alta prioridade.
    # Consome do buffer de áudio.
    
    start = time.perf_counter()

    with player_state.lock:
        is_playing = player_state.playing

    if is_playing:
        try:
            audio_block = audio_buffer.get_nowait()

            # Simula envio do bloco ao dispositivo de áudio.
            busy_work(0.002)  # 2 ms

            with player_state.lock:
                player_state.elapsed_time += PERIOD_AUDIO_PLAYBACK

        except queue.Empty:
            print("[ERRO] Buffer vazio: falha na reprodução!")

    execution_time = time.perf_counter() - start
    update_stats("audio_playback", execution_time, DEADLINE_AUDIO_PLAYBACK)


def audio_decode_task():

    # Tarefa periódica.
    # Produz trechos de áudio e joga no buffer.

    start = time.perf_counter()

    with player_state.lock:
        is_playing = player_state.playing

    if is_playing:
        if audio_buffer.qsize() < AUDIO_BUFFER_MAX_SIZE:
            # Simula tempo variável de decodificação.
            decode_time = random.uniform(0.003, 0.007)
            busy_work(decode_time)

            try:
                audio_buffer.put_nowait("audio_block")
            except queue.Full:
                pass

    execution_time = time.perf_counter() - start
    update_stats("audio_decode", execution_time, DEADLINE_AUDIO_DECODE)


def interface_task():

    start = time.perf_counter()

    with player_state.lock:
        status = "tocando" if player_state.playing else "pausado"
        track = player_state.current_track
        volume = player_state.volume
        elapsed = player_state.elapsed_time

    buffer_level = audio_buffer.qsize()

    print(
        f"[INTERFACE] Faixa: {track} | Estado: {status} | "
        f"Tempo: {elapsed:.2f}s | Volume: {volume}% | "
        f"Buffer: {buffer_level}/{AUDIO_BUFFER_MAX_SIZE}"
    )

    busy_work(0.005)  # 5 ms

    execution_time = time.perf_counter() - start
    update_stats("interface", execution_time, DEADLINE_INTERFACE)


def button_reader_task():

    # Simula uma tarefa periódica que lê botões.

    start = time.perf_counter()

    possible_commands = [
        None,
        None,
        None,
        "pause",
        "play",
        "next",
        "volume_up",
        "volume_down",
    ]

    command = random.choice(possible_commands) #Representação de pressionar um botão do ipod.

    if command is not None:
        command_queue.put(command)

    busy_work(0.001)  # 1 ms

    execution_time = time.perf_counter() - start
    update_stats("button_reader", execution_time, DEADLINE_BUTTON_READER)


def command_handler_task():

    while player_state.running:
        try:
            command = command_queue.get(timeout=0.1)
        except queue.Empty:
            continue

        start = time.perf_counter()

        with player_state.lock:
            if command == "pause":
                player_state.playing = False
                print("[COMANDO] Pause")

            elif command == "play":
                player_state.playing = True
                print("[COMANDO] Play")

            elif command == "next":
                player_state.current_track += 1
                player_state.elapsed_time = 0.0

                # Esvazia o buffer ao trocar de faixa.
                while not audio_buffer.empty():
                    try:
                        audio_buffer.get_nowait()
                    except queue.Empty:
                        break

                print("[COMANDO] Próxima faixa")

            elif command == "volume_up":
                player_state.volume = min(100, player_state.volume + 5)
                print("[COMANDO] Aumentar volume")

            elif command == "volume_down":
                player_state.volume = max(0, player_state.volume - 5)
                print("[COMANDO] Diminuir volume")

        busy_work(0.004)  # 4 ms

        execution_time = time.perf_counter() - start
        update_stats("command_handler", execution_time, 0.100)


def print_final_report():
    print("\n================ RELATÓRIO FINAL DA SIMULAÇÃO ================\n")

    for task_name, stats in task_stats.items():
        print(f"Tarefa: {task_name}")
        print(f"  Execuções: {stats['executions']}")
        print(f"  Deadlines perdidos: {stats['deadline_misses']}")
        print(f"  Maior tempo de execução observado: {stats['max_execution_time'] * 1000:.3f} ms")
        print()

    print(f"Nível final do buffer: {audio_buffer.qsize()}/{AUDIO_BUFFER_MAX_SIZE}")

    with player_state.lock:
        print(f"Faixa final: {player_state.current_track}")
        print(f"Tempo reproduzido da faixa atual: {player_state.elapsed_time:.2f} s")
        print(f"Volume final: {player_state.volume}%")
        print(f"Estado final: {'tocando' if player_state.playing else 'pausado'}")

    print("\n===============================================================\n")


def main():
    print("Iniciando simulação do tocador de música em tempo real...\n")

    threads = [
        threading.Thread(
            target=periodic_task_loop,
            args=(audio_playback_task, PERIOD_AUDIO_PLAYBACK, "audio_playback"),
            daemon=True,
        ),
        threading.Thread(
            target=periodic_task_loop,
            args=(audio_decode_task, PERIOD_AUDIO_DECODE, "audio_decode"),
            daemon=True,
        ),
        threading.Thread(
            target=periodic_task_loop,
            args=(interface_task, PERIOD_INTERFACE, "interface"),
            daemon=True,
        ),
        threading.Thread(
            target=periodic_task_loop,
            args=(button_reader_task, PERIOD_BUTTON_READER, "button_reader"),
            daemon=True,
        ),
        threading.Thread(
            target=command_handler_task,
            daemon=True,
        ),
    ]

    for thread in threads:
        thread.start()

    time.sleep(SIMULATION_TIME)

    player_state.running = False

    for thread in threads:
        thread.join(timeout=1.0)

    print_final_report()


if __name__ == "__main__":
    main()