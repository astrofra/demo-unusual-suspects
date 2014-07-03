## png_to_copper_list.py

import png

filename_in = ['face_all.png']

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
				optimized_line_palette = original_line_palette[0:31]

			##	Remap the current line
			##	Using the optimized palette
			line_out_buffer = []
			for p in buffer_in[j]:
				current_pixel_color = original_palette[p]
				if current_pixel_color in optimized_line_palette:
					line_out_buffer.append(current_pixel_color[0])
					line_out_buffer.append(current_pixel_color[1])
					line_out_buffer.append(current_pixel_color[2])
				else:
					line_out_buffer.append(255)
					line_out_buffer.append(0)
					line_out_buffer.append(255)

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