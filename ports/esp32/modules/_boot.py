import gc
import machine
import uos
from flashbdev import bdev

# Turn off IR LED on J10
machine.Pin(16, machine.Pin.OUT).value(0)

try:
    if bdev:
        uos.mount(bdev, '/')
except OSError:
    import inisetup
    vfs = inisetup.setup()

gc.collect()
