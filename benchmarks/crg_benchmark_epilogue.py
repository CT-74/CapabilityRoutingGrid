import os
import csv
import subprocess
import platform
import matplotlib.pyplot as plt
import numpy as np

# --- PATH CONFIGURATION ---
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
EXE = os.path.join(CURRENT_DIR, "bin", "epilogue.bin" if platform.system() != "Windows" else "epilogue.exe")
IMG_DIR = os.path.abspath(os.path.join(CURRENT_DIR, "..", "img"))
DATA_DIR = os.path.join(CURRENT_DIR, "data")

def run_pipeline():
    os.makedirs(os.path.join(CURRENT_DIR, "bin"), exist_ok=True)
    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(IMG_DIR, exist_ok=True)
    
    print("🔨 Compiling...")
    subprocess.run(["clang++", "-O3", "-march=native", "-std=c++17", "crg_benchmark_epilogue.cpp", "-o", EXE], check=True)
    
    print("🚀 Running tests...")
    subprocess.run([EXE], check=True, cwd=CURRENT_DIR)

    colors = {'OOP': '#e74c3c', 'ECS': '#f39c12', 'CRG': '#2ecc71'}
    paradigms = ['OOP', 'ECS', 'CRG']

    # --- IMAGE 1: TIME-SLICING ---
    ts_data = {p: {25: 0, 100: 0} for p in paradigms}
    with open(os.path.join(DATA_DIR, "crg_bench_timeslice.csv"), 'r') as f:
        for r in csv.DictReader(f):
            if int(r['N']) == 1000000:
                ts_data[r['Paradigm']][int(r['FreqPct'])] = float(r['Nrg_uJ'])
    
    fig, ax = plt.subplots(figsize=(10, 7))
    x = np.arange(len(paradigms))
    width = 0.35
    vals_25 = [ts_data[p][25] for p in paradigms]
    vals_100 = [ts_data[p][100] for p in paradigms]

    ax.bar(x - width/2, vals_25, width, label='25% Freq', color=[colors[p] for p in paradigms], alpha=0.4, edgecolor='black', linestyle='--')
    ax.bar(x + width/2, vals_100, width, label='100% Freq', color=[colors[p] for p in paradigms], edgecolor='black')

    ax.set_title("Time-Slicing Dividend (N=1M)\n(Energy Impact)", fontweight='bold', fontsize=14)
    ax.set_xticks(x); ax.set_xticklabels(paradigms, fontweight='bold'); ax.set_ylabel("MicroJoules (µJ)")
    ax.legend(); plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "crg_part5_timeslicing.png"), dpi=200); plt.close()

    # --- IMAGE 2: TRIPLE PANEL ---
    mut_data = {p: {m: {'N':[],'avg':[],'jit':[],'nrg':[]} for m in [1, 5, 10]} for p in paradigms}
    with open(os.path.join(DATA_DIR, "crg_bench_mutation.csv"), 'r') as f:
        for r in csv.DictReader(f):
            p, m = r['Paradigm'], int(r['MutPct'])
            mut_data[p][m]['N'].append(int(r['N']))
            mut_data[p][m]['avg'].append(float(r['Avg_ms']))
            mut_data[p][m]['jit'].append(float(r['Jit_ms']))
            mut_data[p][m]['nrg'].append(float(r['Nrg_uJ']))

    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(24, 8))
    
    for p in paradigms:
        # Throughput
        ax1.plot(mut_data[p][10]['N'], mut_data[p][10]['avg'], 'o-', color=colors[p], linewidth=2, label=f"{p} (10%)")
        ax1.plot(mut_data[p][1]['N'], mut_data[p][1]['avg'], 's--', color=colors[p], alpha=0.3, label=f"{p} (1%)")
        # Jitter
        ax2.plot(mut_data[p][10]['N'], mut_data[p][10]['jit'], 'o-', color=colors[p], linewidth=2, label=p)
        # Energy
        y_nrg = [mut_data[p][m]['nrg'][-1] for m in [1, 5, 10]]
        ax3.plot([1, 5, 10], y_nrg, 'o-', color=colors[p], linewidth=3, label=p)

    ax1.set_title("1. Throughput (ms)", fontweight='bold'); ax1.set_xscale('log'); ax1.grid(True, alpha=0.2); ax1.legend()
    ax2.set_title("2. Stability/Jitter (ms)", fontweight='bold'); ax2.set_xscale('log'); ax2.grid(True, alpha=0.2); ax2.legend()
    ax3.set_title("3. Energy Break-Even (N=1M)", fontweight='bold'); ax3.set_xticks([1, 5, 10]); ax3.grid(True, alpha=0.2); ax3.legend()
    
    plt.suptitle("Final Performance & Stability Analysis", fontsize=22, fontweight='bold', y=0.96)
    plt.subplots_adjust(top=0.85, bottom=0.15, left=0.05, right=0.95, wspace=0.2)
    plt.savefig(os.path.join(IMG_DIR, "crg_epilogue_victory_metrics.png"), dpi=200); plt.close()
    print("✅ Visuals updated.")

if __name__ == "__main__":
    run_pipeline()