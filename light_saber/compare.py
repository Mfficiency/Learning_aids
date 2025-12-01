strip = neopixel.create(DigitalPin.P0, 53, NeoPixelMode.RGBW)

BRIGHTNESS = 10
def apply_brightness(r, g, b, w=0):
    r = r * BRIGHTNESS // 255
    g = g * BRIGHTNESS // 255
    b = b * BRIGHTNESS // 255
    return (r << 16) | (g << 8) | b

def pulse_blue():
    for i in range(0, 256, 5):
        color = apply_brightness(0, 0, i)
        strip.show_color(color)
        basic.pause(30)
    for i in range(255, -1, -5):
        color = apply_brightness(0, 0, i)
        strip.show_color(color)
        basic.pause(30)

def show_white():
    # RGBW strip, so set W channel to brightness
    r = 255
    g = 255
    b = 255
    w = 255
    strip.show_color(apply_brightness(r, g, b, w))

def on_button_a():
    pulse_blue()

def on_button_b():
    show_white()

input.on_button_pressed(Button.A, on_button_a)
input.on_button_pressed(Button.B, on_button_b)
