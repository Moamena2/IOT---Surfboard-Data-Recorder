import tkinter as tk
from tkinter import filedialog
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import numpy as np
import csv


def read_file(filepath, graph_type, canvas, fig, highlight_interval=None):
    with open(filepath, 'r') as file:
        lines = file.readlines()

    # Skip the first line (metadata)
    data_lines = lines[1:]

    t = []
    x, y, z = [], [], []
    u, v, w = [], [], []
    time_stamps = []
    single_value = []
    bmi_line_cntr = 0
    force_line_cntr = 0
    first_bmi_time = 0
    first_force_time = 0
    for line in data_lines:
        file_values = line.split()
        values = list(map(float, file_values[:-1]))
        if graph_type == 'gyroscope':
            print(len(values))
            if(len(values) != 7):
                continue
            if(bmi_line_cntr == 0):
                first_bmi_time = values[6]
            t.append(values[6])
            x.append(values[0])
            y.append(values[1])
            z.append(values[2])
            if (len(file_values) == 8):
                time_stamps.append(file_values[-1])
            bmi_line_cntr += 1
        elif graph_type == 'acceleration':
            if len(values) != 7:
                continue;
            if (bmi_line_cntr == 0):
                first_bmi_time = values[6]
            t.append(values[6])
            u.append(values[3])
            v.append(values[4])
            w.append(values[5])
            if(len(file_values) == 8):
                time_stamps.append(file_values[-1])
            bmi_line_cntr += 1
        elif graph_type == 'force':
            if len(values) != 2:
                continue;
            if (force_line_cntr == 0):
                first_force_time = values[1]
            t.append(values[1])
            single_value.append(values[0])
            if (len(file_values) == 3):
                time_stamps.append(file_values[-1])
            force_line_cntr += 1

        values = []

    fig.clear()
    ax = fig.add_subplot(111)

    if graph_type == 'gyroscope':
        ax.plot(t, x, label='x vs t')
        ax.plot(t, y, label='y vs t')
        ax.plot(t, z, label='z vs t')
    elif graph_type == 'acceleration':
        ax.plot(t, u, label='u vs t')
        ax.plot(t, v, label='v vs t')
        ax.plot(t, w, label='w vs t')
    elif graph_type == 'force':
        ax.plot(t, single_value, label='Value vs t')

    ax.set_xlabel('t (ms)')
    ax.set_ylabel('rad/s' if graph_type == 'gyroscope' else 'g (9.81 m/s^2)' if graph_type == 'acceleration' else 'gr')
    ax.set_title(f"{'GyroScope' if graph_type == 'gyroscope' else 'Acceleration' if graph_type == 'acceleration' else 'Force'} Graph")
    ax.legend()
    ax.grid(True)

    if highlight_interval:
        start_time, end_time = highlight_interval
        ax.axvspan(start_time, end_time, color='yellow', alpha=0.3)  # Highlight interval

    canvas.draw()
    return time_stamps,t, x, y, z, u, v, w, single_value


def on_click(event):
    if event.inaxes and not zoom_enabled:
        global click_count, start_time, end_time
        t_value = event.xdata

        if click_count == 0:
            start_time = t_value
            click_count += 1
        elif click_count == 1:
            end_time = t_value
            click_count = 0
            # Update the plot with the new interval highlighted
            t, x, y, z, u, v, w, single_value,time_stamp = read_file(filepath, graph_type, canvas, fig, highlight_interval=(
            min(start_time, end_time), max(start_time, end_time)))
            take_screenshot_button.pack(side=tk.RIGHT, padx=10, pady=10)  # Show the screenshot button


def zoom(event):
    if zoom_enabled:
        ax = fig.gca()
        xlim = ax.get_xlim()
        ylim = ax.get_ylim()

        scale_factor = 1 / 1.2 if event.button == 'up' else 1.2

        # Update x and y limits
        new_xlim = [event.xdata - (event.xdata - xlim[0]) * scale_factor,
                    event.xdata + (xlim[1] - event.xdata) * scale_factor]
        new_ylim = [event.ydata - (event.ydata - ylim[0]) * scale_factor,
                    event.ydata + (ylim[1] - event.ydata) * scale_factor]

        ax.set_xlim(new_xlim)
        ax.set_ylim(new_ylim)

        canvas.draw()


def toggle_zoom():
    global zoom_enabled
    zoom_enabled = not zoom_enabled
    toggle_button.config(bg='green' if zoom_enabled else 'red')


def open_file(graph_type):
    global filepath, graph_data
    filepath = filedialog.askopenfilename()
    if filepath:
        graph_data = read_file(filepath, graph_type, canvas, fig)
        take_screenshot_button.pack_forget()  # Hide the screenshot button when a new file is opened


def take_screenshot():
    global start_time, end_time

    def find_first_greater_index(my_list, value):
        for i, element in enumerate(my_list):
            if element > value:
                return i
        return -1
    file_path = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV files", "*.csv")])
    if file_path:
        # Filter data within the highlighted interval
        print(start_time);
        print(end_time);



        time_stamps,t, x, y, z, u, v, w, single_value = graph_data
        start_idx = find_first_greater_index(t, start_time)
        end_idx = find_first_greater_index(t, end_time)

        if graph_type == 'gyroscope':
            interval_t = t[start_idx:end_idx]
            interval_x = x[start_idx:end_idx]
            interval_y = y[start_idx:end_idx]
            interval_z = z[start_idx:end_idx]
            interval_time_stamps = time_stamps[start_idx:end_idx]
            interval_data = list(zip(interval_t, interval_x, interval_y, interval_z,interval_time_stamps))
            headers = ['t [m/s]', 'x[rad/s]', 'y[rad/s]', 'z[rad/s]', 'time stamp']
        elif graph_type == 'acceleration':
            interval_t = t[start_idx:end_idx]
            interval_u = u[start_idx:end_idx]
            interval_v = v[start_idx:end_idx]
            interval_w = w[start_idx:end_idx]
            interval_time_stamps = time_stamps[start_idx:end_idx]
            interval_data = list(zip(interval_t, interval_u, interval_v, interval_w,interval_time_stamps))
            headers = ['t [m/s]', 'u[g]', 'v[g]', 'w[g]','time stamp']
        elif graph_type == 'force':
            interval_t = t[start_idx:end_idx]
            interval_values = single_value[start_idx:end_idx]
            interval_time_stamps = time_stamps[start_idx:end_idx]
            interval_data = list(zip(interval_t, interval_values,interval_time_stamps))
            headers = ['t [m/s]', 'value [gr]', 'time stamp']

        # Save the interval data as a CSV file
        with open(file_path, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(headers)
            writer.writerows(interval_data)

        # Create and save the interval graph as an image
        interval_fig, interval_ax = plt.subplots()
        if graph_type == 'gyroscope':
            interval_ax.plot(interval_t, interval_x, label='x vs t')
            interval_ax.plot(interval_t, interval_y, label='y vs t')
            interval_ax.plot(interval_t, interval_z, label='z vs t')
        elif graph_type == 'acceleration':
            interval_ax.plot(interval_t, interval_u, label='u vs t')
            interval_ax.plot(interval_t, interval_v, label='v vs t')
            interval_ax.plot(interval_t, interval_w, label='w vs t')
        elif graph_type == 'force':
            interval_ax.plot(interval_t, interval_values, label='Value vs t')

        interval_ax.set_xlabel('t')
        interval_ax.set_ylabel('Values')
        interval_ax.set_title(f"{'GyroScope' if graph_type == 'gyroscope' else 'Acceleration' if graph_type == 'acceleration' else 'Force'} Graph Interval")
        interval_ax.legend()
        interval_ax.grid(True)

        img_buffer = BytesIO()
        interval_fig.savefig(img_buffer, format='png')
        plt.close(interval_fig)
        img_buffer.seek(0)

        img_str = base64.b64encode(img_buffer.read()).decode('utf-8')

        # Add the image as a base64 string to the CSV file
        with open(file_path, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([])
            writer.writerow(['Graph Image (Base64 Encoded)'])
            writer.writerow([img_str])
        print(f"Data and graph saved as {file_path}")


def on_mouse_press(event):
    global press_x, press_y
    if event.inaxes and zoom_enabled:
        press_x = event.xdata
        press_y = event.ydata


def on_mouse_drag(event):
    if event.inaxes and zoom_enabled:
        global press_x, press_y
        ax = fig.gca()
        xlim = ax.get_xlim()
        ylim = ax.get_ylim()

        dx = event.xdata - press_x
        dy = event.ydata - press_y

        ax.set_xlim([x - dx for x in xlim])
        ax.set_ylim([y - dy for y in ylim])

        press_x = event.xdata
        press_y = event.ydata

        canvas.draw()


def on_mouse_release(event):
    if event.inaxes and zoom_enabled:
        global press_x, press_y
        press_x, press_y = None, None


def main():
    global canvas, fig, filepath, graph_type, click_count, start_time, end_time, zoom_enabled, toggle_button, take_screenshot_button
    global press_x, press_y, graph_data

    root = tk.Tk()
    root.title("Graph Plotter")

    frame = tk.Frame(root)
    frame.pack(padx=10, pady=10)

    btn_frame = tk.Frame(frame)
    btn_frame.pack(side=tk.TOP, pady=10)

    btn_first_graph = tk.Button(btn_frame, text="GyroScope Graph",
                                command=lambda: [open_file('gyroscope'), set_graph_type('gyroscope')])
    btn_first_graph.pack(side=tk.LEFT, padx=5)

    btn_second_graph = tk.Button(btn_frame, text="Acceleration Graph",
                                 command=lambda: [open_file('acceleration'), set_graph_type('acceleration')])
    btn_second_graph.pack(side=tk.LEFT, padx=5)

    btn_third_graph = tk.Button(btn_frame, text="Force Graph",
                                command=lambda: [open_file('force'), set_graph_type('force')])
    btn_third_graph.pack(side=tk.LEFT, padx=5)

    toggle_button = tk.Button(btn_frame, text="Toggle Zoom", command=toggle_zoom, bg='red')
    toggle_button.pack(side=tk.LEFT, padx=5)

    fig = plt.Figure(figsize=(10, 6))
    canvas = FigureCanvasTkAgg(fig, master=frame)
    canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

    click_count = 0
    start_time = None
    end_time = None
    zoom_enabled = False
    press_x = press_y = None

    # Button for taking screenshots
    take_screenshot_button = tk.Button(root, text="Save as CSV", command=take_screenshot)
    take_screenshot_button.pack(side=tk.RIGHT, padx=10, pady=10)
    take_screenshot_button.pack_forget()  # Hide the button initially

    # Bind events
    canvas.mpl_connect('button_press_event', on_mouse_press)
    canvas.mpl_connect('motion_notify_event', on_mouse_drag)
    canvas.mpl_connect('button_release_event', on_mouse_release)
    canvas.mpl_connect('button_press_event', on_click)
    canvas.mpl_connect('scroll_event', zoom)

    root.mainloop()


def set_graph_type(type):
    global graph_type
    graph_type = type


if __name__ == "__main__":
    main()
