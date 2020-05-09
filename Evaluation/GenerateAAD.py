import base64
import string
import random

data_list = []
for i in range(100):
	data = ''.join(random.sample(string.ascii_letters + string.digits, 32))
	data_list.append(data)
	data = data.encode()
	
	encoded = base64.b64encode(data)
	print (encoded.decode())

for i in data_list:
	print(i)