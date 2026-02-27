# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Test script for hand-building specific DDS headers

import struct
import os
from struct import *

size = pack('I', 124) # 124
flags = pack('I', 0x1007) # 0x2=height, 0x800000=depth, texture=0x1007 (CAPS|HEIGHT|WIDTH|PIXELFORMAT)
height = pack('I', 4)
width = pack('I', 256)
pitchOrLinearSize = pack('I', 1024)
depth = pack('I', 0)
mipMapCount = pack('I', 0)
reserved1 = pack('I', 0) * 11

ddpfSize = pack('I', 32) #32
ddpfFlags = pack('I', 0x4) # FOURCC=0x4
ddpfFourCC = pack('I', 0x31545844) # DX10=0x30315844, DXT1=0x31545844
ddpfRGBBitCount = pack('I', 0)
ddpfRBitMask = pack('I', 0)
ddpfGBitMask = pack('I', 0)
ddpfBBitMask = pack('I', 0)
ddpfABitMask = pack('I', 0)

caps = pack('I', 0)
caps2 = pack('I', 0) # 0x200=cubemap, cubemapAllFaces=0xFE00
caps3 = pack('I', 0)
caps4 = pack('I', 0)
reserved2 = pack('I', 0)

header = (size + flags + height + width + pitchOrLinearSize + depth + mipMapCount + reserved1 +
           ddpfSize + ddpfFlags + ddpfFourCC + ddpfRGBBitCount + ddpfRBitMask + ddpfGBitMask + ddpfBBitMask + ddpfABitMask +
           caps + caps2 + caps3 + caps4 + reserved2)

dxgiFormat = pack('I', 28) # DXGI_FORMAT_R8G8B8A8_UNORM=28
dxgiDimension = pack('I', 3) # DXGI_DIMENSION_TEXTURE2D=3
dxgiMiscFlag = pack('I', 0)
dxgiArraySize = pack('I', 1)
dxgiMiscFlag2 = pack('I', 0)

#header10 = (dxgiFormat + dxgiDimension + dxgiMiscFlag + dxgiArraySize + dxgiMiscFlag2)

filename = "D:/Temp/poc.dds"

with open(filename, "wb") as f:
    f.write(b"DDS ")
    f.write(header)
#    f.write(header10)
