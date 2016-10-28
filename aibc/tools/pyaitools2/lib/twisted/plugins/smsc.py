from twisted.application.service import ServiceMaker

serviceMaker = ServiceMaker(
    "SMSC Emulator",
    "pyailib.tap.smsc",
    "A simple SMSC emulator",
    "smsc")