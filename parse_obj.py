with open("teapot.obj", "r") as f:
    verticies = []
    faces = []
    for line in f.readlines():
        if line.startswith('v '):
            line = line.split()[1:]
            
            #this isnt technically needed for the x and y but it removes trailing zeros
            line[0] = str(float(line[0]))
            line[1] = str(-float(line[1])) #flip the y coordinate, needed
            line[2] = str(float(line[2])) 
            line = '{' + 'f, '.join(line) +'f},'
            verticies.append(line)
        elif line.startswith('f'):
            face = [
                int(index_set.split('/')[0])-1 
                for index_set in line.split()[1:]
                ]
            #if it isnt already triangles we need to convert it (fan method)
            faces.append(f"{{{face[0]}, {face[1]}, {face[2]}}},")
            for i in range(3, len(face)):
                faces.append(f"{{{face[0]}, {face[i-1]}, {face[i]}}},")

with open("verticies.txt", "w") as f:
    f.writelines(verticies)

with open("faces.txt", "w") as f:
    f.writelines(faces)

print("finished")