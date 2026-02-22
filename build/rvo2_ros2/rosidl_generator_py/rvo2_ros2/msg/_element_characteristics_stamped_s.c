// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from rvo2_ros2:msg/ElementCharacteristicsStamped.idl
// generated code does not contain a copyright notice
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <stdbool.h>
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "numpy/ndarrayobject.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
#include "rosidl_runtime_c/visibility_control.h"
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__struct.h"
#include "rvo2_ros2/msg/detail/element_characteristics_stamped__functions.h"

ROSIDL_GENERATOR_C_IMPORT
bool std_msgs__msg__header__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * std_msgs__msg__header__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__pose__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__pose__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__vector3__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__vector3__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__vector3__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__vector3__convert_to_py(void * raw_ros_message);

ROSIDL_GENERATOR_C_EXPORT
bool rvo2_ros2__msg__element_characteristics_stamped__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[77];
    {
      char * class_name = NULL;
      char * module_name = NULL;
      {
        PyObject * class_attr = PyObject_GetAttrString(_pymsg, "__class__");
        if (class_attr) {
          PyObject * name_attr = PyObject_GetAttrString(class_attr, "__name__");
          if (name_attr) {
            class_name = (char *)PyUnicode_1BYTE_DATA(name_attr);
            Py_DECREF(name_attr);
          }
          PyObject * module_attr = PyObject_GetAttrString(class_attr, "__module__");
          if (module_attr) {
            module_name = (char *)PyUnicode_1BYTE_DATA(module_attr);
            Py_DECREF(module_attr);
          }
          Py_DECREF(class_attr);
        }
      }
      if (!class_name || !module_name) {
        return false;
      }
      snprintf(full_classname_dest, sizeof(full_classname_dest), "%s.%s", module_name, class_name);
    }
    assert(strncmp("rvo2_ros2.msg._element_characteristics_stamped.ElementCharacteristicsStamped", full_classname_dest, 76) == 0);
  }
  rvo2_ros2__msg__ElementCharacteristicsStamped * ros_message = _ros_message;
  {  // header
    PyObject * field = PyObject_GetAttrString(_pymsg, "header");
    if (!field) {
      return false;
    }
    if (!std_msgs__msg__header__convert_from_py(field, &ros_message->header)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // id
    PyObject * field = PyObject_GetAttrString(_pymsg, "id");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->id = (int32_t)PyLong_AsLong(field);
    Py_DECREF(field);
  }
  {  // type
    PyObject * field = PyObject_GetAttrString(_pymsg, "type");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->type = (int32_t)PyLong_AsLong(field);
    Py_DECREF(field);
  }
  {  // dynamic
    PyObject * field = PyObject_GetAttrString(_pymsg, "dynamic");
    if (!field) {
      return false;
    }
    assert(PyBool_Check(field));
    ros_message->dynamic = (Py_True == field);
    Py_DECREF(field);
  }
  {  // pose
    PyObject * field = PyObject_GetAttrString(_pymsg, "pose");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__pose__convert_from_py(field, &ros_message->pose)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // position_x_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "position_x_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->position_x_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // position_y_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "position_y_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->position_y_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // position_z_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "position_z_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->position_z_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // velocity
    PyObject * field = PyObject_GetAttrString(_pymsg, "velocity");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__vector3__convert_from_py(field, &ros_message->velocity)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // velocity_x_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "velocity_x_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->velocity_x_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // velocity_y_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "velocity_y_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->velocity_y_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // velocity_z_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "velocity_z_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->velocity_z_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // size
    PyObject * field = PyObject_GetAttrString(_pymsg, "size");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__vector3__convert_from_py(field, &ros_message->size)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // radius_std
    PyObject * field = PyObject_GetAttrString(_pymsg, "radius_std");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->radius_std = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // protective_zone
    PyObject * field = PyObject_GetAttrString(_pymsg, "protective_zone");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->protective_zone = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * rvo2_ros2__msg__element_characteristics_stamped__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of ElementCharacteristicsStamped */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("rvo2_ros2.msg._element_characteristics_stamped");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "ElementCharacteristicsStamped");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  rvo2_ros2__msg__ElementCharacteristicsStamped * ros_message = (rvo2_ros2__msg__ElementCharacteristicsStamped *)raw_ros_message;
  {  // header
    PyObject * field = NULL;
    field = std_msgs__msg__header__convert_to_py(&ros_message->header);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "header", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // id
    PyObject * field = NULL;
    field = PyLong_FromLong(ros_message->id);
    {
      int rc = PyObject_SetAttrString(_pymessage, "id", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // type
    PyObject * field = NULL;
    field = PyLong_FromLong(ros_message->type);
    {
      int rc = PyObject_SetAttrString(_pymessage, "type", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // dynamic
    PyObject * field = NULL;
    field = PyBool_FromLong(ros_message->dynamic ? 1 : 0);
    {
      int rc = PyObject_SetAttrString(_pymessage, "dynamic", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // pose
    PyObject * field = NULL;
    field = geometry_msgs__msg__pose__convert_to_py(&ros_message->pose);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "pose", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // position_x_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->position_x_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "position_x_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // position_y_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->position_y_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "position_y_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // position_z_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->position_z_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "position_z_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // velocity
    PyObject * field = NULL;
    field = geometry_msgs__msg__vector3__convert_to_py(&ros_message->velocity);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "velocity", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // velocity_x_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->velocity_x_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "velocity_x_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // velocity_y_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->velocity_y_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "velocity_y_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // velocity_z_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->velocity_z_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "velocity_z_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // size
    PyObject * field = NULL;
    field = geometry_msgs__msg__vector3__convert_to_py(&ros_message->size);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "size", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // radius_std
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->radius_std);
    {
      int rc = PyObject_SetAttrString(_pymessage, "radius_std", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // protective_zone
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->protective_zone);
    {
      int rc = PyObject_SetAttrString(_pymessage, "protective_zone", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
