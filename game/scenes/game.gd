extends Node

var udp := PacketPeerUDP.new()

func _ready():
	udp.connect_to_host("127.0.0.1", 4242)

	var msg = "hello"
	udp.put_packet(msg.to_utf8_buffer())

	print("hello envoyé au serveur")
