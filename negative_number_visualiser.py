import tkinter as tk
from tkinter import ttk

CELL = 40          # pixel size of one square
PADDING = 20       # canvas padding
GAP = 20           # horizontal gap between columns
POS_COLOR = "#8ecae6"   # color for stacked squares
NEG_COLOR = "#b0b0b0"   # color for hole squares
GROUND_COLOR = "#8b5a2b"  # ground fill color
LINE_COLOR = "#000000"    # outline/grid color

class StackedSquaresApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Stacked Squares / Ground Holes")
        self.resizable(False, False)

        self.show_sum = False  # whether to display the 3rd column (sum)

        # Top bar with inputs
        top = ttk.Frame(self, padding=10)
        top.pack(fill="x")

        ttk.Label(top, text="Column A:").grid(row=0, column=0, padx=(0,6))
        self.a_var = tk.StringVar(value="2")
        a_entry = ttk.Entry(top, width=8, textvariable=self.a_var)
        a_entry.grid(row=0, column=1, padx=(0,16))

        ttk.Label(top, text="Column B:").grid(row=0, column=2, padx=(0,6))
        self.b_var = tk.StringVar(value="-1")
        b_entry = ttk.Entry(top, width=8, textvariable=self.b_var)
        b_entry.grid(row=0, column=3, padx=(0,16))

        # Buttons: Add and Reset
        btns = ttk.Frame(top)
        btns.grid(row=0, column=4, padx=(0,0))
        self.add_btn = ttk.Button(btns, text="Add", command=self.on_add)
        self.add_btn.grid(row=0, column=0, padx=(0,8))
        self.reset_btn = ttk.Button(btns, text="Reset", command=self.on_reset)
        self.reset_btn.grid(row=0, column=1)

        # Live re-render on typing; if sum is shown, keep it in sync too
        a_entry.bind("<KeyRelease>", lambda e: self.render())
        b_entry.bind("<KeyRelease>", lambda e: self.render())

        # Canvas placeholder; size will be set dynamically on render
        self.canvas = tk.Canvas(self, width=400, height=300, bg="white", highlightthickness=0)
        self.canvas.pack(padx=10, pady=(0,10))

        self.status = tk.StringVar(value="Tip: Positive = stacks above ground, Negative = holes below ground.")
        ttk.Label(self, textvariable=self.status, padding=(10,0)).pack(anchor="w")

        self.render()

    def parse_inputs(self):
        def to_int(s):
            try:
                return int(s.strip())
            except Exception:
                return 0
        a = to_int(self.a_var.get())
        b = to_int(self.b_var.get())
        return a, b

    def on_add(self):
        self.show_sum = True
        self.render()

    def on_reset(self):
        self.show_sum = False
        self.render()

    def render(self):
        a, b = self.parse_inputs()
        values = [a, b]
        labels = ["A", "B"]

        if self.show_sum:
            values.append(a + b)
            labels.append("A+B")

        # Compute vertical extents (3.7+ compatible)
        positives = [v for v in values if v > 0]
        negatives = [-v for v in values if v < 0]
        max_up = max([0] + positives)
        max_down = max([0] + negatives)

        # Ensure at least 1 cell visible for context
        max_up = max(max_up, 1)
        max_down = max(max_down, 1)

        cols = len(values)
        width = PADDING*2 + cols*CELL + (cols-1)*GAP
        height = PADDING*2 + (max_up + max_down)*CELL

        # Resize canvas
        self.canvas.config(width=width, height=height)
        self.canvas.delete("all")

        baseline_y = PADDING + max_up*CELL  # y position of ground line

        # Draw ground area (below baseline)
        ground_top = baseline_y
        ground_bottom = height - PADDING
        self.canvas.create_rectangle(
            PADDING//2, ground_top, width - PADDING//2, ground_bottom,
            fill=GROUND_COLOR, outline=""
        )

        # Draw ground baseline
        self.canvas.create_line(
            PADDING//2, baseline_y, width - PADDING//2, baseline_y,
            fill=LINE_COLOR, width=2
        )

        # Draw each column
        for i, v in enumerate(values):
            x0 = PADDING + i*(CELL + GAP)
            x1 = x0 + CELL

            if v > 0:
                # Draw v stacked squares above ground
                for k in range(v):
                    y1 = baseline_y - k*CELL
                    y0 = y1 - CELL
                    self.canvas.create_rectangle(x0, y0, x1, y1, fill=POS_COLOR, outline=LINE_COLOR)
            elif v < 0:
                depth = -v
                # Draw depth squares as “holes” below ground
                for k in range(depth):
                    y0 = baseline_y + k*CELL
                    y1 = y0 + CELL
                    # Hole cell
                    self.canvas.create_rectangle(x0, y0, x1, y1, fill=NEG_COLOR, outline=LINE_COLOR)
                    # Crosshatch to suggest emptiness
                    self.canvas.create_line(x0, y0, x1, y1, fill=LINE_COLOR)
                    self.canvas.create_line(x1, y0, x0, y1, fill=LINE_COLOR)
            else:
                # 0 -> nothing
                pass

            # Column value label under ground
            self.canvas.create_text((x0+x1)//2, height - PADDING//2, text=str(v))

            # Column name above column
            self.canvas.create_text((x0+x1)//2, PADDING//2, text=labels[i])

        # Update status text
        status_cols = " | ".join(f"{labels[i]}={values[i]}" for i in range(len(values)))
        self.status.set(f"{status_cols}  |  Ground at y = {baseline_y}  |  Sum shown: {self.show_sum}")

if __name__ == "__main__":
    app = StackedSquaresApp()
    app.mainloop()
