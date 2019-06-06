import json
import random

with open("opcode_list.json", "r") as f:
    out_file = open("opcodes.txt", "w")
    data = json.load(f)

    for opcode in data.keys():
        for c in opcode:
            if c == 'X':
                regX = random.randint(0, 7)
                regX = hex(regX)[2:].upper()
                opcode = opcode.replace("X", regX)

            elif c == 'Y':
                regY = random.randint(8, 15)
                regY = hex(regY)[2:].upper()
                opcode = opcode.replace("Y", regY)
            
            elif c =='N':
                num = random.randint(0, 15)
                num = hex(num)[2:].upper()
                opcode = opcode.replace("N", num, 1)

        out_file.write(opcode);
        out_file.write("\n");
    
    out_file.close()