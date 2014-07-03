## png_to_copper_list.py

import png

filename_in = ['face_all.png']

def main():
	print('png_to_copper_list')

	for _filename in filename_in:
		print('Loading bitmap : ' + _filename)
		png_buffer = png.Reader(filename = _filename)
		b = png_buffer.read()
		# print(b)

		w = b[0]
		h = b[1]
		print('w = ' + str(w) + ', h = ' + str(h))
		print('bitdepth = ' + str(b[3]['bitdepth']))

		if b[3]['greyscale']:
			print('!!!Error, cannot process greyscale image :(')
			return 0

		buffer_in = list(b[2])
		for j in range(0, h):
			line_stat = {}
			for p in buffer_in[j]:
				if str(p) in line_stat:
					line_stat[str(p)] += 1
				else:
					line_stat[str(p)] = 1

			print('Found ' + str(len(line_stat)) + ' colors in line ' + str(j) + '.')
			print(line_stat)

		return 1


main()