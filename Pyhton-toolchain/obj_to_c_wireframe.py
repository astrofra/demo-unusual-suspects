import os
import string
import codecs
import ast
from vector3 import Vector3

filename_in = "amiga.obj"
filename_out = ""
scale_factor = 2500.0

face_list = []
vertex_list = []
normal_list = []

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
	_face.append(_face[0])

	return _face

def main():
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

	print('Parser stats : ' + str(len(vertex_list)) + ' vertices, ' + str(len(normal_list)) + ' normals, ' + str(len(face_list)) + ' faces, ')

	##  Creates the C file that lists the vertices
	filename_out = '../Assets/object_' + string.replace(filename_in, '.obj', '.c')
	f = codecs.open(filename_out, 'w')

	##  Iterate on vertices
	f.write('/* List of vertices */' + '\n')
	_str_out = '\tdc.w\t' + str(len(vertex_list) - 1)
	f.write(_str_out + '\n')

	for _vertex in vertex_list:
		_str_out = '\tdc.w\t'
		_str_out = _str_out + str(int(_vertex.x)) + ',' + str(int(_vertex.y)) + ',' + str(int(_vertex.z))
		f.write(_str_out + '\n')

	##  Creates the C file that lists the faces

	##  Iterate on faces
	f.write('/* List of faces */' + '\n')
	_str_out = '\tdc.w\t' + str(len(face_list) - 1)
	f.write(_str_out + '\n')

	for _face in face_list:
		_str_out = '\tdc.w\t'

		corner_idx = 0
		for _corners in _face:
			_str_out += str(_corners['vertex'])
			corner_idx += 1
			if corner_idx < 5:
				_str_out += ','
		# _str_out += str(_face[0]['vertex'])

		f.write(_str_out + '\n')

	f.close()

main()