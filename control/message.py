#!/usr/bin/env python3

# The MIT License (MIT)
#
# Copyright (c) 2016 Ivor Wanders
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import ctypes
from collections import namedtuple

# packet length to be used for all communication.
PACKET_SIZE = 64

#############################################################################
# Message type enum
#############################################################################
# enum-like construction of msg_type_t
msg_type_t = namedtuple("MsgType", ["nop",
                                    "config",
                                    "color"])
# can do msg_type.nop or msg_type.get_config now.
msg_type = msg_type_t(*range(0, len(msg_type_t._fields)))

# Depending on the message type, we interpret the payload as to be this field:
msg_type_field = {
  msg_type_t._fields.index("nop"): "raw",
  msg_type_t._fields.index("config"): "config",
  msg_type_t._fields.index("color"): "color",
}

# Reverse lookup for msg type, that is id->name
msg_type_name = dict(enumerate(msg_type_t._fields))

# Reverse lookup for msg type, that is name->id
msg_type_id = dict((k, v) for v, k in enumerate(msg_type_t._fields))


#############################################################################
# Mixins & structures
#############################################################################
# Convenience mixin to allow construction of struct from a byte like object.
class Readable:
    @classmethod
    def read(cls, byte_object):
        a = cls()
        ctypes.memmove(ctypes.addressof(a), bytes(byte_object),
                       min(len(byte_object), ctypes.sizeof(cls)))
        return a


# Mixin to allow conversion of a ctypes structure to and from a dictionary.
class Dictionary:
    # Implement the iterator method such that dict(...) results in the correct
    # dictionary.
    def __iter__(self):
        for k, t in self._fields_:
            if (issubclass(t, ctypes.Structure)):
                yield (k, dict(getattr(self, k)))
            else:
                yield (k, getattr(self, k))

    # Implement the reverse method, with some special handling for dict's and
    # lists.
    def from_dict(self, dict_object):
        for k, t in self._fields_:
            set_value = dict_object[k]
            if (isinstance(set_value, dict)):
                v = t()
                v.from_dict(set_value)
                setattr(self, k, v)
            elif (isinstance(set_value, list)):
                v = getattr(self, k)
                for j in range(0, len(set_value)):
                    v[j] = set_value[j]
                setattr(self, k, v)
            else:
                setattr(self, k, set_value)

    def __str__(self):
        return str(dict(self))


#############################################################################
# Structs for the various parts in the firmware. These correspond to
# the structures as defined in the header files.
class Config(ctypes.LittleEndianStructure, Dictionary):
    _pack_ = 1
    _fields_ = [("decay_time_delay_ms", ctypes.c_uint32),
                ("decay_interval_us", ctypes.c_uint32),
                ("decay_amount", ctypes.c_uint32),
               ]

class RGB(ctypes.LittleEndianStructure, Dictionary):
    _pack_ = 1
    _fields_ = [("R", ctypes.c_uint8),
                ("G", ctypes.c_uint8),
                ("B", ctypes.c_uint8)]

class ColorData(ctypes.LittleEndianStructure, Dictionary):
    _pack_ = 1
    _fields_ = [
               ("offset", ctypes.c_uint16),
               ("settings", ctypes.c_uint8),
               ("color", RGB * 19)
                ]


# create the composite message.
class _MsgBody(ctypes.Union):
    _fields_ = [("config", Config),
                ("color", ColorData),
                ("raw", ctypes.c_byte * (60))]

#############################################################################


# Class which represents all messages. That is; it holds all the structs.
class Msg(ctypes.LittleEndianStructure, Readable):
    type = msg_type
    _pack_ = 1
    _fields_ = [("msg_type", ctypes.c_uint8),
                ("pad", ctypes.c_uint8 * 3),
                ("_body", _MsgBody)]
    _anonymous_ = ["_body"]

    # Pretty print the message according to its type.
    def __str__(self):
        if (self.msg_type in msg_type_field):
            payload_text = str(getattr(self, msg_type_field[self.msg_type]))
            message_field = msg_type_name[self.msg_type]
        else:
            message_field = msg_type_name[self.msg_type]
            payload_text = "-"
        return "<Msg {}: {}>".format(message_field, payload_text)

    # We have to treat the mixin slightly different here, since we there is
    # special handling for the message type and thus the body.
    def __iter__(self):
        for k, t in self._fields_:
            if (k == "_body"):
                if (self.msg_type in msg_type_field):
                    message_field = msg_type_field[self.msg_type]
                    body = dict(getattr(self, msg_type_field[self.msg_type]))
                else:
                    message_field = "raw"
                    body = [a for a in getattr(self, message_field)]
                yield (message_field, body)
            elif (issubclass(t, ctypes.Structure)):
                yield (k, dict(getattr(self, k)))
            else:
                yield (k, getattr(self, k))

    def from_dict(self, dict_object):
        # Walk through the dictionary, as we do not know which elements from
        # the struct we would need.
        for k, set_value in dict_object.items():
            if (isinstance(set_value, dict)):
                v = getattr(self, k)
                v.from_dict(set_value)
                setattr(self, k, v)
            elif (isinstance(set_value, list)):
                v = getattr(self, k)
                for j in range(0, len(set_value)):
                    v[j] = set_value[j]
                setattr(self, k, v)
            else:
                setattr(self, k, set_value)

if __name__ == "__main__":
    print("Msg: {}".format(ctypes.sizeof(Msg)))
    print("Config: {}".format(ctypes.sizeof(Config)))
    print("RGB: {}".format(ctypes.sizeof(RGB)))
