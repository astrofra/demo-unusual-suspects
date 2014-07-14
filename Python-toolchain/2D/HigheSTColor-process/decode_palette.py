import struct
import codecs

palette_filename = 'data360_2000.pal'
index_filename = 'data360_2000.idx'
root_name = 'faces_all'
filename_out =	"../../../Assets/"

index_stride = 16
color_stride = 16

def main():

	##	Decode the index table
	index_count = 0
	f_out = codecs.open(filename_out + root_name + '_palettes.c', 'w')
	f_out.write('#include <exec/types.h>\n')
	f_out.write('#include <intuition/intuition.h>\n\n')

	f_out.write('/*	Color index to modify on each scanline, grouped by ' + str(index_stride) + ' */\n\n')

	f_out.write('int ' + root_name + '_index[] =' + '\n')
	f_out.write('{' + '\n')

	f_in = open(index_filename, "rb")
	while True:
		bytes = f_in.read(index_stride)
		if bytes != '':
			line_str = '\t'
			for i in range(0,index_stride):
				line_str += str(struct.unpack('>1B', bytes[i])[0]) + ','
				index_count += 1
			line_str += '\n'
			f_out.write(line_str);
		else:
			break
	f_in.close()

	f_out.write('};' + '\n')

	##	Decode the initial 32 coulors palette.
	f_in = open(palette_filename, "rb")
	f_out.write('/*	Initial palette of ' + str(color_stride * 2) + ' RGB4 colors. */\n\n')

	f_out.write('UWORD ' + root_name + '_PaletteRGB4[] =' + '\n')
	f_out.write('{' + '\n')

	line_str = '\t'
	for i in range(0, color_stride * 2):
		long_word = f_in.read(2)
		if long_word != '':
			color_str = struct.unpack('>H', long_word)[0]
			color_str = str(hex(color_str))
			line_str += color_str + ','
			# print(color_str)
		else:
			break

	line_str += '\n'
	f_out.write(line_str);

	f_out.write('};' + '\n')

	f_in.close()

	##	Decode the scanline palettes table.
	palette_count = 0
	f_in = open(palette_filename, "rb")
	f_out.write('/*	RGB4 colors, grouped by ' + str(color_stride) + ' */\n\n')

	f_out.write('UWORD ' + root_name + '_scanline_PaletteRGB4[] =' + '\n')
	f_out.write('{' + '\n')

	bytes = f_in.read(color_stride * 4)	##	Skip the first 32 colors

	long_word = ' '
	while long_word != '':
		line_str = '\t'
		for i in range(0, color_stride):
			long_word = f_in.read(2)
			if long_word != '':
				color_str = struct.unpack('>H', long_word)[0]
				color_str = str(hex(color_str))
				line_str += color_str + ','

				palette_count += 1
				# print(color_str)
			else:
				break

		line_str += '\n'
		f_out.write(line_str);

	f_out.write('};' + '\n')

	f_out.close()

	##	Makes the header file.
	f_out = codecs.open(filename_out + root_name + '_palettes.h', 'w')
	f_out.write('#include <exec/types.h>\n')
	f_out.write('#include <intuition/intuition.h>\n\n')

	f_out.write('extern int ' + root_name + '_index[' + str(index_count) + '];' + '\n')
	f_out.write('extern UWORD ' + root_name + '_PaletteRGB4[32];' + '\n')
	f_out.write('extern UWORD ' + root_name + '_scanline_PaletteRGB4[' + str(palette_count) + '];' + '\n')
	f_out.close()

main()