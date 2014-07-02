import os
import string
import codecs
import ast
from vector3 import Vector3

filename_list = ["face_00.obj", "amiga.obj", "cube.obj", "spiroid.obj"]
filename_out = ""
scale_factor = 100.0

def parse_obj_vector(_string):
	_args = _string.split(' ')
	_vector = Vector3(float(_args[1]), float(_args[2]), float(_args[3]))
	_vector *= scale_factor
	_vector.x = float(int(_vector.x))
	_vector.y = float(int(_vector.y))
	_vector.z = float(int(_vector.z))
	return _vector

def parse_obj_face(_string):
	## f 13//1 15//2 4//3 2//4
	_args = _string.split(' ')
	_args.pop(0)
	_face = []
	_vertex_index = -1
	_uv_index = -1
	_normal_index = -1
	for _arg in _args:
		_corner = _arg.split('/')
		_vertex_index = -1
		_uv_index = -1
		_normal_index = -1
		if len(_corner) > 0:
			if _corner[0] != '':
				_vertex_index = int(_corner[0]) - 1
			if _corner[1] != '':
				_uv_index = int(_corner[1]) - 1
			if _corner[2] != '':
				_normal_index = int(_corner[2]) - 1

		_face.append({'vertex':_vertex_index, 'uv':_uv_index, 'normal':_normal_index})

	# _face = _face[::-1]
	# _face.append(_face.pop(0))
	# _face.append(_face[0])

	return _face

def main():
	for filename_in in filename_list:
		face_list = []
		vertex_list = []
		normal_list = []

		f = codecs.open(filename_in, 'r')
		for line in f:
			# print(repr(line))
			if len(line) > 0:
				line = string.replace(line, '\t', ' ')
				line = string.replace(line, '  ', ' ')
				line = string.replace(line, '  ', ' ')
				line = string.strip(line)
				if line.startswith('v '):
					# print('found a vertex')
					vertex_list.append(parse_obj_vector(line))

				if line.startswith('vn '):
					# print('found a vertex normal')
					normal_list.append(parse_obj_vector(line))

				if line.startswith('f '):
					# print('found a face')
					face_list.append(parse_obj_face(line))

		f.close()

		print('OBJ Parser : "' + filename_in + '", ' + str(len(vertex_list)) + ' vertices, ' + str(len(normal_list)) + ' normals, ' + str(len(face_list)) + ' faces, ')

		obj_name = string.replace(filename_in, '.obj', '')
		obj_name = string.replace(obj_name, ' ', '')
		obj_name = string.replace(obj_name, '-', '_')
		obj_name = obj_name.lower()

		##  Creates the C file that lists the vertices
		filename_out = '../Assets/object_' + string.replace(filename_in, '.obj', '.h')
		f = codecs.open(filename_out, 'w')
		f.write('extern const int object_' + obj_name + '_verts[' + str(len(vertex_list) * 3) + '];\n')
		f.write('extern const int object_' + obj_name + '_faces[' + str(len(face_list) * 4) + '];\n')

		f.close()

		##  Creates the C file that lists the vertices
		filename_out = '../Assets/object_' + string.replace(filename_in, '.obj', '.c')
		f = codecs.open(filename_out, 'w')

		f.write('/* List of vertices */' + '\n')
		f.write('int const object_' + obj_name + '_verts[] =\n')
		f.write('{\n')

		##  Iterate on vertices
		for _vertex in vertex_list:
			_str_out = str(int(_vertex.x)) + ',\t' + str(int(_vertex.z)) + ',\t' + str(int(_vertex.y * -1.0)) + ','
			f.write('\t' + _str_out + '\n')

		_str_out = '};'
		f.write(_str_out + '\n')

		##  Creates the C file that lists the faces

		##  Iterate on faces
		f.write('\n')
		f.write('/* List of faces */' + '\n')

		f.write('int const object_' + obj_name + '_faces[] =\n')
		f.write('{\n')

		for _face in face_list:
			_str_out = '\t'

			corner_idx = 0
			for _corners in _face:
				_str_out += str(_corners['vertex'])
				corner_idx += 1
				_str_out += ','

			f.write(_str_out + '\n')

		_str_out = '};'
		f.write(_str_out + '\n')

		f.close()

main()