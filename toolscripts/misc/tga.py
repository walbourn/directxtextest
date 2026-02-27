# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Test script for hand-building specific TGA headers

import struct
import os
from struct import *

bIDLength = pack('B', 0)
bColorMapType = pack('B', 0)
bImageType = pack('B', 2)
wColorMapFirst = pack('H', 0)
wColorMapLength = pack('H', 0)
bColorMapSize = pack('B', 0)
wXOrigin = pack('H', 0)
wYOrigin = pack('H', 0)
wWidth = pack('H', 0)
wHeight = pack('H', 0)
bBitsPerPixel = pack('B', 24)
bDescriptor = pack('B', 0)

header = bIDLength+bColorMapType+bImageType+wColorMapFirst+wColorMapLength+bColorMapSize+wXOrigin+wYOrigin+wWidth+wHeight+bBitsPerPixel+bDescriptor;

filename = "D:/Temp/poc.tga"

with open(filename, "wb") as f:
    f.write(header)
