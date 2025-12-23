
import os
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = "."
FILES = {
    "construccion_segtree.csv": "Construcción: Segment Tree",
    "construccion_sparsetable.csv": "Construcción: Sparse Table",
    "consulta_segtree.csv": "Consultas (Q=4096): Segment Tree",
    "consulta_sparsetable.csv": "Consultas (Q=4096): Sparse Table",
}

X_CANDIDATES = ["n", "size", "tamano", "tamaño", "pos", "position", "indice", "índice"]
Y_MEAN_CANDIDATES = ["t_mean", "tmean", "mean_time", "tiempo_medio", "tiempo_promedio"]
Y_STD_CANDIDATES = ["t_std", "tstd", "time_stdev", "desviacion", "stdev", "std"]

UNIT = "us"  # ns, us, ms, s
SCALE = {"ns": 1.0, "us": 1e-3, "ms": 1e-6, "s": 1e-9}[UNIT]

def read_csv_robust(path):
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

def pick_columns(df):
    cols = [c for c in df.columns]
    lower = [c.strip().lower() for c in cols]
    xcol = next((cols[i] for i,c in enumerate(lower) if c in X_CANDIDATES), cols[0])
    ymean = next((cols[i] for i,c in enumerate(lower) if c in Y_MEAN_CANDIDATES),
                 next((cols[i] for i,c in enumerate(lower) if c.startswith("t_")), cols[1]))
    ystd = next((cols[i] for i,c in enumerate(lower) if c in Y_STD_CANDIDATES), None)
    return xcol, ymean, ystd

def load_all():
    data = {}
    for fname, title in FILES.items():
        path = os.path.join(BASE_DIR, fname)
        if not os.path.exists(path):
            print(f"[Aviso] No encontrado: {fname}")
            continue
        df = read_csv_robust(path)
        xcol, ycol, yscol = pick_columns(df)
        df = df.sort_values(by=xcol).copy()
        df[ycol] = df[ycol] * SCALE
        if yscol is not None:
            df[yscol] = df[yscol] * SCALE
        data[fname] = (df, {"title": title, "xcol": xcol, "ycol": ycol, "yscol": yscol})
    return data

def save_individual_plots(dataset):
    outs = []
    for fname, (df, meta) in dataset.items():
        x, y, ys = meta["xcol"], meta["ycol"], meta["yscol"]
        plt.figure(figsize=(8, 5))
        plt.plot(df[x].values, df[y].values, linewidth=1.5, label="t_mean")
        if ys is not None:
            y_up = df[y] + df[ys]
            y_dn = df[y] - df[ys]
            plt.fill_between(df[x].values, y_dn.values, y_up.values, alpha=0.2, label="±1 desv.est.")
        plt.title(meta["title"])
        plt.xlabel("Tamaño n")
        plt.ylabel(f"Tiempo promedio ({UNIT})")
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.tight_layout()
        out_png = os.path.join(BASE_DIR, fname.replace(".csv", f"_{UNIT}.png"))
        plt.savefig(out_png, dpi=160)
        plt.close()
        print(f"[OK] Guardado: {out_png}")
        outs.append(out_png)
    return outs

def save_comparative_plots(dataset):
    outs = []
    build_keys = [k for k in dataset if "construccion" in k]
    if build_keys:
        plt.figure(figsize=(8, 5))
        for k in sorted(build_keys):
            df, meta = dataset[k]
            x, y = meta["xcol"], meta["ycol"]
            plt.plot(df[x].values, df[y].values, linewidth=1.5, label=meta["title"])
        plt.title("Comparación: tiempo de construcción")
        plt.xlabel("Tamaño n")
        plt.ylabel(f"Tiempo promedio ({UNIT})")
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.tight_layout()
        out_png = os.path.join(BASE_DIR, f"comparacion_construccion_{UNIT}.png")
        plt.savefig(out_png, dpi=160)
        plt.close()
        print(f"[OK] Guardado: {out_png}")
        outs.append(out_png)
    query_keys = [k for k in dataset if "consulta" in k]
    if query_keys:
        plt.figure(figsize=(8, 5))
        for k in sorted(query_keys):
            df, meta = dataset[k]
            x, y = meta["xcol"], meta["ycol"]
            plt.plot(df[x].values, df[y].values, linewidth=1.5, label=meta["title"])
        plt.title("Comparación: tiempo de consultas (Q=4096 por n)")
        plt.xlabel("Tamaño n")
        plt.ylabel(f"Tiempo total ({UNIT})")
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.tight_layout()
        out_png = os.path.join(BASE_DIR, f"comparacion_consultas_{UNIT}.png")
        plt.savefig(out_png, dpi=160)
        plt.close()
        print(f"[OK] Guardado: {out_png}")
        outs.append(out_png)
    return outs

def main():
    dataset = load_all()
    save_individual_plots(dataset)
    save_comparative_plots(dataset)

if __name__ == "__main__":
    main()
