from tkinter import ttk


class Selector(ttk.Combobox):
	def __init__(self, root, symbols):
		ttk.Combobox.__init__(self, root, values=symbols)

		self.selected_symbol = symbols[0]
		self.set(self.selected_symbol)
		self.bind('<<ComboboxSelected>>', self.selector_select)
		self.pack()

	def selector_select(self, event):
		self.selected_symbol = self.get()
