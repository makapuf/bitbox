with open('data.bin','w') as f : 
    for i in range (16) : 
        for j in range(256) : 
            f.write(chr(j))

