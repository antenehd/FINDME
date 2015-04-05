
First configure "servid.conf" and "address.conf" files.


File "serv.conf" should be configured only once when the master server starts for the first time after that it should not be changed since it contains the maximum server id given by this master server. At first it should be configure with the first server id.After this the program will increment this value and save the incremented value whenever it provide new server id.
To configure the first server id, save the following in "serv.conf" file:

servid=firstservid

where firstservid is replaced by the real first server id and it should always be 5 characters long,if first server id is not 5 character long preceed it with zeros to make it 5 characters long.




File "address.conf" contains the local ip address and port number this master server listen for incoming messages. if this is not configured this values should be give in terminal as argument when the program is started.To configure the address value save the following in "address.conf" file:

port=portnumber	
ipv4=ipv4address	
ipv6=ipv6address

where portnumber,ipv4address and ipv6address is replaced with the real value.





To build: where '>' is the command line prompt

	>make


To run this program:

	>./mserver


To clean:

	>make clean

