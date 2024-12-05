from tkinter import ttk
import tkinter as tk
from TkElements.AnalyseCheckBoxList import AnalyseCheckBoxList
from TkElements.RemoveAnalyseForm import RemoveAnalyseForm
from TkElements.AddAnalyseForm import AddAnalyseForm
from GraphDrawer.GraphDrawer import GraphDrawer
from TkElements.Selector import Selector


class MainWindow():
	def __init__(self, master, symbols, get_analyzers, get_analysis, add_analyse, remove_analyse):
		self.symbols = symbols
		self.get_analyzers = get_analyzers
		self.get_analysis = get_analysis
		self.add_analyse = add_analyse
		self.remove_analyse = remove_analyse
		self.master = master
		self.master.geometry('1000x800')
		self.master.title('App')

		self.main_frame = ttk.Frame(self.master)
		self.main_frame.pack(fill=tk.BOTH, expand=True)

		self.canvas = GraphDrawer(self.main_frame)

		self.right_frame = tk.Frame(self.main_frame, width=300)
		self.right_frame.pack(side=tk.RIGHT, fill=tk.BOTH)

		self.selector = Selector(self.right_frame, self.symbols)

		self.add_analyse_form_is_opened = None
		self.remove_analyse_form_is_opened = None
		tk.Button(self.right_frame, text="Add Analyse", command=self.add_analyse_form).pack()
		tk.Button(self.right_frame, text="Remove Analyse", command=self.remove_analyse_form).pack()

		self.checkboxes = AnalyseCheckBoxList(self.right_frame)

	def draw_canvas(self, df, analysis):
		self.canvas.redraw(df, self.selector.selected_symbol, self.symbols, analysis, self.checkboxes.get_active_names())

	def add_analyse_form(self):
		if self.add_analyse_form_is_opened is None or not self.add_analyse_form_is_opened.winfo_exists():
			analyzers = self.get_analyzers()
			self.add_analyse_form_is_opened = AddAnalyseForm(self.master, self.add_analyse, self.get_analysis, analyzers)

	def remove_analyse_form(self):
		if self.remove_analyse_form_is_opened is None or not self.remove_analyse_form_is_opened.winfo_exists():
			names = [row[1] for row in self.get_analysis()]
			self.remove_analyse_form_is_opened = RemoveAnalyseForm(self.master, names, self.remove_analyse)

