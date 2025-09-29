# graficar.py
# Requiere: pip install pandas matplotlib

import os
import pandas as pd
import matplotlib.pyplot as plt

# Carpeta donde están tus CSV (ajusta si es necesario)
BASE_DIR = "."
UNIT = "ns"   # 'ns' (nanosegundos), 'us', 'ms', 's'

# Archivos esperados (6 CSV del experimento de heaps)
FILES = [
    "resultadosInsertBinaryHeap.csv",
    "resultadosInsertBinomialHeap.csv",
    "resultadosInsertFibonacciHeap.csv",
    "resultadosextractMinBinaryHeap.csv",
    "resultadosextractMinBinomialHeap.csv",
    "resultadosextractMinFibonacciHeap.csv",
]

# Títulos por archivo
TITLE = {
    "resultadosInsertBinaryHeap.csv":       ("Insert — Binary Heap",       "Tamaño (n)"),
    "resultadosInsertBinomialHeap.csv":     ("Insert — Binomial Heap",     "Tamaño (n)"),
    "resultadosInsertFibonacciHeap.csv":    ("Insert — Fibonacci Heap",    "Tamaño (n)"),
    "resultadosextractMinBinaryHeap.csv":   ("extractMin — Binary Heap",   "Tamaño (n)"),
    "resultadosextractMinBinomialHeap.csv": ("extractMin — Binomial Heap", "Tamaño (n)"),
    "resultadosextractMinFibonacciHeap.csv": ("extractMin — Fibonacci Heap", "Tamaño (n)"),
}

# Conversión de unidades (ns -> deseada)
SCALE = {"ns": 1.0, "us": 1e-3, "ms": 1e-6, "s": 1e-9}[UNIT]

def read_csv_robust(path):
    """Intenta leer con separadores comunes."""
    for sep in [",", ";", "\t", None]:
        try:
            if sep is None:
                df = pd.read_csv(path, engine="python")
            else:
                df = pd.read_csv(path, sep=sep)
            if df.shape[1] >= 2:
                return df
        except Exception:
            pass
    raise RuntimeError(f"No se pudo leer {path} con separadores comunes.")

def pick_xy(df):
    """Elige columnas X e Y (X: n/size, Y: t_mean) de forma flexible."""
    cols = [c.strip() for c in df.columns]
    lower = [c.lower() for c in cols]

    x_candidates = ["n", "size", "tamano", "tamaño"]
    y_candidates = ["t_mean", "tmean", "mean_time", "tiempo_medio", "tiempo_promedio"]

    xcol = next((cols[i] for i,c in enumerate(lower) if c in x_candidates), cols[0])
    ycol = next((cols[i] for i,c in enumerate(lower) if c in y_candidates),
                next((cols[i] for i,c in enumerate(lower) if c.startswith("t_")), cols[1]))
    return xcol, ycol

def plot_one(csv_name):
    path = os.path.join(BASE_DIR, csv_name)
    if not os.path.exists(path):
        print(f"[Aviso] No encontrado: {csv_name}")
        return

    df = read_csv_robust(path)
    xcol, ycol = pick_xy(df)

    # Ordena por X y convierte unidades (ns -> UNIT)
    df = df.sort_values(by=xcol).copy()
    df[ycol] = df[ycol] * SCALE

    # Promedio global del tiempo (en la unidad elegida)
    y_mean = df[ycol].mean()

    # Etiquetas
    title, xlabel = TITLE.get(csv_name, ("Experimento", "n"))
    # Deducir si es insert o extractMin para etiqueta Y amigable
    if "insert" in csv_name.lower():
        ylabel = f"Tiempo promedio por inserción ({UNIT})"
    elif "extractmin" in csv_name.lower():
        ylabel = f"Tiempo promedio por extracción de mínimo ({UNIT})"
    else:
        ylabel = f"Tiempo promedio ({UNIT})"

    # Gráfico
    plt.figure(figsize=(8, 5))
    plt.plot(df[xcol].values, df[ycol].values, linewidth=1.5, marker="o", label="t_mean (ns→{})".format(UNIT))

    # Línea horizontal del promedio
    plt.axhline(y=y_mean, linestyle="--", linewidth=1.2,
                label=f"Promedio global = {y_mean:.2f} {UNIT}")

    plt.title(f"{title}: tiempo vs {xlabel}")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True, alpha=0.3)
    plt.legend()  # muestra la leyenda con el promedio
    plt.tight_layout()

    out_png = os.path.join(BASE_DIR, csv_name.replace(".csv", f"_{UNIT}.png"))
    plt.savefig(out_png, dpi=160)
    print(f"[OK] Guardado: {out_png}")
    plt.close()

def main():
    for f in FILES:
        plot_one(f)

if __name__ == "__main__":
    main()
