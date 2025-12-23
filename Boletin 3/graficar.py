# graficar.py
# Genera gráficos de barras comparativos (FM vs RK)
# Requiere: pandas, matplotlib

import os
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = "."
FILES = {
    "fm_texto1GB.csv": ("FM-Index", "Texto 1GB"),
    "fm_texto200MB.csv": ("FM-Index", "Texto 200MB"),
    "fm_patron1GB.csv": ("FM-Index", "Patrón 1GB"),
    "fm_patron200MB.csv": ("FM-Index", "Patrón 200MB"),
    "rk_texto1GB.csv": ("Rabin-Karp", "Texto 1GB"),
    "rk_texto200MB.csv": ("Rabin-Karp", "Texto 200MB"),
    "rk_patron1GB.csv": ("Rabin-Karp", "Patrón 1GB"),
    "rk_patron200MB.csv": ("Rabin-Karp", "Patrón 200MB"),
}

def read_csv_auto(path):
    """Lee CSV con separador flexible"""
    for sep in [",", ";", "\t"]:
        try:
            df = pd.read_csv(path, sep=sep)
            if "t_mean" in df.columns:
                return df
        except Exception:
            pass
    raise RuntimeError(f"No se pudo leer correctamente: {path}")

data = []
for fname, (algo, exp) in FILES.items():
    if not os.path.exists(fname):
        print(f"[Aviso] Falta archivo: {fname}")
        continue
    df = read_csv_auto(fname)
    mean_time = df["t_mean"].mean() * 1e-3  # ns → μs
    data.append({"Algoritmo": algo, "Experimento": exp, "Tiempo (μs)": mean_time})

df_all = pd.DataFrame(data)
if df_all.empty:
    raise SystemExit("No se encontraron datos para graficar.")

# --- Gráficos de barras individuales ---
os.makedirs("graficos", exist_ok=True)
for exp, df_exp in df_all.groupby("Experimento"):
    plt.figure(figsize=(6, 4))
    plt.bar(df_exp["Algoritmo"], df_exp["Tiempo (μs)"],
            color=["#00a2e8" if a == "FM-Index" else "#f58231" for a in df_exp["Algoritmo"]])
    plt.title(f"Comparación de tiempo promedio - {exp}")
    plt.ylabel("Tiempo promedio (μs)")
    plt.grid(axis="y", alpha=0.3)
    for i, v in enumerate(df_exp["Tiempo (μs)"]):
        plt.text(i, v + 0.05 * max(df_exp["Tiempo (μs)"]), f"{v:.1f}", ha="center", fontsize=9)
    plt.tight_layout()
    plt.savefig(f"graficos/barras_{exp.replace(' ', '_')}.png", dpi=150)
    plt.close()
    print(f"[OK] Guardado: graficos/barras_{exp.replace(' ', '_')}.png")

# --- Gráfico comparativo global ---
plt.figure(figsize=(8, 5))
for exp, df_exp in df_all.groupby("Experimento"):
    plt.bar(df_exp["Algoritmo"] + " " + exp, df_exp["Tiempo (μs)"],
            color=["#00a2e8" if a == "FM-Index" else "#f58231" for a in df_exp["Algoritmo"]],
            label=exp)
plt.xticks(rotation=45, ha="right")
plt.ylabel("Tiempo promedio (μs)")
plt.title("Comparación global FM-Index vs Rabin-Karp")
plt.grid(axis="y", alpha=0.3)
plt.tight_layout()
plt.savefig("graficos/comparacion_global_barras.png", dpi=150)
plt.close()
print("[OK] Guardado: graficos/comparacion_global_barras.png")

print("\n=== RESUMEN ===")
print(df_all)
