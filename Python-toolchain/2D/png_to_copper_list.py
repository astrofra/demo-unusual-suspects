## png_to_copper_list.py

import png
import math
import colorsys

def color_distance(color1, color2):
	return math.sqrt(sum([(e1-e2)**2 for e1, e2 in zip(color1, color2)]))

def color_best_match(sample, colors):
	by_distance = sorted(colors, key=lambda c: color_distance(c, sample))
	return by_distance[0]

filename_in = ['face_all.png']

def color_compute_EHB_value(_color):
	_new_color = [0,0,0]
	_new_color[0] = int(_color[0] / 2)
	_new_color[1] = int(_color[1] / 2)
	_new_color[2] = int(_color[2] / 2)
	return _new_color

global g_max_color_per_line 
g_max_color_per_line = 32

def main():
	print('png_to_copper_list')

	for _filename in filename_in:
		print('Loading bitmap : ' + _filename)
		##	Loads the PNG image
		png_buffer = png.Reader(filename = _filename)
		b = png_buffer.read()
		# print(b)

		##	Get size & depth
		w = b[0]
		h = b[1]
		print('w = ' + str(w) + ', h = ' + str(h))
		print('bitdepth = ' + str(b[3]['bitdepth']))

		if b[3]['greyscale']:
			print('!!!Error, cannot process a greyscale image :(')
			return 0

		if b[3]['bitdepth'] > 8:
			print('!!!Error, cannot process a true color image :(')
			return 0

		original_palette  =	b[3]['palette']

		png_out_buffer = []
		prev_optimized_line_palette = []

		buffer_in = list(b[2])
		##	For each line of the image
		for j in range(0, h):
			line_stat = {}
			optimized_line_palette = []

			##	Calculate the occurence of each color index
			##	used in the current pixel row
			for p in buffer_in[j]:
				if str(p) in line_stat:
					line_stat[str(p)] += 1
				else:
					line_stat[str(p)] = 1

			##	Occurence weighted by the saturation of the color
			for color_index_by_occurence in line_stat:
				_rgb_color = original_palette[int(color_index_by_occurence)]
				w_factor = (colorsys.rgb_to_hls(_rgb_color[0], _rgb_color[1], _rgb_color[2])[2] * 2.0) + 1.0
				line_stat[color_index_by_occurence] = int(w_factor * line_stat[color_index_by_occurence])


			print('Found ' + str(len(line_stat)) + ' colors in line ' + str(j) + '.')
			original_line_palette = []
			for color_index_by_occurence in sorted(line_stat, key=line_stat.get):
				original_line_palette.append(original_palette[int(color_index_by_occurence)])

			##	If there's less than 32 colors on the same row
			##	the hardware can handle it natively, there's no need
			##	to reduce the palette.
			if len(original_line_palette) <= g_max_color_per_line:
				optimized_line_palette = original_line_palette
				for i in range(0, g_max_color_per_line - len(original_line_palette)):
					original_line_palette.append(original_palette[0])
			else:
			##	If there's more than 32 colors on the same row
			##	the hardware cannot handle it, so the palette
			##	must be reduced.
				optimized_line_palette = []
				found_a_color = True
				while len(optimized_line_palette) < g_max_color_per_line and len(original_line_palette) > 0 and found_a_color is True:
					found_a_color = False
					for _color in original_line_palette:
						if _color[0] > 127 and _color[1] > 127 and _color[2] > 127:
							optimized_line_palette.append(_color)
							original_line_palette.remove(_color)
							found_a_color = True

				while len(optimized_line_palette) < g_max_color_per_line and len(original_line_palette) > 0:
					for _color in original_line_palette:
						optimized_line_palette.append(_color)
						original_line_palette.remove(_color)
						break

				print('Temporary line palette is ' + str(len(optimized_line_palette)) + ' colors.')

				_tmp_palette = list(optimized_line_palette)
				for _color in _tmp_palette:
					optimized_line_palette.append(color_compute_EHB_value(_color))

				print('Final line palette is ' + str(len(optimized_line_palette)) + ' colors.')
				# print(optimized_line_palette)

				# original_line_palette[0:31]

			##	Mix the line palette with the previous line palette
			##	So that the copper will only have to change 16 colors per line
			# _col_start = 0
			# if j % 2 == 0:
			# 	_col_start = 1
			# if len(prev_optimized_line_palette) > 0:
			# 	for _idx in range(0, len(prev_optimized_line_palette), 2):
			# 		if _idx < len(optimized_line_palette):
			# 			optimized_line_palette[_idx] = prev_optimized_line_palette[_idx]
			
			prev_optimized_line_palette = list(optimized_line_palette)

			##	Remap the current line
			##	Using the optimized palette
			line_out_buffer = []
			for p in buffer_in[j]:
				current_pixel_color = original_palette[p]
				if current_pixel_color in optimized_line_palette:
					##	We found the exact color we were looking for
					line_out_buffer.append(current_pixel_color[0])
					line_out_buffer.append(current_pixel_color[1])
					line_out_buffer.append(current_pixel_color[2])
				else:
					_color_match = color_best_match(current_pixel_color, optimized_line_palette)
					line_out_buffer.append(_color_match[0])
					line_out_buffer.append(_color_match[1])
					line_out_buffer.append(_color_match[2])

			png_out_buffer.append(line_out_buffer)

		if len(png_out_buffer) > 0:
			_filename_out = _filename.replace('.png', '_out.png')
			f = open(_filename_out, 'wb')
			w = png.Writer(w, h)
			w.write(f, png_out_buffer)
			f.close()

			print(original_line_palette)

		return 1


main()