from tensorflow.keras.layers import Input, Dense, Reshape, Flatten, Conv2D
from tensorflow.keras.models import Model
from ctypes import CDLL, c_int32, POINTER
import numpy as np


class CNN:
	def __init__(self):
		self.known_len = 8
		self.dll_indicators = CDLL('./Analyzers/indicators.dll')
		self.dll_indicators.rsi.argtypes = [
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data
			c_int32,                                                                 # int data_len
			c_int32,                                                                 # int period
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* output
			POINTER(c_int32)                                                         # int* output_len
		]
		self.dll_indicators.ema.argtypes = [
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data_close
			c_int32,                                                                 # int data_len
			c_int32,                                                                 # int period
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* output
			POINTER(c_int32)                                                         # int* output_len
		]
		self.dll_indicators.mfi.argtypes = [
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data_high
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data_low
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data_close
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* data_volume
			c_int32,                                                                 # int data_len
			c_int32,                                                                 # int period
			np.ctypeslib.ndpointer(dtype=np.float32, ndim=1, flags='C_CONTIGUOUS'),  # float* output
			POINTER(c_int32)                                                         # int* output_len
		]

		self.model_btc = self._net_btc()
		self.model_eth = self._net_eth()
		self.model_ltc = self._net_ltc()
		self.model_btc.load_weights('./Analyzers/Weights/weight_btc.h5')
		self.model_eth.load_weights('./Analyzers/Weights/weight_eth.h5')
		self.model_ltc.load_weights('./Analyzers/Weights/weight_ltc.h5')

	@staticmethod
	def about_me():
		ret = {
			'required_fields':
				{
					'BTC Threshold Up':
						{
							'min': 0,
							'max': 100,
						},
					'BTC Threshold Down':
						{
							'min': 0,
							'max': 100,
						},
					'ETH Threshold Up':
						{
							'min': 0,
							'max': 100,
						},
					'ETH Threshold Down':
						{
							'min': 0,
							'max': 100,
						},
					'LTC Threshold Up':
						{
							'min': 0,
							'max': 100,
						},
					'LTC Threshold Down':
						{
							'min': 0,
							'max': 100,
						},
				},
			'colors':
				[
					'Color'
				]
		}
		return ret

	def calc_output_len(self, data_len, params):
		return [1000, 1000, 1000]

	def _net_btc(self):
		i = Input(shape=(8, 93))
		r = Reshape(target_shape=(8, 93, 1))(i)
		c1 = Conv2D(24, (3, 93), activation='relu')(r)
		f = Flatten()(c1)
		o = Dense(1, activation='sigmoid')(f)
		return Model(inputs=i, outputs=o)

	def _net_eth(self):
		i = Input(shape=(8, 93))
		r = Reshape(target_shape=(8, 93, 1))(i)
		c1 = Conv2D(32, (3, 93), activation='relu')(r)
		f = Flatten()(c1)
		o = Dense(1, activation='sigmoid')(f)
		return Model(inputs=i, outputs=o)

	def _net_ltc(self):
		i = Input(shape=(8, 93))
		r = Reshape(target_shape=(8, 93, 1))(i)
		c1 = Conv2D(32, (3, 93), activation='relu')(r)
		f = Flatten()(c1)
		o = Dense(1, activation='sigmoid')(f)
		return Model(inputs=i, outputs=o)

	def calc(self, df, symbols, params):
		threshold_btc_up = params['BTC Threshold Up'] / 100.0
		threshold_btc_down = params['BTC Threshold Down'] / 100.0
		threshold_eth_up = params['ETH Threshold Up'] / 100.0
		threshold_eth_down = params['ETH Threshold Down'] / 100.0
		threshold_ltc_up = params['LTC Threshold Up'] / 100.0
		threshold_ltc_down = params['LTC Threshold Down'] / 100.0

		btc_high = np.array(df['BTCEUR-High-price'].values, dtype=np.float32)
		btc_low = np.array(df['BTCEUR-Low-price'].values, dtype=np.float32)
		btc_close = np.array(df['BTCEUR-Close-price'].values, dtype=np.float32)
		btc_volume = np.array(df['BTCEUR-Volume'].values, dtype=np.float32)
		btc_volume_buy = np.array(df['BTCEUR-Taker-buy-base-asset-volume'].values, dtype=np.float32)
		btc_num_of_trades = np.array(df['BTCEUR-Number-of-trades'].values, dtype=np.float32)
		btc_volume_sell = btc_volume - btc_volume_buy
		eth_high = np.array(df['ETHEUR-High-price'].values, dtype=np.float32)
		eth_low = np.array(df['ETHEUR-Low-price'].values, dtype=np.float32)
		eth_close = np.array(df['ETHEUR-Close-price'].values, dtype=np.float32)
		eth_volume = np.array(df['ETHEUR-Volume'].values, dtype=np.float32)
		eth_volume_buy = np.array(df['ETHEUR-Taker-buy-base-asset-volume'].values, dtype=np.float32)
		eth_num_of_trades = np.array(df['ETHEUR-Number-of-trades'].values, dtype=np.float32)
		eth_volume_sell = eth_volume - eth_volume_buy
		ltc_high = np.array(df['LTCEUR-High-price'].values, dtype=np.float32)
		ltc_low = np.array(df['LTCEUR-Low-price'].values, dtype=np.float32)
		ltc_close = np.array(df['LTCEUR-Close-price'].values, dtype=np.float32)
		ltc_volume = np.array(df['LTCEUR-Volume'].values, dtype=np.float32)
		ltc_volume_buy = np.array(df['LTCEUR-Taker-buy-base-asset-volume'].values, dtype=np.float32)
		ltc_num_of_trades = np.array(df['LTCEUR-Number-of-trades'].values, dtype=np.float32)
		ltc_volume_sell = ltc_volume - ltc_volume_buy

		btc_close_min = np.min(btc_close)
		btc_close_max = np.max(btc_close)
		btc_close_last = btc_close[-self.known_len:]
		if btc_close_min == btc_close_max:
			btc_d = 1.0
		else:
			btc_d = btc_close_max - btc_close_min
		btc_close_last = (btc_close_last - btc_close_min) / btc_d
		btc_volume_min = np.min(btc_volume)
		btc_volume_max = np.max(btc_volume)
		btc_volume_buy_last = btc_volume_buy[-self.known_len:]
		btc_volume_sell_last = btc_volume_sell[-self.known_len:]
		if btc_volume_min == btc_volume_max:
			btc_volume_buy_last = btc_volume_buy_last - btc_volume_min
			btc_volume_sell_last = btc_volume_sell_last - btc_volume_min
		else:
			btc_volume_buy_last = (btc_volume_buy_last - btc_volume_min) / (btc_volume_max - btc_volume_min)
			btc_volume_sell_last = (btc_volume_sell_last - btc_volume_min) / (btc_volume_max - btc_volume_min)
		btc_not_min = np.min(btc_num_of_trades)
		btc_not_max = np.max(btc_num_of_trades)
		btc_num_of_trades_last = btc_num_of_trades[-self.known_len:]
		if btc_not_min == btc_not_max:
			btc_num_of_trades_last = btc_num_of_trades_last - btc_not_min
		else:
			btc_num_of_trades_last = (btc_num_of_trades_last - btc_not_min) / (btc_not_max - btc_not_min)
		eth_close_min = np.min(eth_close)
		eth_close_max = np.max(eth_close)
		eth_close_last = eth_close[-self.known_len:]
		if eth_close_min == eth_close_max:
			eth_d = 1.0
		else:
			eth_d = eth_close_max - eth_close_min
		eth_close_last = (eth_close_last - eth_close_min) / eth_d
		eth_volume_min = np.min(eth_volume)
		eth_volume_max = np.max(eth_volume)
		eth_volume_buy_last = eth_volume_buy[-self.known_len:]
		eth_volume_sell_last = eth_volume_sell[-self.known_len:]
		if eth_volume_min == eth_volume_max:
			eth_volume_buy_last = eth_volume_buy_last - eth_volume_min
			eth_volume_sell_last = eth_volume_sell_last - eth_volume_min
		else:
			eth_volume_buy_last = (eth_volume_buy_last - eth_volume_min) / (eth_volume_max - eth_volume_min)
			eth_volume_sell_last = (eth_volume_sell_last - eth_volume_min) / (eth_volume_max - eth_volume_min)
		eth_not_min = np.min(eth_num_of_trades)
		eth_not_max = np.max(eth_num_of_trades)
		eth_num_of_trades_last = eth_num_of_trades[-self.known_len:]
		if eth_not_min == eth_not_max:
			eth_num_of_trades_last = eth_num_of_trades_last - eth_not_min
		else:
			eth_num_of_trades_last = (eth_num_of_trades_last - eth_not_min) / (eth_not_max - eth_not_min)
		ltc_close_min = np.min(ltc_close)
		ltc_close_max = np.max(ltc_close)
		ltc_close_last = ltc_close[-self.known_len:]
		if ltc_close_min == ltc_close_max:
			ltc_d = 1.0
		else:
			ltc_d = ltc_close_max - ltc_close_min
		ltc_close_last = (ltc_close_last - ltc_close_min) / ltc_d
		ltc_volume_min = np.min(ltc_volume)
		ltc_volume_max = np.max(ltc_volume)
		ltc_volume_buy_last = ltc_volume_buy[-self.known_len:]
		ltc_volume_sell_last = ltc_volume_sell[-self.known_len:]
		if ltc_volume_min == ltc_volume_max:
			ltc_volume_buy_last = ltc_volume_buy_last - ltc_volume_min
			ltc_volume_sell_last = ltc_volume_sell_last - ltc_volume_min
		else:
			ltc_volume_buy_last = (ltc_volume_buy_last - ltc_volume_min) / (ltc_volume_max - ltc_volume_min)
			ltc_volume_sell_last = (ltc_volume_sell_last - ltc_volume_min) / (ltc_volume_max - ltc_volume_min)
		ltc_not_min = np.min(ltc_num_of_trades)
		ltc_not_max = np.max(ltc_num_of_trades)
		ltc_num_of_trades_last = ltc_num_of_trades[-self.known_len:]
		if ltc_not_min == ltc_not_max:
			ltc_num_of_trades_last = ltc_num_of_trades_last - ltc_not_min
		else:
			ltc_num_of_trades_last = (ltc_num_of_trades_last - ltc_not_min) / (ltc_not_max - ltc_not_min)

		known_last = np.zeros((self.known_len, 93), dtype=np.float32)
		ind_c = np.zeros(1000, dtype=np.float32)
		ind_c_len = c_int32()

		known_last[:, 0] = btc_close_last
		known_last[:, 1] = btc_volume_buy_last
		known_last[:, 2] = btc_volume_sell_last
		known_last[:, 3] = btc_num_of_trades_last
		self.dll_indicators.rsi(btc_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 4] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 5] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 6] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 7] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 8] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 9] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 10] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 11] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(btc_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 12] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 3, ind_c, ind_c_len)
		known_last[:, 13] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 5, ind_c, ind_c_len)
		known_last[:, 14] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 7, ind_c, ind_c_len)
		known_last[:, 15] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 9, ind_c, ind_c_len)
		known_last[:, 16] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 11, ind_c, ind_c_len)
		known_last[:, 17] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 13, ind_c, ind_c_len)
		known_last[:, 18] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 15, ind_c, ind_c_len)
		known_last[:, 19] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 17, ind_c, ind_c_len)
		known_last[:, 20] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(btc_high, btc_low, btc_close, btc_volume, 1000, 19, ind_c, ind_c_len)
		known_last[:, 21] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.ema(btc_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 22] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 23] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 24] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 25] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 26] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 27] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 28] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 29] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		self.dll_indicators.ema(btc_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 30] = (ind_c[:ind_c_len.value][-self.known_len:] - btc_close_min) / btc_d
		known_last[:, 31] = eth_close_last
		known_last[:, 32] = eth_volume_buy_last
		known_last[:, 33] = eth_volume_sell_last
		known_last[:, 34] = eth_num_of_trades_last
		self.dll_indicators.rsi(eth_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 35] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 36] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 37] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 38] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 39] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 40] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 41] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 42] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(eth_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 43] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 3, ind_c, ind_c_len)
		known_last[:, 44] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 5, ind_c, ind_c_len)
		known_last[:, 45] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 7, ind_c, ind_c_len)
		known_last[:, 46] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 9, ind_c, ind_c_len)
		known_last[:, 47] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 11, ind_c, ind_c_len)
		known_last[:, 48] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 13, ind_c, ind_c_len)
		known_last[:, 49] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 15, ind_c, ind_c_len)
		known_last[:, 50] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 17, ind_c, ind_c_len)
		known_last[:, 51] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(eth_high, eth_low, eth_close, eth_volume, 1000, 19, ind_c, ind_c_len)
		known_last[:, 52] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.ema(eth_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 53] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 54] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 55] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 56] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 57] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 58] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 59] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 60] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		self.dll_indicators.ema(eth_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 61] = (ind_c[:ind_c_len.value][-self.known_len:] - eth_close_min) / eth_d
		known_last[:, 62] = ltc_close_last
		known_last[:, 63] = ltc_volume_buy_last
		known_last[:, 64] = ltc_volume_sell_last
		known_last[:, 65] = ltc_num_of_trades_last
		self.dll_indicators.rsi(ltc_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 66] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 67] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 68] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 69] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 70] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 71] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 72] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 73] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.rsi(ltc_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 74] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 3, ind_c, ind_c_len)
		known_last[:, 75] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 5, ind_c, ind_c_len)
		known_last[:, 76] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 7, ind_c, ind_c_len)
		known_last[:, 77] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 9, ind_c, ind_c_len)
		known_last[:, 78] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 11, ind_c, ind_c_len)
		known_last[:, 79] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 13, ind_c, ind_c_len)
		known_last[:, 80] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 15, ind_c, ind_c_len)
		known_last[:, 81] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 17, ind_c, ind_c_len)
		known_last[:, 82] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.mfi(ltc_high, ltc_low, ltc_close, ltc_volume, 1000, 19, ind_c, ind_c_len)
		known_last[:, 83] = ind_c[:ind_c_len.value][-self.known_len:] / 100.0
		self.dll_indicators.ema(ltc_close, 1000, 3, ind_c, ind_c_len)
		known_last[:, 84] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 5, ind_c, ind_c_len)
		known_last[:, 85] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 7, ind_c, ind_c_len)
		known_last[:, 86] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 9, ind_c, ind_c_len)
		known_last[:, 87] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 11, ind_c, ind_c_len)
		known_last[:, 88] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 13, ind_c, ind_c_len)
		known_last[:, 89] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 15, ind_c, ind_c_len)
		known_last[:, 90] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 17, ind_c, ind_c_len)
		known_last[:, 91] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d
		self.dll_indicators.ema(ltc_close, 1000, 19, ind_c, ind_c_len)
		known_last[:, 92] = (ind_c[:ind_c_len.value][-self.known_len:] - ltc_close_min) / ltc_d

		known_last = known_last.reshape((1, 8, 93))

		prediction_btc_value = self.model_btc.predict(known_last)[0, 0]
		prediction_eth_value = self.model_eth.predict(known_last)[0, 0]
		prediction_ltc_value = self.model_ltc.predict(known_last)[0, 0]

		prediction_btc_label = 2 if prediction_btc_value >= threshold_btc_up else 0 if prediction_btc_value < threshold_btc_down else 1
		prediction_eth_label = 2 if prediction_eth_value >= threshold_eth_up else 0 if prediction_eth_value < threshold_eth_down else 1
		prediction_ltc_label = 2 if prediction_ltc_value >= threshold_ltc_up else 0 if prediction_ltc_value < threshold_ltc_down else 1

		return prediction_btc_value, prediction_btc_label, prediction_eth_value, prediction_eth_label, prediction_ltc_value, prediction_ltc_label
