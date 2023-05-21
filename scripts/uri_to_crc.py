#
# Copyright 2023 RTIO authors.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 


import crc
import time

hasher = crc.Calculator(crc.Crc32.CRC32)

 # mark uri and crc32
def mark(uri):
    global hasher, index
    print("#", hex(hasher.checksum(uri.encode())), uri, 
          "Add@", time.strftime("%Y-%m-%d %H:%M:%S %Z", time.gmtime()),)


if __name__ == '__main__':
    # mark("123456789") # 0xcbf43926 123456789 Add@ 2023-04-10 09:31:29 GMT
    # mark("/command")  # 0x56d29a39 /command Add@ 2023-04-10 09:31:29 GMT
    # mark("/command/v1")# 0x2c2209a4 /command/v1 Add@ 2023-04-10 09:31:29 GMT

    mark("/test")# 0x7c88eed8 /test Add@ 2023-04-22 01:34:55 GMT
    mark("/printer/action")# 0x44d87c69 /printer/action Add@ 2023-05-19 03:49:54 GMT
    mark("/printer/status")# 0x781495e7 /printer/status Add@ 2023-05-19 03:49:54 GM

