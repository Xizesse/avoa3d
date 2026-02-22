########### Timestamp, frame_id definition - Relative to the working frame
std_msgs/Header header

############ Number of elements in the scene - 0...N
int32 id 

########## Defines the class of each element - 0...M
int32 type

########## Define if the object is static (False) or dynamic (True)
bool dynamic

################################### Current pose of the observed object
geometry_msgs/Pose pose

############ Position uncertainty (standard deviation for x, y, z)
float32 position_x_std
float32 position_y_std
float32 position_z_std

################################### Velocity (Vx, Vy, Vz) in the observer base_link
geometry_msgs/Vector3 velocity

############ Velocity uncertainty (standard deviation for vx, vy, vz)
float32 velocity_x_std
float32 velocity_y_std
float32 velocity_z_std

################################### Geometric dimensions of the element observed
geometry_msgs/Vector3 size

############ Radius uncertainty (standard deviation)
float32 radius_std

# Avoid zone around obstacles
float32 protective_zone