import numpy as np


class WMA:
	def __init__(self):
		pass

	@staticmethod
	def about_me():
		ret = {
			'required_fields':
				{
					'Period':
						{
							'min': 1,
							'max': 200,
						},
				},
			'colors':
				[
					'Color'
				]
		}
		return ret

	def calc_output_len(self, data_len, params):
		period = params['Period']
		return [data_len - period + 1]

	def calc(self, df, symbols, params):
		period = params['Period']
		column_names = sum([[symbol + column for column in ['-Close-price']] for symbol in symbols], [])
		data = df[column_names].values.astype(np.float32)

		kernel = np.flip(np.arange(1, period + 1, 1, dtype=np.float32) / np.float32(period * (period + 1) / 2))
		return np.apply_along_axis(lambda x: np.convolve(x, kernel, mode='valid'), axis=0, arr=data)

