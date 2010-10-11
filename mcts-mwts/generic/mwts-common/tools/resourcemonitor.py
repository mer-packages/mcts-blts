import socket

HOST = ''
PORT = 12345
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
while 1:
	print "Waiting for connection"
	s.listen(1)
	conn, addr = s.accept()
	print 'Connected by ' , addr
	while 1:
		data = conn.recv(1024)
		if not data:
			break

		print data
			
	conn.close()
	print "Connection closed"



