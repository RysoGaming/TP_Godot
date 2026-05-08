extends Node

var udp := PacketPeerUDP.new()
var sequence: int = 0

func _ready():
	udp.connect_to_host("127.0.0.1", 4242)
	udp.put_packet("hello".to_utf8_buffer())
	print("hello envoyé au serveur")

func _process(delta):
	var buffer = PackedByteArray()

	buffer.append(1)

	buffer.append(sequence & 0xFF)
	buffer.append((sequence >> 8) & 0xFF)
	buffer.append((sequence >> 16) & 0xFF)
	buffer.append((sequence >> 24) & 0xFF)

	var time = Time.get_ticks_msec()
	buffer.append(time & 0xFF)
	buffer.append((time >> 8) & 0xFF)
	buffer.append((time >> 16) & 0xFF)
	buffer.append((time >> 24) & 0xFF)

	var input_mask = 0
	if Input.is_action_pressed("ui_right"):
		input_mask |= 0x08
	if Input.is_action_pressed("ui_left"):
		input_mask |= 0x04
	if Input.is_action_pressed("ui_up"):
		input_mask |= 0x01
	if Input.is_action_pressed("ui_down"):
		input_mask |= 0x02

	buffer.append(input_mask & 0xFF)
	buffer.append((input_mask >> 8) & 0xFF)

	buffer.append(0)
	buffer.append(0)

	buffer.append(0)
	buffer.append(0)

	udp.put_packet(buffer)
	print("Packet envoyé taille :", buffer.size())
	sequence += 1

	while udp.get_available_packet_count() > 0:
		var packet = udp.get_packet()

		print("Packet reçu taille :", packet.size())

		if packet.size() < 12:
			continue

		var packet_type = packet.decode_u32(0)

		if packet_type == 1:
			var network_id = packet.decode_u32(8)
			spawn_entity(network_id)

func spawn_entity(id):
	if has_node("Entity_" + str(id)):
		return

	var node = Sprite2D.new()
	node.texture = load("res://icon.svg")
	node.name = "Entity_" + str(id)
	node.position = Vector2((id % 5) * 100, (id / 5) * 100)
	add_child(node)

	print("Node spawn dans la scène :", node.name)
