#################################################################
# python Third Space Vest controller object
# By Kyle Machulis <kyle@nonpolynomial.com>
# http://www.nonpolynomial.com
#
# Distributed as part of the libthirdspacevest project
#
# Repo: http://www.github.com/qdot/libthirdspacevest
#
# Licensed under the BSD License, as follows
#
# Copyright (c) 2010, Kyle Machulis/Nonpolynomial Labs
# All rights reserved.
#
# Redistribution and use in source and binary forms,
# with or without modification, are permitted provided
# that the following conditions are met:
#
#    * Redistributions of source code must retain the
#      above copyright notice, this list of conditions
#      and the following disclaimer.
#    * Redistributions in binary form must reproduce the
#      above copyright notice, this list of conditions and
#      the following disclaimer in the documentation and/or
#      other materials provided with the distribution.
#    * Neither the name of the Nonpolynomial Labs nor the names
#      of its contributors may be used to endorse or promote
#      products derived from this software without specific
#      prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#################################################################

# Requires PyUSB >= 1.0a1 (Or really, any of them that support the
# new API.)
#
# http://pyusb.berlios.de/
import usb

import sys
import time
import random
from ctypes import c_ubyte, c_uint32

class ThirdSpaceVest:
    """
    Class for controlling the TN Games Third Space Vest.
    """

    # The 16 bytes of the CRC8 table contains in tngaming.lib, used
    # for checksumming.
    CRC8_TABLE = [0x00, 0x07, 0x0E, 0x09,
                  0x1C, 0x1B, 0x12, 0x15,
                  0x38, 0x3F, 0x36, 0x31,
                  0x24, 0x23, 0x2A, 0x2D]

    # The cache key creation table. Not explicitly needed since we can
    # use the same cache key over and over, but whatever.
    CACHE_KEY_TABLE= [0x55, 0x02, 0x15, 0x2E, 0x41, 0x3D, 0x0B, 0x6D,
                      0x17, 0x02, 0x5F, 0x24, 0x12, 0x3E, 0x6F, 0x5F,
                      0x2E, 0x1C, 0x57, 0x6B, 0x27, 0x08, 0x71, 0x52,
                      0x7A, 0x2E, 0x5B, 0x62, 0x62, 0x7B, 0x70, 0x26,
                      0x5B, 0x19, 0x4C, 0x6B, 0x21, 0x76, 0x4C, 0x3C,
                      0x31, 0x3E, 0x0A, 0x64, 0x46, 0x5B, 0x64, 0x72,
                      0x5C, 0x7B, 0x75, 0x2F, 0x2F, 0x09, 0x1A, 0x29,
                      0x3C, 0x31, 0x6E, 0x2B, 0x3E, 0x60, 0x4D, 0x41,
                      0x31, 0x41, 0x53, 0x37, 0x51, 0x40, 0x5C, 0x1A,
                      0x1B, 0x09, 0x05, 0x35, 0x49, 0x09, 0x29, 0x14,
                      0x5A, 0x6A, 0x64, 0x03, 0x07, 0x1A, 0x13, 0x0F,
                      0x4E, 0x45, 0x51, 0x03, 0x69, 0x55, 0x7A, 0x6B,
                      0x56, 0x78, 0x2A, 0x14, 0x51, 0x19, 0x3D, 0x08,
                      0x54, 0x64, 0x50, 0x15, 0x1D, 0x46, 0x3E, 0x46,
                      0x27, 0x6A, 0x23, 0x68, 0x2F, 0x3C, 0x5C, 0x05,
                      0x2F, 0x68, 0x03, 0x6B, 0x65, 0x5B, 0x76, 0x26,
                      0x4C, 0x3F, 0x51, 0x00, 0x21, 0x03, 0x6E, 0x07,
                      0x5E, 0x50, 0x6B, 0x06, 0x41, 0x13, 0x23, 0x09,
                      0x45, 0x79, 0x32, 0x5C, 0x27, 0x6D, 0x75, 0x0C,
                      0x61, 0x1B, 0x06, 0x64, 0x31, 0x70, 0x43, 0x70,
                      0x12, 0x17, 0x48, 0x7C, 0x41, 0x7C, 0x6F, 0x15,
                      0x38, 0x4B, 0x56, 0x06, 0x35, 0x71, 0x58, 0x5B,
                      0x33, 0x19, 0x11, 0x61, 0x6F, 0x2F, 0x5D, 0x22,
                      0x63, 0x5F, 0x59, 0x6C, 0x4D, 0x15, 0x60, 0x4A,
                      0x28, 0x7E, 0x0E, 0x09, 0x30, 0x05, 0x40, 0x33,
                      0x62, 0x57, 0x11, 0x16, 0x79, 0x5E, 0x5D, 0x3D,
                      0x71, 0x48, 0x40, 0x75, 0x06, 0x00, 0x16, 0x49,
                      0x35, 0x32, 0x7C, 0x04, 0x39, 0x4B, 0x4D, 0x35,
                      0x0E, 0x76, 0x25, 0x25, 0x70, 0x1F, 0x61, 0x62,
                      0x5C, 0x72, 0x1B, 0x37, 0x0D, 0x5B, 0x31, 0x30,
                      0x7F, 0x07, 0x3F, 0x19, 0x6E, 0x61, 0x1F, 0x7F,
                      0x57, 0x16, 0x6F, 0x2D, 0x75, 0x10, 0x0A, 0x2F,
                      0x44, 0x7D, 0x0C, 0x51, 0x00, 0x48, 0x52, 0x20,
                      0x26, 0x1D, 0x76, 0x67, 0x71, 0x69, 0x56, 0x32,
                      0x5D, 0x57, 0x0E, 0x4E, 0x26, 0x53, 0x78, 0x45,
                      0x49, 0x09, 0x32, 0x65, 0x01, 0x66, 0x17, 0x39,
                      0x4A, 0x14, 0x43, 0x0E, 0x60, 0x01, 0x13, 0x6F,
                      0x40, 0x59, 0x21, 0x27, 0x25, 0x06, 0x4B, 0x45,
                      0x0B, 0x36, 0x2C, 0x12, 0x2E, 0x54, 0x21, 0x1C,
                      0x0B, 0x0C, 0x45, 0x2E, 0x5D, 0x4B, 0x74, 0x54,
                      0x20, 0x3C, 0x4A, 0x5A, 0x10, 0x4B, 0x23, 0x4D,
                      0x2A, 0x24, 0x1C, 0x78, 0x28, 0x34, 0x10, 0x67,
                      0x09, 0x25, 0x1B, 0x66, 0x06, 0x65, 0x1A, 0x02,
                      0x1D, 0x20, 0x28, 0x06, 0x08, 0x40, 0x21, 0x7E,
                      0x45, 0x73, 0x21, 0x37, 0x10, 0x24, 0x04, 0x3B,
                      0x63, 0x7F, 0x67, 0x58]


    # Set to true to print out debug messages
    DEBUG = False

    # VID/PID for amBX Controller
    TSV_VENDOR_ID = 0x1BD7
    TSV_PRODUCT_ID = 0x5000

    #Convienence strings for device endpoints
    DEVICE_EP = { 'in'  : 0x82,
                  'out' : 0x01
                  }

    def __init__(self):
        """Sets up member variables"""

        # Device as retreived from the bus listing
        self.tsv_device = None

    def open(self, index = 0):
        """ Given an index, opens the related third space vest
        device. The index refers to the position of the device on the
        USB bus. Index 0 is default, and will open the first device
        found.

        Returns True if open successful, False otherwise.
        """

        self.tsv_device = usb.core.find(idVendor = self.TSV_VENDOR_ID,
                                        idProduct = self.TSV_PRODUCT_ID)
        if self.tsv_device is None:
            return False

        # If we're on linux, the first time around we have to detach
        # from the kernel driver since HID will pick it up. On all
        # other platforms this will most likely fail, but we just let
        # it fall through and post a message right now.
        try:
            self.tsv_device.detach_kernel_driver(0)
        except usb.core.USBError:
            print "Probably already detached, continuing"

        self.tsv_device.set_configuration(1)
        return True

    def close(self):
        """Closes the third space device currently held by the object,
        if it is open.
        """

        if self.tsv_device is not None:
            self.tsv_device = None

    def write(self, command):
        """Given a list of raw bytes, writes them to the out endpoint of the
        third space controller.

        Returns total number of bytes written.
        """

        if self.DEBUG:
            print "Sending %s" % (["0x%.2x " % (x) for x in command])

        # Make sure command is in correct format to send
        command = [chr(x) for x in command]

        # Send command, return value
        return self.tsv_device.write(self.DEVICE_EP['out'], map(ord, command), 0, 100)

    def read(self, size=64):
        """Reads in the requested amount of bytes
        from the in endpoint of the third space device.

        Returns list of bytes read.
        """
        return self.tsv_device.read(self.DEVICE_EP['in'], size, 0, 100)

    def form_checksum(self, index, speed):
        """Returns a nybble based CRC8 of the index and speed"""
        a = (self.CRC8_TABLE[index] << 4) & 0xFF
        b = (self.CRC8_TABLE[index] >> 4) ^ (speed >> 4)
        c = a ^ self.CRC8_TABLE[b]
        d = (c << 4) & 0xFF
        return self.CRC8_TABLE[((c >> 4) ^ speed) & 0x0F] ^ d

    def form_le_int32(self, l, i):
        """Given a list of bytes, assuming they're in big-endian
        order, return a little endian int"""
        return l[i+3] | l[i+2] << 8 | l[i+1] << 16 | l[i] << 24

    def form_le_int32_list(self, i):
        """Given a little-endian int, return a list of bytes in
        big-endian format"""
        return [((i >> x) & 0xFF) for x in [0, 8, 16, 24]]

    # Yanked from http://stackoverflow.com/questions/2588364/python-tea-implementation
    def tea_encipher(self, data, cache_key):
        """Given a 64-bit block and a 128-bit cache key, return a
        64-bit encrypted block, as a list of bytes"""

        v = [self.form_le_int32(data, i) for i in range(0, 8, 4)]
        k = [self.form_le_int32(cache_key, i) for i in range(0, 16, 4)]
        y=c_uint32(v[0]);
        z=c_uint32(v[1]);
        sum=c_uint32(0);
        delta=0x9E3779B9;
        n=32

        while(n>0):
            sum.value += delta
            y.value += ( z.value << 4 ) + k[0] ^ z.value + sum.value ^ ( z.value >> 5 ) + k[1]
            z.value += ( y.value << 4 ) + k[2] ^ y.value + sum.value ^ ( y.value >> 5 ) + k[3]
            n -= 1

        return self.form_le_int32_list(y.value) + self.form_le_int32_list(z.value)

    # Yanked from http://stackoverflow.com/questions/2588364/python-tea-implementation
    def tea_decipher(self, data, cache_key):
        """Given a 64-bit block and a 128-bit cache key, return 2
        32-bit ints in a list. Untested."""

        v = [self.form_le_int32(data, i) for i in range(0, 8, 4)]
        k = [self.form_le_int32(cache_key, i) for i in range(0, 16, 4)]

        y=c_uint32(v[0])
        z=c_uint32(v[1])
        sum=c_uint32(0xC6EF3720)
        delta=0x9E3779B9
        n=32
        w=[0,0]

        while(n>0):
            z.value -= ( y.value << 4 ) + k[2] ^ y.value + sum.value ^ ( y.value >> 5 ) + k[3]
            y.value -= ( z.value << 4 ) + k[0] ^ z.value + sum.value ^ ( z.value >> 5 ) + k[1]
            sum.value -= delta
            n -= 1

        w[0]=y.value
        w[1]=z.value
        return w

    def form_cipher_cache_key(self, cache_key_index = None):
        """Generate a cache key, either from an 8-bit index passed to
        the function, or a random 8-bit number otherwise"""

        if cache_key_index is None:
            index = random.randint(0, 255)
        else:
            index = cache_key_index
        key_offset = [3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12]
        return [index, [self.CACHE_KEY_TABLE[x + index] for x in key_offset]]

    def form_packet(self, index, speed):
        """Form the packet to send to the device, in unencrypted
        format. Calculates the checksum for the index/speed pair."""

        packet = [0x0,
                  0x0,
                  0x0,
                  0x0,
                  speed,
                  index,
                  self.form_checksum(index, speed),
                  0x0]
        return packet

    def encrypt_packet(self, packet, cache_key_index = None):
        """Given a cache key index and an unencrypted packet, generate
        a cache key or use the one in the parameters, then form the
        HID report needed to talk to the vest, with cache key index
        and encrypted data."""

        cache_key = self.form_cipher_cache_key(cache_key_index)
        encrypted_packet = [0x02, cache_key[0]] + self.tea_encipher(packet, cache_key[1])
        return encrypted_packet

    def send_actuator_command(self, index, speed, cache_key_index = None):
        """Given a cell index and a speed, send an event to the
        vest"""
        # Create our 64-bit packet, and encrypt it
        packet = self.encrypt_packet(self.form_packet(index, speed), cache_key_index)

        # Send it to the device
        self.write(packet)

def main(argv=None):
    # Create the vest device
    tsv_device = ThirdSpaceVest()

    # Open the device
    if tsv_device.open() is False:
        print "No third_space device connected"
        return

    # Always print out what we're sending
    tsv_device.DEBUG = True

    # Until someone hits Control-C
    while 1:
        try:
            # Rotate through each cell, turning it on then off
            # quickly.
            for i in range(0, 8):
                tsv_device.send_actuator_command(i, 10, 0x5D)
                print tsv_device.read()
                time.sleep(.1)
                tsv_device.send_actuator_command(i, 0x0, 0x5D)
                print tsv_device.read()
                time.sleep(.1)
        except KeyboardInterrupt, e:
            # On keyboard interrupt, turn all cells off. Cells staying
            # on HURTS. PHYSICALLY HURTS.
            for i in range(0, 8):
                tsv_device.send_actuator_command(i, 0x0, 0x5D)
                print tsv_device.read()
                time.sleep(.05)
            break

    # Close out the device
    tsv_device.close()

if __name__ == "__main__":
    sys.exit(main())

