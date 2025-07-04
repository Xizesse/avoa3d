#Retirado do repositorio AVOA do Daniel

# Timestamp, frame_id definition - Relative to the working frame
std_msgs/Header header

# Number of elements in the scene - 0...N
int32 id 

# Defines the class of each element - 0...M
int32 type

# Define if the object is static (False) or dynamic (True)
bool dynamic

# Current pose of the observed object
geometry_msgs/Pose pose

# Velocity (Vx, Vy, Vz) in the observer base_link
geometry_msgs/Vector3 velocity

# Geometric dimensions of the element observed
geometry_msgs/Vector3 size

# Avoid zone around obstacles
float32 protective_zone
