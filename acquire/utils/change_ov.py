import numpy as np
import sys

bias_file = sys.argv[1]

bias_data = np.genfromtxt(bias_file, delimiter='\t', dtype=None, names=True, encoding='utf-8')
header = '\t'.join(bias_data.dtype.names)

bias_data[int(sys.argv[2])][7] = float(sys.argv[3])

np.savetxt(bias_file, bias_data, delimiter='\t', header=header, fmt='%s')

