import sys
import svgwrite

pixels_per_grid = 10

materials = {}
materials['metal1'] = 'blue'
materials['metal2'] = 'purple'
materials['default'] = 'black'

infile = open(sys.argv[1])
outfile = sys.argv[1].rsplit('.', 1)[0] + ".svg"

texts = []

dwg = svgwrite.Drawing(outfile, size = ("1000px", "1000px"), viewBox=('-50 -50 1000 1000'))

current_material = 'black'

for i in range(100):
    dwg.add(dwg.line(start=(i*pixels_per_grid,0), end=(i*pixels_per_grid,1000), stroke='gray', stroke_width="1px"))
    dwg.add(dwg.line(start=(0,i*pixels_per_grid), end=(1000,i*pixels_per_grid), stroke='gray', stroke_width="1px"))

def rect(x, y, w, h, stroke_width="0", fill_opacity=0.5, stroke="black"):
    x *= pixels_per_grid
    y *= pixels_per_grid
    w *= pixels_per_grid
    h *= pixels_per_grid
    dwg.add(dwg.rect(insert = (x, y),
                       size = (w, h),
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
        rect(x1, y1, x2-x1, y2-y1)
        continue
    if line.startswith("transform"):
        _, t1, t2, cur_x, t3, t4, cur_y = line.strip().split()
        cur_x = int(cur_x)
        cur_y = int(cur_y)
        continue
    if line.startswith("box"):
        _, x1, y1, x2, y2 = line.strip().split()
        x1 = min(int(x1), int(x2))
        y1 = min(int(y1), int(y2))
        x2 = max(int(x1), int(x2))
        y2 = max(int(y1), int(y2))
        rect(cur_x+x1, cur_y+y1, x2-x1, y2-y1, stroke_width="5px", fill_opacity=0)
        continue
    if line.startswith("rlabel"):
        _, mat, x1, y1, x2, y2, pos, text = line.strip().split()
        x1 = min(int(x1), int(x2))
        y1 = min(int(y1), int(y2))
        x2 = max(int(x1), int(x2))
        y2 = max(int(y1), int(y2))
        x = (x2-x1)/2.0+x1
        y = (y2-y1)/2.0+y1
        x *= pixels_per_grid
        y *= pixels_per_grid
        print "text: %d %d" % (x,y)
        t = dwg.text(text, insert=(x,y), fill="white")
        t["font-size"] = 10
        t["text-anchor"] = "middle"
        t["dominant-baseline"] = "central"
        texts.append(t)
        continue

for t in texts:
    dwg.add(t)

dwg.save()
