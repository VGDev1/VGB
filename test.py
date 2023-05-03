import pyduinocli

arduino = pyduinocli.Arduino("./arduino-cli")
print(arduino.version())


