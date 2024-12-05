from ctypes import CDLL, c_uint64, c_uint32, c_float
import tkinter as tk
import numpy as np


class GraphDrawer:
	def __init__(self, main_frame):
		self.dll = CDLL('./GraphDrawer/opengl_drawer.dll')
		self.dll.Initialize.argtypes = [c_uint64]
		self.dll.Destructor.argtypes = []
		self.dll.Redraw.argtypes = [
			c_uint32,                                                                # UINT32 width,
			c_uint32,                                                                # UINT32 height,

			c_uint32,                                                                # UINT32 mouse_in_canvas,
			c_uint32,                                                                # UINT32 mouse_x,
			c_uint32,                                                                # UINT32 mouse_y,
			c_uint32,                                                                # UINT32 size_x,

			c_uint64,                                                                # UINT64 timestamp,

			c_uint32,                                                                # UINT32 data_rows,
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=2, flags='C_CONTIGUOUS'),  # FLOAT* data,

			c_uint32,                                                                # UINT32 analyse1_config_rows,
			np.ctypeslib.ndpointer(dtype=np.uint32, ndim=2, flags='C_CONTIGUOUS'),   # UINT32* analyse1_config,
			c_uint32,                                                                # UINT32 analyse1_config_cols,
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=2, flags='C_CONTIGUOUS'),  # FLOAT* analyse1_data,

			c_uint32,                                                                # UINT32 analyse2_config_rows,
			np.ctypeslib.ndpointer(dtype=np.uint32, ndim=2, flags='C_CONTIGUOUS'),   # UINT32* analyse2_config,
			c_uint32,                                                                # UINT32 analyse2_config_cols,
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=2, flags='C_CONTIGUOUS'),  # FLOAT* analyse2_data,

			c_uint32,                                                                # UINT32 analyse3_config_rows,
			np.ctypeslib.ndpointer(dtype=np.uint32, ndim=2, flags='C_CONTIGUOUS'),   # UINT32* analyse3_config,
			c_uint32,                                                                # UINT32 analyse4_config_cols,
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=2, flags='C_CONTIGUOUS'),  # FLOAT* analyse3_data,

			c_uint32,                                                                # UINT32 nn_active,
			c_float,                                                                 # FLOAT nn_selected_value,
			c_uint32,                                                                # UINT32 nn_selected_label,
			c_float,                                                                 # FLOAT nn_btc_value,
			c_uint32,                                                                # UINT32 nn_btc_label,
			c_float,                                                                 # FLOAT nn_eth_value,
			c_uint32,                                                                # UINT32 nn_eth_label,
			c_float,                                                                 # FLOAT nn_ltc_value,
			c_uint32,                                                                # UINT32 nn_ltc_label,
			c_uint32,                                                                # UINT32 nn_r,
			c_uint32,                                                                # UINT32 nn_g,
			c_uint32,                                                                # UINT32 nn_b,
		]

		self.canvas = tk.Canvas(main_frame, highlightthickness=0)
		self.canvas.bind("<MouseWheel>", self.mouse_canvas_scroll)
		self.canvas.bind("<Motion>", self.mouse_canvas_coordinates)
		self.canvas.bind("<Enter>", self.mouse_canvas_enter)
		self.canvas.bind("<Leave>", self.mouse_canvas_leave)
		self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

		self.mouse_x = 0
		self.mouse_y = 0
		self.mouse_in_canvas = 0
		self.mouse_scroll_min = 1
		self.mouse_scroll = 2
		self.mouse_scroll_max = 20
		self.mouse_scroll_scale = 5

		self.height_info = 100
		self.height_timeline = 10
		self.height_volumes = 200
		self.height_graph_padding = 10
		self.height_volumes_padding = 10
		self.width_unknown = 300

		self.dll.Initialize(self.canvas.winfo_id())

	def redraw(self, df, symbol, symbols, analysis, active_names):
		width = self.canvas.winfo_width()
		height = self.canvas.winfo_height()
		column_names = [symbol + x for x in ['-Open-price', '-High-price', '-Low-price', '-Close-price', '-Volume', '-Taker-buy-base-asset-volume']]
		data = df[column_names].values.astype(np.float32).copy(order='C')
		timestamp = df[symbol + '-Kline-open-time'].iloc[-1] if len(data) > 0 else 0

		# extract all
		symbol_idx = symbols.index(symbol)
		done_types = []
		done_names = []
		done_colors = []
		done_timestamp = []
		done_data = []
		for row in analysis:
			done_types.append(row[0])
			done_names.append(row[1])
			done_colors.append(row[2])
			done_timestamp.append(row[3])
			done_data.append(row[4])  # [B, T, C] lub [K[B, T, C]20, D[B, T, C]15, J[B, T, C]10]

		# filter active and actual and group
		type_to_int = {'SMA': 1, 'EMA': 2, 'WMA': 3, 'VWAP': 4, 'TRIX': 5, 'RSI': 6, 'MFI': 7, 'StochRSI': 8, 'MACD': 9, 'KDJ': 10, 'CNN': 11}
		type_to_group = {'SMA': 1, 'EMA': 1, 'WMA': 1, 'VWAP': 1, 'TRIX': 1, 'RSI': 1, 'MFI': 1, 'StochRSI': 2, 'MACD': 3, 'KDJ': 3, 'CNN': 4}
		max_len1 = 0
		max_len2 = 0
		max_len3 = 0
		analyse1_config = []  # type, len,            color_r, color_g, color_b
		analyse2_config = []  # type, len, len,       color_r, color_g, color_b, color_r, color_g, color_b
		analyse3_config = []  # type, len, len, len,  color_r, color_g, color_b, color_r, color_g, color_b, color_r, color_g, color_b
		analyse1_data = []
		analyse2_data = []
		analyse3_data = []
		nn_active = 0
		nn_selected_value = 0.0
		nn_selected_label = 0
		nn_btc_value = 0.0
		nn_btc_label = 0
		nn_eth_value = 0.0
		nn_eth_label = 0
		nn_ltc_value = 0.0
		nn_ltc_label = 0
		nn_r = 0
		nn_g = 0
		nn_b = 0
		for name in active_names:
			if name in done_names:
				idx = done_names.index(name)
				if timestamp == done_timestamp[idx]:
					group = type_to_group[done_types[idx]]
					config_type = type_to_int[done_types[idx]]
					if group == 1:
						config_len = len(done_data[idx])
						if config_len > max_len1:
							max_len1 = config_len
						config_color_r, config_color_g, config_color_b = [int(x, 16) for x in [done_colors[idx][0][y:y + 2] for y in range(1, 7, 2)]]

						analyse1_config.append([config_type, config_len, config_color_r, config_color_g, config_color_b])
						analyse1_data.append(done_data[idx][:, symbol_idx])
					elif group == 2:
						config_color1_r, config_color1_g, config_color1_b = [int(x, 16) for x in [done_colors[idx][0][y:y + 2] for y in range(1, 7, 2)]]
						config_color2_r, config_color2_g, config_color2_b = [int(x, 16) for x in [done_colors[idx][1][y:y + 2] for y in range(1, 7, 2)]]

						data1 = done_data[idx][0][:, symbol_idx]
						data2 = done_data[idx][1][:, symbol_idx]
						config_len1 = len(data1)
						config_len2 = len(data2)
						if max(config_len1, config_len2) > max_len2:
							max_len2 = max(config_len1, config_len2)

						analyse2_config.append([config_type, config_len1, config_len2, config_color1_r, config_color1_g, config_color1_b, config_color2_r, config_color2_g, config_color2_b])
						analyse2_data.append(data1)
						analyse2_data.append(data2)
					elif group == 3:
						config_color1_r, config_color1_g, config_color1_b = [int(x, 16) for x in [done_colors[idx][0][y:y + 2] for y in range(1, 7, 2)]]
						config_color2_r, config_color2_g, config_color2_b = [int(x, 16) for x in [done_colors[idx][1][y:y + 2] for y in range(1, 7, 2)]]
						config_color3_r, config_color3_g, config_color3_b = [int(x, 16) for x in [done_colors[idx][2][y:y + 2] for y in range(1, 7, 2)]]

						data1 = done_data[idx][0][:, symbol_idx]
						data2 = done_data[idx][1][:, symbol_idx]
						data3 = done_data[idx][2][:, symbol_idx]
						config_len1 = len(data1)
						config_len2 = len(data2)
						config_len3 = len(data3)
						if max(config_len1, config_len2, config_len3) > max_len3:
							max_len3 = max(config_len1, config_len2, config_len3)

						analyse3_config.append([config_type, config_len1, config_len2, config_len3, config_color1_r, config_color1_g, config_color1_b, config_color2_r, config_color2_g, config_color2_b, config_color3_r, config_color3_g, config_color3_b])
						analyse3_data.append(data1)
						analyse3_data.append(data2)
						analyse3_data.append(data3)
					elif group == 4:
						nn_active = 1
						nn_btc_value = done_data[idx][0]
						nn_btc_label = done_data[idx][1]
						nn_eth_value = done_data[idx][2]
						nn_eth_label = done_data[idx][3]
						nn_ltc_value = done_data[idx][4]
						nn_ltc_label = done_data[idx][5]
						if symbol == 'BTCEUR':
							nn_selected_value = nn_btc_value
							nn_selected_label = nn_btc_label
						elif symbol == 'LTCEUR':
							nn_selected_value = nn_ltc_value
							nn_selected_label = nn_ltc_label
						elif symbol == 'ETHEUR':
							nn_selected_value = nn_eth_value
							nn_selected_label = nn_eth_label
						nn_r, nn_g, nn_b = [int(x, 16) for x in [done_colors[idx][0][y:y + 2] for y in range(1, 7, 2)]]

		if len(analyse1_config) == 0:
			analyse1_config_to_draw = np.zeros((0, 0), dtype=np.uint32)
		else:
			analyse1_config_to_draw = np.array(analyse1_config, dtype=np.uint32)
		if len(analyse2_config) == 0:
			analyse2_config_to_draw = np.zeros((0, 0), dtype=np.uint32)
		else:
			analyse2_config_to_draw = np.array(analyse2_config, dtype=np.uint32)
		if len(analyse3_config) == 0:
			analyse3_config_to_draw = np.zeros((0, 0), dtype=np.uint32)
		else:
			analyse3_config_to_draw = np.array(analyse3_config, dtype=np.uint32)

		analyse1_data_to_draw = np.zeros((len(analyse1_data), max_len1), dtype=np.float32)
		analyse2_data_to_draw = np.zeros((len(analyse2_data), max_len2), dtype=np.float32)
		analyse3_data_to_draw = np.zeros((len(analyse3_data), max_len3), dtype=np.float32)
		for x, sublist in enumerate(analyse1_data):
			analyse1_data_to_draw[x, :len(sublist)] = sublist
		for x, sublist in enumerate(analyse2_data):
			analyse2_data_to_draw[x, :len(sublist)] = sublist
		for x, sublist in enumerate(analyse3_data):
			analyse3_data_to_draw[x, :len(sublist)] = sublist

		self.dll.Redraw(
			width,
			height,

			self.mouse_in_canvas,
			self.mouse_x,
			self.mouse_y,
			self.mouse_scroll * self.mouse_scroll_scale,

			timestamp,

			data.shape[0],
			data,

			analyse1_config_to_draw.shape[0],
			analyse1_config_to_draw,
			analyse1_data_to_draw.shape[1],
			analyse1_data_to_draw,

			analyse2_config_to_draw.shape[0],
			analyse2_config_to_draw,
			analyse2_data_to_draw.shape[1],
			analyse2_data_to_draw,

			analyse3_config_to_draw.shape[0],
			analyse3_config_to_draw,
			analyse3_data_to_draw.shape[1],
			analyse3_data_to_draw,

			nn_active,
			nn_selected_value,
			nn_selected_label,
			nn_btc_value,
			nn_btc_label,
			nn_eth_value,
			nn_eth_label,
			nn_ltc_value,
			nn_ltc_label,
			nn_r,
			nn_g,
			nn_b
		)

	def mouse_canvas_coordinates(self, event):
		self.mouse_x, self.mouse_y = event.x, event.y

	def mouse_canvas_enter(self, event):
		self.mouse_in_canvas = 1

	def mouse_canvas_leave(self, event):
		self.mouse_in_canvas = 0

	def mouse_canvas_scroll(self, event):
		if event.delta < 0:
			self.mouse_scroll -= 1
			if self.mouse_scroll < self.mouse_scroll_min:
				self.mouse_scroll = self.mouse_scroll_min
		else:
			self.mouse_scroll += 1
			if self.mouse_scroll > self.mouse_scroll_max:
				self.mouse_scroll = self.mouse_scroll_max

	def __del__(self):
		self.dll.Destructor()
