extends Node3D

@export var move_speed: float = 1000.0
@export var mouse_sensitivity: float = 0.003

var camera: Camera3D
var yaw: float = 0.0
var pitch: float = -0.5
var mouse_captured: bool = false

func _ready():
	camera = $Camera3D
	camera.current = true
	camera.far = 10000.0
	camera.near = 0.1

	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
	mouse_captured = true

	# Dark background
	RenderingServer.set_default_clear_color(Color(0.02, 0.02, 0.05))

	# Sun light
	var sun = DirectionalLight3D.new()
	sun.name = "Sun"
	sun.light_color = Color(1.0, 0.95, 0.8)
	sun.light_energy = 1.2
	sun.rotation = Vector3(-0.7, 0.5, 0)
	add_child(sun)

	# Ambient
	var env = WorldEnvironment.new()
	var env_res = Environment.new()
	env_res.ambient_light_color = Color(0.4, 0.45, 0.55)
	env_res.ambient_light_energy = 0.6
	add_child(env)
	env.environment = env_res

	# Load the BSP level
	$BSP.load_level("C:/Users/aaron/Desktop/dev/xiii_extracted/Common_Game_Files", "Plage01")

	# Position camera above level center, looking down
	camera.position = Vector3(-190, 1600, 508)
	pitch = -0.5
	yaw = 0.0
	camera.rotation = Vector3(pitch, yaw, 0)

	print("[Main] Camera at ", camera.position, " rot ", camera.rotation)
	print("[Main] BSP children: ", $BSP.get_child_count())
	for child in $BSP.get_children():
		print("[Main] BSP child: ", child.name, " type=", child.get_class())
		if child is MeshInstance3D:
			var mesh = child.mesh
			if mesh:
				print("[Main]   mesh surfaces: ", mesh.get_surface_count())
				print("[Main]   mesh aabb: ", mesh.get_aabb())
			else:
				print("[Main]   mesh is null!")

func _input(event: InputEvent):
	if not camera:
		return

	if event.is_action_pressed("toggle_mouse"):
		mouse_captured = not mouse_captured
		if mouse_captured:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)

	if event.is_action_pressed("quit"):
		get_tree().quit()

	if mouse_captured and event is InputEventMouseMotion:
		yaw -= event.relative.x * mouse_sensitivity
		pitch -= event.relative.y * mouse_sensitivity
		pitch = clampf(pitch, -PI * 0.49, PI * 0.49)
		camera.rotation = Vector3(pitch, yaw, 0)

func _process(delta: float) -> void:
	if not camera:
		return

	var dir := Vector3.ZERO
	if Input.is_action_pressed("move_forward"):  dir -= camera.global_transform.basis[2]
	if Input.is_action_pressed("move_back"):      dir += camera.global_transform.basis[2]
	if Input.is_action_pressed("move_left"):      dir -= camera.global_transform.basis[0]
	if Input.is_action_pressed("move_right"):     dir += camera.global_transform.basis[0]
	if Input.is_action_pressed("move_up"):        dir += Vector3(0, 1, 0)
	if Input.is_action_pressed("move_down"):      dir -= Vector3(0, 1, 0)

	if dir.length_squared() > 0:
		dir = dir.normalized() * move_speed * delta
		camera.global_position += dir
