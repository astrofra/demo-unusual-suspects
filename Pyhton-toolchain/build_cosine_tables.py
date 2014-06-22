import os
import string
import codecs
import ast
import math
from vector3 import Vector3

filename_out			=	"../Assets/cosine_table.c"
table_size				=	360
fixed_point_precision 	=	512

def dumpCosine(_cosine_func, display_name, f):
	f.write('int ' + display_name + '[] =' + '\n')
	f.write('{' + '\n')

	_str_out = '\t'
	for angle in range(0,table_size):
		_cos = int(_cosine_func(angle) * fixed_point_precision)
		_str_out += str(_cos) + ','
		if angle%10 == 9:
			f.write(_str_out + '\n')
			_str_out = '\t'

	f.write('};' + '\n')

def  main():
	f = codecs.open(filename_out, 'w')

	dumpCosine(_cosine_func = math.cos, display_name = 'tcos', f = f)
	f.write('\n')
	dumpCosine(_cosine_func = math.sin, display_name = 'tsin', f = f)

	f.close()

main()