require 'libethernet'


libethernet.open_socket('lo')

while true do
   print('received frame:',libethernet.receive_string())
end

libethernet.close_socket()
