import sys
import svgwrite

pixels_per_grid = 1

materials = {}
materials['metal1'] = 'blue'
materials['metal2'] = 'purple'
materials['default'] = 'black'

infile = open(sys.argv[1])
outfile = sys.argv[1].rsplit('.', 1)[0] + ".svg"

dwg = svgwrite.Drawing(outfile, size = ("1000px", "1000px"), viewBox=('0 0 1000 1000'))

current_material = 'black'

def rect(x1, y1, x2, y2, stroke_width="0", fill_opacity=0.5, stroke="black"):
    x1 *= pixels_per_grid
    y1 *= pixels_per_grid
    x2 *= pixels_per_grid
    y2 *= pixels_per_grid
    dwg.add(dwg.rect(insert = (x1, y1),
                       size = (x2, y2),
                       stroke_width = stroke_width,
                       stroke = stroke,
                       fill_opacity = fill_opacity,
                       fill = current_material))

for line in infile:
    if line.startswith("<<"):
        _, mat, _ = line.strip().split()
        if mat in materials:
            current_material = materials[mat]
        else:
            current_material = materials['default']
        continue
    if line.startswith("rect"):
        _, x1, y1, x2, y2 = line.strip().split()
        x1 = int(x1)
        x2 = int(x2)
        y1 = int(y1)
        y2 = int(y2)
        print "rect: %d %d %d %d" % (x1, y1, x2, y2)
        rect(x1, y2, x2-x1, y2-y1)
        continue
    if line.startswith("transform"):
        _, t1, t2, cur_x, t3, t4, cur_y = line.strip().split()
        cur_x = int(cur_x)
        cur_y = int(cur_y)
        continue
    if line.startswith("box"):
        _, x1, y1, x2, y2 = line.strip().split()
        x1 = int(x1)
        x2 = int(x2)
        y1 = int(y1)
        y2 = int(y2)
        rect(cur_x+x1, cur_y+y2, x2-x1, y2-y1, stroke_width="1px", fill_opacity=0)
        continue

"""
dwg.add(dwg.rect(insert = (-50, -50),
                   size = (100, 100),
                   stroke_width = "1",
                   stroke = "black",
                   fill = 'green'))
"""
dwg.save()
