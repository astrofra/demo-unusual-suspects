## png_to_copper_list.py

import png
import math

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

			print('Found ' + str(len(line_stat)) + ' colors in line ' + str(j) + '.')
			original_line_palette = []
			for color_index_by_occurence in sorted(line_stat, key=line_stat.get):
				original_line_palette.append(original_palette[int(color_index_by_occurence)])

			##	If there's less than 32 colors on the same row
			##	the hardware can handle it natively, there's no need
			##	to reduce the palette.
			if len(original_line_palette) <= 32:
				optimized_line_palette = original_line_palette
			else:
			##	If there's more than 32 colors on the same row
			##	the hardware cannot handle it, so the palette
			##	must be reduced.
				optimized_line_palette = []
				found_a_color = True
				while len(optimized_line_palette) < 32 and len(original_line_palette) > 0 and found_a_color is True:
					found_a_color = False
					for _color in original_line_palette:
						if _color[0] > 127 and _color[1] > 127 and _color[2] > 127:
							optimized_line_palette.append(_color)
							original_line_palette.remove(_color)
							found_a_color = True

				while len(optimized_line_palette) < 32 and len(original_line_palette) > 0:
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