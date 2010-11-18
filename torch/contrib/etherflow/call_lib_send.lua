require 'libethernet'

libethernet.open_socket('lo')

libethernet.send_frame('lua test 1')
libethernet.send_frame('lua test 2')
libethernet.send_frame('lua test 3')
libethernet.send_frame('lua test 4')
libethernet.send_frame('lua test 5')

libethernet.close_socket()