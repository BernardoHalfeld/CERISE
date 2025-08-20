# interface_gui.py (versão com correção de thread-safety)
import customtkinter as ctk
from tkinter import filedialog
import os
import re
from pathlib import Path
import threading
import queue
import sys
import multiprocessing
from PIL import Image

try:
    from Codigo_Matheus import process_scan_pair
except ImportError:
    print("ERRO: O arquivo 'Codigo_Matheus.py' não foi encontrado ou contém erros.")
    sys.exit(1)

def show_ply_in_new_process(filepath):
    import open3d as o3d
    from pathlib import Path
    if not filepath or not Path(filepath).exists():
        print(f"ERRO (Processo Filho): Arquivo não encontrado - {filepath}")
        return
    try:
        pcd = o3d.io.read_point_cloud(str(filepath))
        if not pcd.has_points():
            print(f"ERRO (Processo Filho): O arquivo .ply está vazio - {filepath}")
            return
        o3d.visualization.draw_geometries([pcd], window_name=f"Visualizador - {Path(filepath).name}")
    except Exception as e:
        print(f"ERRO (Processo Filho): Ocorreu um erro ao visualizar - {e}")

class QueueIO(queue.Queue):
    def write(self, text): self.put(text)
    def flush(self): sys.__stdout__.flush()

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Cerise 3D Processor")
        self.geometry("1200x850")
        self.after(20, lambda: self.state('zoomed'))
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        self.ply_visualize_buttons = []

        logo_frame = ctk.CTkFrame(self, fg_color="transparent")
        logo_frame.grid(row=0, column=0, padx=10, pady=(5,0), sticky="ew")
        logo_frame.grid_columnconfigure(0, weight=1)
        
        try:
            logo_image_data = Image.open("logo_cerise.png")
            logo_image = ctk.CTkImage(light_image=logo_image_data, size=(200, 50))
            logo_label = ctk.CTkLabel(logo_frame, image=logo_image, text="")
            logo_label.grid(row=0, column=0, pady=10)
        except FileNotFoundError:
            logo_label = ctk.CTkLabel(logo_frame, text="Cerise 3D", font=ctk.CTkFont(size=20, weight="bold"))
            logo_label.grid(row=0, column=0, pady=10)
            print("Aviso: Arquivo 'logo_cerise.png' não encontrado.")

        path_frame = ctk.CTkFrame(self, fg_color="transparent")
        path_frame.grid(row=1, column=0, padx=10, pady=10, sticky="ew")
        path_frame.grid_columnconfigure(1, weight=1)
        
        ctk.CTkLabel(path_frame, text="Pasta de Arquivos:").grid(row=0, column=0, padx=(10,5))
        self.path_entry = ctk.CTkEntry(path_frame, placeholder_text="Selecione a pasta de trabalho...")
        self.path_entry.grid(row=0, column=1, padx=5, sticky="ew")
        self.browse_button = ctk.CTkButton(path_frame, text="Procurar...", command=self.select_folder)
        self.browse_button.grid(row=0, column=2, padx=(5,10))

        main_container = ctk.CTkFrame(self, fg_color="transparent")
        main_container.grid(row=2, column=0, padx=10, pady=5, sticky="nsew")
        main_container.grid_columnconfigure((0, 1), weight=1)
        main_container.grid_rowconfigure(0, weight=1)
        main_container.grid_rowconfigure(1, weight=2)

        q1_frame = ctk.CTkFrame(main_container)
        q1_frame.grid(row=0, column=0, padx=(0, 5), pady=(0, 5), sticky="nsew")
        q1_frame.grid_rowconfigure(1, weight=1)
        q1_frame.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(q1_frame, text="Scans Disponíveis para Processamento", font=ctk.CTkFont(family="Roboto", size=18, weight='bold')).grid(row=0, column=0, padx=10, pady=5)
        self.scrollable_scans_frame = ctk.CTkScrollableFrame(q1_frame)
        self.scrollable_scans_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")
        self.scan_checkboxes = {}

        q2_frame = ctk.CTkFrame(main_container)
        q2_frame.grid(row=0, column=1, padx=(5, 0), pady=(0, 5), sticky="nsew")
        q2_frame.grid_columnconfigure(1, weight=1)
        ctk.CTkLabel(q2_frame, text="Parâmetros de Processamento", font=ctk.CTkFont(family="Roboto", size=18, weight='bold')).grid(row=0, column=0, columnspan=2, pady=5, padx=10)
        self.param_entries = {}
        parameters = {"Percentil A Manter": "40.0", "Voxel Size": "0.001", "Neighbor Density Factor": "300.0", "Std Ratio": "1.7", "Min Neighbors": "200", "Max Neighbors": "10000"}
        param_keys = {"Percentil A Manter": "percentil_a_manter", "Voxel Size": "voxel_size", "Neighbor Density Factor": "neighbor_density_factor", "Std Ratio": "std_ratio", "Min Neighbors": "min_neighbors", "Max Neighbors": "max_neighbors"}
        for i, (p_label, p_val) in enumerate(parameters.items(), start=1):
            ctk.CTkLabel(q2_frame, text=f"{p_label}:").grid(row=i, column=0, padx=10, pady=5, sticky="w")
            entry = ctk.CTkEntry(q2_frame)
            entry.insert(0, p_val)
            entry.grid(row=i, column=1, padx=10, pady=5, sticky="ew")
            self.param_entries[param_keys[p_label]] = entry

        q3_frame = ctk.CTkFrame(main_container)
        q3_frame.grid(row=1, column=0, padx=(0, 5), pady=(5, 0), sticky="nsew")
        q3_frame.grid_rowconfigure(2, weight=1)
        q3_frame.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(q3_frame, text="Progresso e Logs", font=ctk.CTkFont(family="Roboto", size=18, weight='bold')).grid(row=0, column=0, padx=10, pady=5)
        progress_frame_q3 = ctk.CTkFrame(q3_frame)
        progress_frame_q3.grid(row=1, column=0, padx=5, pady=5, sticky="ew")
        progress_frame_q3.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(progress_frame_q3, text="Geral:", width=50).pack(side="left", padx=(5,0))
        self.geral_progressbar = ctk.CTkProgressBar(progress_frame_q3)
        self.geral_progressbar.set(0)
        self.geral_progressbar.pack(side="left", fill="x", expand=True, padx=5)
        self.geral_progress_label = ctk.CTkLabel(progress_frame_q3, text="N/A", width=40)
        self.geral_progress_label.pack(side="left", padx=(0,5))
        ctk.CTkLabel(progress_frame_q3, text="Tarefa:", width=50).pack(side="left", padx=(5,0))
        self.tarefa_progressbar = ctk.CTkProgressBar(progress_frame_q3)
        self.tarefa_progressbar.set(0)
        self.tarefa_progressbar.pack(side="left", fill="x", expand=True, padx=5)
        self.tarefa_progress_label = ctk.CTkLabel(progress_frame_q3, text="0%", width=40)
        self.tarefa_progress_label.pack(side="left", padx=(0,5))
        self.log_textbox = ctk.CTkTextbox(q3_frame, state="disabled", activate_scrollbars=True)
        self.log_textbox.grid(row=2, column=0, padx=5, pady=5, sticky="nsew")

        q4_frame = ctk.CTkFrame(main_container)
        q4_frame.grid(row=1, column=1, padx=(5, 0), pady=(5, 0), sticky="nsew")
        q4_frame.grid_rowconfigure(1, weight=1)
        q4_frame.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(q4_frame, text="Visualizador de Arquivos .PLY", font=ctk.CTkFont(family="Roboto", size=18, weight='bold')).grid(row=0, column=0, padx=10, pady=5)
        self.scrollable_ply_frame = ctk.CTkScrollableFrame(q4_frame)
        self.scrollable_ply_frame.grid(row=1, column=0, padx=5, pady=5, sticky="nsew")

        footer_frame = ctk.CTkFrame(self, fg_color="transparent")
        footer_frame.grid(row=3, column=0, padx=10, pady=10, sticky="ew")
        footer_frame.grid_columnconfigure((0, 1), weight=1)
        
        self.start_button = ctk.CTkButton(footer_frame, text="Iniciar Processamento", command=self.start_processing_thread)
        self.start_button.grid(row=0, column=0, padx=10, pady=5, sticky="e")
        
        self.open_folder_button = ctk.CTkButton(footer_frame, text="Abrir Pasta de Arquivos", command=self.open_output_folder, state="disabled")
        self.open_folder_button.grid(row=0, column=1, padx=10, pady=5, sticky="w")
        
        self.log_queue = QueueIO()
        sys.stdout = self.log_queue
        self.after(100, self.update_log_from_queue)

    def select_folder(self):
        folder_path = filedialog.askdirectory()
        if not folder_path: return
        self.path_entry.delete(0, "end")
        self.path_entry.insert(0, folder_path)
        self.open_folder_button.configure(state="normal")
        self.update_scan_list(folder_path)
        self.update_ply_list(folder_path)

    def update_scan_list(self, base_path_str):
        for widget in self.scrollable_scans_frame.winfo_children(): widget.destroy()
        self.scan_checkboxes = {}
        base_path = Path(base_path_str)
        if not base_path.is_dir(): return
        scan_ids = [m.group(1) for f in base_path.glob("raw_lidar_pan_*.txt") if (m := re.search(r'raw_lidar_pan_(\d+)\.txt', f.name)) and (base_path / f"raw_imu_pan_{m.group(1)}.txt").exists()]
        scan_ids.sort(key=int)
        if not scan_ids:
            ctk.CTkLabel(self.scrollable_scans_frame, text="Nenhum par de scan (LIDAR/IMU) válido.").pack(pady=10)
            return
        for scan_id in scan_ids:
            var = ctk.StringVar(value="on")
            cb = ctk.CTkCheckBox(self.scrollable_scans_frame, text=f"Scan ID: {scan_id}", variable=var, onvalue="on", offvalue="off")
            cb.pack(anchor="w", padx=10, pady=2, fill="x")
            self.scan_checkboxes[scan_id] = var

    def update_ply_list(self, base_path_str):
        for widget in self.scrollable_ply_frame.winfo_children(): widget.destroy()
        self.ply_visualize_buttons.clear()
        base_path = Path(base_path_str)
        if not base_path.is_dir(): return
        
        ply_files = sorted(list(base_path.glob("*.ply")), key=os.path.getmtime, reverse=True)
        if not ply_files:
            ctk.CTkLabel(self.scrollable_ply_frame, text="Nenhum arquivo .ply encontrado.").pack(pady=10)
            return

        for ply_file in ply_files:
            item_frame = ctk.CTkFrame(self.scrollable_ply_frame, fg_color="transparent")
            item_frame.pack(fill="x", expand=True, pady=2)
            item_frame.grid_columnconfigure(0, weight=1)
            
            ctk.CTkLabel(item_frame, text=ply_file.name).grid(row=0, column=0, padx=5, sticky="w")
            vis_button = ctk.CTkButton(item_frame, text="Visualizar", width=80, command=lambda p=ply_file: self.visualize_ply_thread(p))
            vis_button.grid(row=0, column=1, padx=5)
            self.ply_visualize_buttons.append(vis_button)

    def set_controls_state(self, state):
        self.start_button.configure(state=state)
        self.browse_button.configure(state=state)
        for button in self.ply_visualize_buttons:
            button.configure(state=state)

    def start_processing_thread(self):
        self.set_controls_state("disabled")
        self.start_button.configure(text="Processando...")
        self.log_textbox.configure(state="normal")
        self.log_textbox.delete("1.0", "end")
        self.log_textbox.configure(state="disabled")
        threading.Thread(target=self.run_processing_logic, daemon=True).start()

    def run_processing_logic(self):
        base_path_str = self.path_entry.get()
        if not base_path_str:
            print("ERRO: Por favor, selecione uma pasta primeiro.")
            self.set_controls_state("normal")
            self.start_button.configure(text="Iniciar Processamento")
            return
        
        base_path = Path(base_path_str)
        selected_scans = [scan_id for scan_id, var in self.scan_checkboxes.items() if var.get() == "on"]
        if not selected_scans:
            print("AVISO: Nenhum scan foi selecionado para processamento.")
            self.set_controls_state("normal")
            self.start_button.configure(text="Iniciar Processamento")
            return

        try:
            params = {key: float(entry.get()) for key, entry in self.param_entries.items()}
        except ValueError:
            print("ERRO: Todos os parâmetros devem ser números válidos.")
            self.set_controls_state("normal")
            self.start_button.configure(text="Iniciar Processamento")
            return
        
        total_scans = len(selected_scans)
        self.update_geral_progress(0, total_scans)
        print(f"\nIniciando processamento para {total_scans} scans selecionados...")
        
        for i, scan_id in enumerate(selected_scans):
            self.update_geral_progress(i, total_scans)
            self.update_tarefa_progress(0, 1)
            process_scan_pair(base_path, scan_id, params, progress_callback=self.update_tarefa_progress)
            self.update_tarefa_progress(1, 1)

        self.update_geral_progress(total_scans, total_scans)
        print("\n" + "#"*80)
        print("PROCESSAMENTO CONCLUÍDO!")
        self.set_controls_state("normal")
        self.start_button.configure(text="Iniciar Processamento")
        
        # --- CORREÇÃO APLICADA AQUI ---
        # Agenda a atualização da lista de PLY para a thread principal de forma segura
        self.after(0, self.update_ply_list, base_path_str)

    def visualize_ply_thread(self, filepath):
        vis_process = multiprocessing.Process(target=show_ply_in_new_process, args=(filepath,), daemon=True)
        vis_process.start()
        
    def update_tarefa_progress(self, current_value, max_value):
        percentage = current_value / max_value if max_value > 0 else 0
        self.tarefa_progressbar.set(percentage)
        self.tarefa_progress_label.configure(text=f"{int(percentage * 100)}%")

    def update_geral_progress(self, current_scan, total_scans):
        percentage = current_scan / total_scans if total_scans > 0 else 0
        self.geral_progressbar.set(percentage)
        self.geral_progress_label.configure(text=f"{current_scan}/{total_scans}")

    def update_log_from_queue(self):
        while not self.log_queue.empty():
            line = self.log_queue.get_nowait()
            self.log_textbox.configure(state="normal")
            self.log_textbox.insert("end", line)
            self.log_textbox.see("end")
            self.log_textbox.configure(state="disabled")
        self.after(100, self.update_log_from_queue)

    def open_output_folder(self):
        if os.path.isdir(self.path_entry.get()): os.startfile(self.path_entry.get())
            
    def on_closing(self):
        sys.stdout = sys.__stdout__
        self.destroy()

if __name__ == "__main__":
    multiprocessing.freeze_support()
    app = App()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()