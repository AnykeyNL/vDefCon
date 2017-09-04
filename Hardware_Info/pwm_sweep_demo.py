from __future__ import division
import time

# vDefcon project - VMworld 2017
# Example how to set PWM to control analog meter.
#
# As the Raspberry outputs 3.3v, we need to convert the PWM signal to 5v.
# This is done using a Logic Level Shifter. We are using 8-channel 74LVC248 chips for this.
# See guthub documents on the wiring.


# Import the PCA9685 module.
import Adafruit_PCA9685

# Initialise the PCA9685 using the default address (0x40).
pwm = Adafruit_PCA9685.PCA9685()

# Alternatively specify a different address and/or bus:
#pwm = Adafruit_PCA9685.PCA9685(address=0x41, busnum=2)

# Configure min and max servo pulse lengths
servo_min = 0  # Min pulse length out of 4096
servo_max = 4095  # Max pulse length out of 4096

# Helper function to make setting a servo pulse width simpler.
def set_servo_pulse(channel, pulse):
    pulse_length = 1000000    # 1,000,000 us per second
    pulse_length //= 60       # 60 Hz
    print('{0}us per period'.format(pulse_length))
    pulse_length //= 4096     # 12 bits of resolution
    print('{0}us per bit'.format(pulse_length))
    pulse *= 1000
    pulse //= pulse_length
    pwm.set_pwm(channel, 0, pulse)

# Set frequency to 60hz, good for servos.
pwm.set_pwm_freq(60)

print('Moving servo on channel 0, press Ctrl-C to quit...')
s = 50
w = 0.05

# Sweep the PWM rate from 0% to 100% and back.

while True:
    # Move servo on channel O between extremes.
    for x in range(0,4095,s):
      pwm.set_pwm(0, 0, x)
      time.sleep(w)
    time.sleep(2)
    for x in range(4095,0,-s):
      pwm.set_pwm(0, 0, x)
      time.sleep(w)
    time.sleep(2)


