import subprocess
import os
import csv
import re
import glob
from typing import List, Dict

# --- CONFIGURAÇÃO ---

# Nome do arquivo CSV de saída
OUTPUT_CSV: str = 'benchmark_results.csv'

# Número de vezes para executar cada combinação de binário/thread
N_TIMES_RUN: int = 10

# Número máximo de threads para testar (vai de 1 até MAX_THREADS)
MAX_THREADS: int = 16

# --- Parâmetros para 'n' ---
N_MIN: int = 1_000_000_000
N_MAX: int = 1_000_000_000
N_STEP: int = N_MIN

# Timeout em segundos para cada execução
RUN_TIMEOUT: float = 600

def get_binary_info(binary_path: str) -> Dict[str, str]:
    """Extract optimization and SIMD information from binary name."""
    basename = os.path.basename(binary_path)
    info = {
        'base_name': basename.split('_-O')[0],
        'opt_level': 'none',
        'simd': 'none'
    }
    
    if '-O' in basename:
        opt_match = re.search(r'-O(\d)', basename)
        if opt_match:
            info['opt_level'] = f"O{opt_match.group(1)}"
    
    if '-msse' in basename:
        info['simd'] = 'sse'
    elif '-mavx2' in basename:
        info['simd'] = 'avx2'
    
    return info

def find_binaries() -> List[str]:
    """Find all compiled binaries in the current directory."""
    # Exclude source files and the benchmark script itself
    exclusions = {'.c', '.py', '.csv', '.txt', '.md', 'Makefile'}
    return [f for f in glob.glob('./*') if os.path.isfile(f) and 
            not any(f.endswith(ext) for ext in exclusions)]

def run_benchmark():
    """
    Executa o benchmark para todos os binários, variando o número de threads,
    e salva os resultados em um arquivo CSV.
    """
    time_regex = re.compile(r"Tempo de execução: (\d+\.\d+) segundos")
    binaries = find_binaries()
    
    if not binaries:
        print("Nenhum binário encontrado! Execute 'make' primeiro.")
        return
    
    print(f"Iniciando benchmarks... Resultados serão salvos em '{OUTPUT_CSV}'")
    
    try:
        with open(OUTPUT_CSV, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(['binary_name', 'base_name', 'opt_level', 'simd', 
                           'n_iterations', 'n_threads', 'run_number', 'execution_time_s'])
            
            for binary_path in sorted(binaries):
                binary_info = get_binary_info(binary_path)
                
                if not os.path.exists(binary_path):
                    continue
                    
                print(f"\n--- Testando Binário: {binary_path} ---")
                print(f"    Base: {binary_info['base_name']}")
                print(f"    Otimização: {binary_info['opt_level']}")
                print(f"    SIMD: {binary_info['simd']}")
                
                for n in range(N_MIN, N_MAX + 1, N_STEP):
                    print(f"  Testando com n = {n:,}...")
                    input_data = f"{n}\n"
                
                    for thread_count in range(1, MAX_THREADS + 1):
                        print(f"    Testando com {thread_count} thread(s)...")
                        
                        for run in range(1, N_TIMES_RUN + 1):
                            try:
                                env = os.environ.copy()
                                env['OMP_NUM_THREADS'] = str(thread_count)
                                
                                result = subprocess.run(
                                    [binary_path],
                                    input=input_data,
                                    capture_output=True,
                                    text=True,
                                    env=env,
                                    timeout=RUN_TIMEOUT,
                                    check=False
                                )
                                
                                if result.returncode != 0:
                                    print(f"    Run {run}/{N_TIMES_RUN}: FALHOU (código de saída {result.returncode})")
                                    print(f"      Stderr: {result.stderr.strip()}")
                                    continue

                                match = time_regex.search(result.stdout)
                                
                                if match:
                                    exec_time = float(match.group(1))
                                    print(f"      Run {run}/{N_TIMES_RUN}: {exec_time:.6f} s")
                                    writer.writerow([
                                        binary_path,
                                        binary_info['base_name'],
                                        binary_info['opt_level'],
                                        binary_info['simd'],
                                        n,
                                        thread_count,
                                        run,
                                        exec_time
                                    ])
                                else:
                                    print(f"      Run {run}/{N_TIMES_RUN}: FALHOU (não foi possível extrair o tempo)")
                                    print(f"        Stdout: {result.stdout.strip()}")

                            except subprocess.TimeoutExpired:
                                print(f"      Run {run}/{N_TIMES_RUN}: FALHOU (timeout de {RUN_TIMEOUT}s atingido)")
                            except Exception as e:
                                print(f"      Run {run}/{N_TIMES_RUN}: FALHOU (exceção: {e})")

    except IOError as e:
        print(f"Erro fatal ao escrever o arquivo CSV: {e}")
    except Exception as e:
        print(f"Um erro inesperado ocorreu: {e}")

    print(f"\nBenchmarks concluídos. Resultados salvos em '{OUTPUT_CSV}'.")

if __name__ == "__main__":
    run_benchmark()

