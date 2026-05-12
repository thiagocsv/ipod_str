import os
import shutil
import struct
import random
import string
from mutagen.easyid3 import EasyID3

PASTA_ORIGEM = os.path.dirname(os.path.abspath(__file__))
PASTA_DESTINO_SD = os.path.join(PASTA_ORIGEM, "sdcard", "music")
TOTAL_PASTAS = 50

def gerar_nome_3_letras():
    return ''.join(random.choice(string.ascii_uppercase) for _ in range(3))

def preparar_pastas_destino():
    if not os.path.exists(PASTA_DESTINO_SD):
        os.makedirs(PASTA_DESTINO_SD)
    for i in range(TOTAL_PASTAS):
        pasta = os.path.join(PASTA_DESTINO_SD, f"F{i:02d}")
        if not os.path.exists(pasta):
            os.makedirs(pasta)

def extrair_metadados(caminho_arquivo):
    try:
        audio = EasyID3(caminho_arquivo)
        artista = audio.get('artist', ['Artista Desconhecido'])[0]
        titulo = audio.get('title', ['Título Desconhecido'])[0]
        return artista, titulo
    except Exception:
        return "Desconhecido", os.path.basename(caminho_arquivo).replace('.mp3', '')

def empacotar_registro(artista, titulo, id_pasta, nome_3letras):
    artista_bytes = artista.encode('utf-8', 'ignore')[:30].ljust(30, b'\x00')
    titulo_bytes = titulo.encode('utf-8', 'ignore')[:30].ljust(30, b'\x00')
    nome_bytes = nome_3letras.encode('ascii')
    return struct.pack('<30s 30s B 3s', artista_bytes, titulo_bytes, id_pasta, nome_bytes)

def carregar_banco_existente(caminho_db):
    """Lê o banco de dados antigo e retorna o estado atual."""
    registros_binarios = []
    nomes_usados_por_pasta = {i: set() for i in range(TOTAL_PASTAS)}
    musicas_existentes = set() # Guarda tuplas (Artista, Titulo)
    total_musicas = 0

    if os.path.exists(caminho_db):
        with open(caminho_db, 'rb') as f:
            # Lê os primeiros 4 bytes (Total)
            cabecalho = f.read(4)
            if cabecalho:
                total_musicas = struct.unpack('<I', cabecalho)[0]

                # Lê cada registro de 64 bytes
                for _ in range(total_musicas):
                    registro = f.read(64)
                    if len(registro) == 64:
                        registros_binarios.append(registro)

                        # Desempacota para descobrir o que já existe
                        art_b, tit_b, id_pasta, nome_b = struct.unpack('<30s 30s B 3s', registro)

                        artista = art_b.decode('utf-8', 'ignore').rstrip('\x00')
                        titulo = tit_b.decode('utf-8', 'ignore').rstrip('\x00')
                        nome_3letras = nome_b.decode('ascii')

                        nomes_usados_por_pasta[id_pasta].add(nome_3letras)
                        musicas_existentes.add((artista, titulo))

    return total_musicas, registros_binarios, nomes_usados_por_pasta, musicas_existentes

def main():
    print(f"Lendo músicas do diretório: {PASTA_ORIGEM}")
    preparar_pastas_destino()

    caminho_db = os.path.join(PASTA_DESTINO_SD, "index.bin")

    # 1. Carrega o estado do banco de dados (se ele existir)
    total_antigo, registros_binarios, nomes_usados_por_pasta, musicas_existentes = carregar_banco_existente(caminho_db)

    if total_antigo > 0:
        print(f"Banco de dados encontrado! {total_antigo} músicas já indexadas.")
    else:
        print("Nenhum banco de dados encontrado. Criando um novo...")

    # 2. Varre as músicas do diretório atual
    arquivos_mp3 = [f for f in os.listdir(PASTA_ORIGEM) if f.lower().endswith('.mp3')]

    if not arquivos_mp3:
        print("Nenhum arquivo MP3 novo encontrado nesta pasta.")
        return

    novas_adicionadas = 0

    for arquivo in arquivos_mp3:
        caminho_origem = os.path.join(PASTA_ORIGEM, arquivo)
        artista, titulo = extrair_metadados(caminho_origem)

        # 3. Pula a música se ela já estiver no banco de dados antigo
        if (artista, titulo) in musicas_existentes:
            print(f"Ignorado (já existe): [{artista[:15]} - {titulo[:15]}]")
            continue

        # 4. Define a pasta continuando o Round-Robin de onde parou
        id_pasta = (total_antigo + novas_adicionadas) % TOTAL_PASTAS

        # 5. Gera o nome checando colisões contra o que já existe no cartão
        nome_3letras = gerar_nome_3_letras()
        while nome_3letras in nomes_usados_por_pasta[id_pasta]:
            nome_3letras = gerar_nome_3_letras()

        nomes_usados_por_pasta[id_pasta].add(nome_3letras)

        # 6. Copia
        nome_arquivo_destino = f"{nome_3letras}.mp3"
        caminho_destino = os.path.join(PASTA_DESTINO_SD, f"F{id_pasta:02d}", nome_arquivo_destino)

        shutil.copy2(caminho_origem, caminho_destino)
        print(f"Adicionado: [{artista[:15]} - {titulo[:15]}] -> F{id_pasta:02d}/{nome_arquivo_destino}")

        # 7. Salva o registro novo
        registro_bytes = empacotar_registro(artista, titulo, id_pasta, nome_3letras)
        registros_binarios.append(registro_bytes)
        novas_adicionadas += 1

    # 8. Regrava o banco de dados inteiro (antigos + novos)
    if novas_adicionadas > 0:
        total_final = total_antigo + novas_adicionadas
        with open(caminho_db, 'wb') as db_file:
            db_file.write(struct.pack('<I', total_final))
            for registro in registros_binarios:
                db_file.write(registro)

        print(f"\nSucesso! {novas_adicionadas} músicas novas adicionadas.")
        print(f"Total agora no cartão: {total_final} músicas.")
    else:
        print("\nNenhuma música nova foi adicionada. O cartão já está atualizado.")

if __name__ == "__main__":
    main()
