strip = neopixel.create(DigitalPin.P0, 53, NeoPixelMode.RGBW)
BRIGHTNESS = 10
def apply_brightness(r, g, b, w=0):
    r = r * BRIGHTNESS // 255
    g = g * BRIGHTNESS // 255
    b = b * BRIGHTNESS // 255
    w = w * BRIGHTNESS // 255
    # For RGBW: pack as 0xWWRRGGBB (white in highest byte)
    return (w << 24) | (r << 16) | (g << 8) | b
pulsing = False
def pulse_cyan():
    global pulsing
    pulsing = True
    while pulsing:
        # ramp up
        for i in range(0, 256, 5):
            if not pulsing:
                return
            color = apply_brightness(0, i, 255, 0)
            strip.show_color(color)
            basic.pause(20)
        # ramp down
        for i in range(255, -1, -5):
            if not pulsing:
                return
            color = apply_brightness(0, i, 255, 0)
            strip.show_color(color)
            basic.pause(20)
def show_white():
    # Use the dedicated white LED, not RGB
    r = 0 
    g = 0 
    b = 0 
    # Ramp up white slowly
    for w in range(0, 256, 5):
        color = apply_brightness(r, g, b, w)
        strip.show_color(color)
        basic.pause(20)
def on_button_a():
    pulse_cyan()
def on_button_b():
    show_white()
input.on_button_pressed(Button.A, on_button_a)
input.on_button_pressed(Button.B, on_button_b)