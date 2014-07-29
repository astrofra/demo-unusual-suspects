import os
import string
import codecs
import ast

frequency_in = 50.0
folder_in = 'sync_files_in'
filename_in_root = 'audio_sync_tr_'
folder_out = '../../Assets/'
filename_out_root = 'audio_sync'
tracks_array = []

def main():
	record_count = -1

	##	Input
	#########
	for audio_channel in range(0,4):
		print("Parsing channel #" + str(audio_channel)) 

		##	File read
		filename_in = os.path.join(folder_in, filename_in_root + str(audio_channel) + '.txt')
		f = codecs.open(filename_in, 'r')

		value_array = []
		max_value = 0.0
		value_sum = 0.0

		for line in f:
			float_value = float(line)
			value_array.append(float_value)
			max_value = max(float_value, max_value)
			value_sum += float_value

		f.close()

		##	Processing
		value_avg = value_sum / len(value_array)
		value_median = sorted(value_array)[len(value_array) / 2]

		print('max_value = ' + str(max_value) + ', value_avg = ' + str(value_avg) + ', value_median = ' + str(value_median) + ', value_count = ' + str(len(value_array)))

		normalize_env_value = value_avg ##(value_avg + max_value) / 2.0

		for i in range(0, len(value_array)):
			value_array[i] = min(value_array[i] / normalize_env_value, 1.0)
			# value_array[i] = pow(value_array[i], 4.0)
			# value_array[i] = min(value_array[i] * normalize_env_value, 1.0)
			# value_array[i] = float(int(value_array[i] * 1000)) / 1000.0 
		# 	value_array[i] = max(value_array[i], 0.0)
			value_array[i] = int(value_array[i] * 3.0)

		tracks_array.append(value_array)

		if (record_count < 0):
			record_count = len(value_array)
		else:
			record_count = min(len(value_array), record_count)

	##	Output
	##########

	##	.c File write
	filename_out = os.path.join(folder_out, filename_out_root + '.c')
	f = codecs.open(filename_out, 'w')
	f.write('/*' + '\n')
	f.write('\tAudio synch data' + '\n')
	f.write('\t4 channels encoded on 2 bits each : CH0 || CH1 << 2 || CH2 << 4 || CH3 << 6' + '\n')
	f.write('\tFrequency : ' + str(frequency_in) + 'Hz' + '\n')
	f.write('\tSamples   : ' + str(record_count) + '\n')
	duration_sec = float(record_count / frequency_in)
	duration_min = int(duration_sec / 60)
	duration_str = str(duration_min) + 'min' + ' ' +  str(int(duration_sec - (duration_min * 60.0))) + 'sec'
	f.write('\tDuration  : ' + duration_str + '\n')
	f.write('*/' + '\n')
	f.write('\n')
	f.write('#include <exec/types.h>\n')
	f.write('\n')
	f.write('UBYTE const audio_sync[' + str(record_count) + '] =\n')
	f.write('{\n')

	print('record_count = ' + str(record_count))
	val_count = 0
	out_str = '\t'
	for line in range(0, record_count):
		int_val = 0
		for audio_channel in range(0,4):
			int_val += int(tracks_array[audio_channel][line]) << (2 * audio_channel)

		out_str += (str(int_val) + ', ')

		val_count += 1
		if val_count >= 25:
			 out_str += '\n'
			 out_str += '\t'
			 val_count = 0

	f.write(out_str + '\n')

	f.write('};\n')

	f.close()

	##	.h File write
	filename_out = os.path.join(folder_out, filename_out_root + '.h')
	f = codecs.open(filename_out, 'w')

	f.write('/*' + '\n')
	f.write('\tAudio synch data headers' + '\n')
	f.write('*/' + '\n')
	f.write('\n')
	f.write('#include <exec/types.h>\n')	
	f.write('\n')
	f.write('#define AUDIO_SYNC_REC_COUNT\t' + str(record_count) + '\n')
	f.write('#define AUDIO_SYNC_FREQ\t\t\t' + str(int(frequency_in)) + '\n')
	f.write('\n')
	f.write('extern UBYTE const audio_sync[' + str(record_count) + '];\n')

	f.close()

main()