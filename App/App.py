from threading import Thread, Lock
import tkinter as tk
import asyncio
from App.MainWindow import MainWindow
from App.Analyzer import Analyzer
from App.Data import Data


class App:
	def __init__(self):
		self.timeseries_past = 1000
		self.interval_ms = 60_000
		self.locks = {'data_past': Lock(), 'analyzer_analysis': Lock(), 'analyzer_data': Lock(), 'analyzer_status': Lock(), 'analyzer_analyzers': Lock()}
		self.symbols = ['BTCEUR', 'LTCEUR', 'ETHEUR']
		self.data = Data(self.symbols, self.interval_ms, self.timeseries_past, self.locks)

		loop = asyncio.get_event_loop()
		self.data_past_thread = Thread(target=self.data.run, args=(loop, self.run_analyser,), daemon=True)
		self.data_past_thread.start()

		self.root = tk.Tk()
		self.main_window = MainWindow(
			self.root,
			self.symbols,
			self.get_analyzers,
			self.get_analysis,
			self.add_analyse,
			self.remove_analyse
		)

		self.analyzer_status = {'working': False, 'analyse_again': False}
		self.analyzer = Analyzer(self.locks, self.timeseries_past)

		self.analyzer.load_analyzers()

		self.frame_time = 1
		self.root.after(0, self.update_canvas)
		self.root.mainloop()

	def update_canvas(self):
		with self.locks['data_past']:
			df = self.data.data_past.copy()
		with self.locks['analyzer_data']:
			analysis = self.analyzer.data.copy()
		self.main_window.draw_canvas(df, analysis)
		# print(time())
		self.root.after(self.frame_time, self.update_canvas)

	def get_analyzers(self):
		with self.locks['analyzer_analyzers']:
			analyzers = self.analyzer.analyzers.copy()
		return analyzers

	def get_analysis(self):
		with self.locks['analyzer_analysis']:
			analysis = self.analyzer.analysis.copy()
		return analysis

	def add_analyse(self, type, name, color, parameters):
		result = self.analyzer.add_analysis(type, name, color, parameters)
		if result == []:
			self.main_window.checkboxes.add_checkbox(name)
			self.run_analyser()
		return result

	def remove_analyse(self, name):
		result = self.analyzer.remove_analysis(name)
		if result == []:
			self.main_window.checkboxes.remove_checkbox(name)
			self.run_analyser()
		return result

	def run_analyser(self, from_callback=False):
		with self.locks['analyzer_status']:
			if from_callback == False:
				if self.analyzer_status['working'] == False and self.analyzer_status['analyse_again'] == False:
					self.analyzer_status['working'] = True
					with self.locks['data_past']:
						df = self.data.data_past.copy()
					Thread(target=self.analyzer.update_analysis, args=(df, self.symbols, self.run_analyser, ), daemon=True).start()
				elif self.analyzer_status['working'] == True and self.analyzer_status['analyse_again'] == False:
					self.analyzer_status['analyse_again'] = True
				elif self.analyzer_status['working'] == True and self.analyzer_status['analyse_again'] == True:
					pass
				elif self.analyzer_status['working'] == False and self.analyzer_status['analyse_again'] == True:
					self.analyzer_status['working'] = True
					self.analyzer_status['analyse_again'] = False
					with self.locks['data_past']:
						df = self.data.data_past.copy()
					Thread(target=self.analyzer.update_analysis, args=(df, self.symbols, self.run_analyser,), daemon=True).start()
			elif from_callback == True:
				if self.analyzer_status['working'] == False and self.analyzer_status['analyse_again'] == False:
					pass
				elif self.analyzer_status['working'] == True and self.analyzer_status['analyse_again'] == False:
					self.analyzer_status['working'] = False
				elif self.analyzer_status['working'] == True and self.analyzer_status['analyse_again'] == True:
					self.analyzer_status['analyse_again'] = False
					with self.locks['data_past']:
						df = self.data.data_past.copy()
					Thread(target=self.analyzer.update_analysis, args=(df, self.symbols, self.run_analyser,), daemon=True).start()
				elif self.analyzer_status['working'] == False and self.analyzer_status['analyse_again'] == True:
					self.analyzer_status['working'] = True
					self.analyzer_status['analyse_again'] = False
					with self.locks['data_past']:
						df = self.data.data_past.copy()
					Thread(target=self.analyzer.update_analysis, args=(df, self.symbols, self.run_analyser,), daemon=True).start()
