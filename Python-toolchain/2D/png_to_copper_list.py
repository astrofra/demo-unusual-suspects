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

		l=list(b[2])
		for j in range(0, h):
			print(l[j])

		return 1


main()