import glob

results = open("results.csv", "w")

for filename in glob.glob('original/R*'):
    if ".tmp" in filename: continue

    _, num_cells, num_nets = filename.split("_")

    with open(filename) as f:
        time = f.readlines()[3].strip()

    results.write("%s %s %s\n" % (num_cells, num_nets, time))

results.close()
