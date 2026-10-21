# generated from rosidl_generator_py/resource/_idl.py.em
# with input from robot_msgs:msg/ChassisState.idl
# generated code does not contain a copyright notice


# Import statements for member types

import builtins  # noqa: E402, I100

import math  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_ChassisState(type):
    """Metaclass of message 'ChassisState'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('robot_msgs')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'robot_msgs.msg.ChassisState')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__chassis_state
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__chassis_state
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__chassis_state
            cls._TYPE_SUPPORT = module.type_support_msg__msg__chassis_state
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__chassis_state

            from std_msgs.msg import Header
            if Header.__class__._TYPE_SUPPORT is None:
                Header.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class ChassisState(metaclass=Metaclass_ChassisState):
    """Message class 'ChassisState'."""

    __slots__ = [
        '_header',
        '_odom_x',
        '_odom_y',
        '_odom_theta',
        '_vel_linear',
        '_vel_angular',
        '_state',
        '_ultrasonic_front',
        '_ultrasonic_rear',
    ]

    _fields_and_field_types = {
        'header': 'std_msgs/Header',
        'odom_x': 'float',
        'odom_y': 'float',
        'odom_theta': 'float',
        'vel_linear': 'float',
        'vel_angular': 'float',
        'state': 'uint8',
        'ultrasonic_front': 'uint8',
        'ultrasonic_rear': 'uint8',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['std_msgs', 'msg'], 'Header'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('float'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from std_msgs.msg import Header
        self.header = kwargs.get('header', Header())
        self.odom_x = kwargs.get('odom_x', float())
        self.odom_y = kwargs.get('odom_y', float())
        self.odom_theta = kwargs.get('odom_theta', float())
        self.vel_linear = kwargs.get('vel_linear', float())
        self.vel_angular = kwargs.get('vel_angular', float())
        self.state = kwargs.get('state', int())
        self.ultrasonic_front = kwargs.get('ultrasonic_front', int())
        self.ultrasonic_rear = kwargs.get('ultrasonic_rear', int())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.header != other.header:
            return False
        if self.odom_x != other.odom_x:
            return False
        if self.odom_y != other.odom_y:
            return False
        if self.odom_theta != other.odom_theta:
            return False
        if self.vel_linear != other.vel_linear:
            return False
        if self.vel_angular != other.vel_angular:
            return False
        if self.state != other.state:
            return False
        if self.ultrasonic_front != other.ultrasonic_front:
            return False
        if self.ultrasonic_rear != other.ultrasonic_rear:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def header(self):
        """Message field 'header'."""
        return self._header

    @header.setter
    def header(self, value):
        if __debug__:
            from std_msgs.msg import Header
            assert \
                isinstance(value, Header), \
                "The 'header' field must be a sub message of type 'Header'"
        self._header = value

    @builtins.property
    def odom_x(self):
        """Message field 'odom_x'."""
        return self._odom_x

    @odom_x.setter
    def odom_x(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'odom_x' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'odom_x' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._odom_x = value

    @builtins.property
    def odom_y(self):
        """Message field 'odom_y'."""
        return self._odom_y

    @odom_y.setter
    def odom_y(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'odom_y' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'odom_y' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._odom_y = value

    @builtins.property
    def odom_theta(self):
        """Message field 'odom_theta'."""
        return self._odom_theta

    @odom_theta.setter
    def odom_theta(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'odom_theta' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'odom_theta' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._odom_theta = value

    @builtins.property
    def vel_linear(self):
        """Message field 'vel_linear'."""
        return self._vel_linear

    @vel_linear.setter
    def vel_linear(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'vel_linear' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'vel_linear' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._vel_linear = value

    @builtins.property
    def vel_angular(self):
        """Message field 'vel_angular'."""
        return self._vel_angular

    @vel_angular.setter
    def vel_angular(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'vel_angular' field must be of type 'float'"
            assert not (value < -3.402823466e+38 or value > 3.402823466e+38) or math.isinf(value), \
                "The 'vel_angular' field must be a float in [-3.402823466e+38, 3.402823466e+38]"
        self._vel_angular = value

    @builtins.property
    def state(self):
        """Message field 'state'."""
        return self._state

    @state.setter
    def state(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'state' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'state' field must be an unsigned integer in [0, 255]"
        self._state = value

    @builtins.property
    def ultrasonic_front(self):
        """Message field 'ultrasonic_front'."""
        return self._ultrasonic_front

    @ultrasonic_front.setter
    def ultrasonic_front(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'ultrasonic_front' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'ultrasonic_front' field must be an unsigned integer in [0, 255]"
        self._ultrasonic_front = value

    @builtins.property
    def ultrasonic_rear(self):
        """Message field 'ultrasonic_rear'."""
        return self._ultrasonic_rear

    @ultrasonic_rear.setter
    def ultrasonic_rear(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'ultrasonic_rear' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'ultrasonic_rear' field must be an unsigned integer in [0, 255]"
        self._ultrasonic_rear = value
