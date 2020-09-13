import sys
from PIL import Image

infile = sys.argv[1]

with Image.open(infile) as im:
    raw = im.tobytes()
    sys.stdout.write('#pragma once\n')
    sys.stdout.write('static unsigned char nimi_icon[] = {')
    for (i, b) in enumerate(raw):
        if i % 12 == 0:
            sys.stdout.write('\n   ')
        sys.stdout.write(f'0x{b:02x}, ')
    sys.stdout.write('\n};')
