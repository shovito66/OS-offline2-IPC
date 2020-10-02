import time

service_room = int(input("Enter service room count : "))
cycle = int(input("Enter cycle count: "))
paybooth = int(input("Enter payment capcity count: "))
filepath = "1605066.txt"
# filepath = input("Enter your filename (i.e: 1605000.txt): ")
text_file = open("simulatorResult.txt", "w")


def get_numbers(text):
    return [int(s) for s in text.split() if s.isdigit()]


# line = "1 started taking service from 1"
# print(get_numbers(line))
# print(state(line))


in_rooms = [[] for i in range(service_room)]
out_rooms = [[] for i in range(service_room)]
paybooth = []
departLine = []
departed = []

with open(filepath) as fp:
    for line in fp:
        # print(line)
        for i in range(service_room):
            #print("In_R{}\tOut_R{}".format(i+1, i+1), end="\t")
            text_file.write("In_R{}\tOut_R{}\t".format(i+1, i+1))
        #print("\tPayBooth\tDepartLine\tDeparted")
        text_file.write("\tPayBooth\tDepartLine\tDeparted\n")

        index = get_numbers(line)

        if(len(index) > 1):
            room = index[1]-1
        cycle = index[0]

        if 'started' in line and 'serviceman' in line:
            if room != 0:
                out_rooms[room-1].remove(cycle)
            in_rooms[room].append(cycle)
        elif 'finished' in line and 'serviceman' in line:
            # None
            in_rooms[room].remove(cycle)
            out_rooms[room].append(cycle)
        elif 'started' in line and 'bill' in line:
            # None
            out_rooms[service_room-1].remove(cycle)
            paybooth.append(cycle)
        elif 'finished' in line and 'bill' in line:
            # None
            paybooth.remove(cycle)
            departLine.append(cycle)
        elif 'departed' in line:
            # None
            departLine.remove(cycle)
            departed.append(cycle)

        for i in range(service_room):
            # print("{}\t{}".format(in_rooms[i], out_rooms[i]), end="\t")
            text_file.write("{}\t\t{}\t\t".format(in_rooms[i], out_rooms[i]))
        # print("\t{}\t\t{}\t\t{}".format(paybooth, departLine, departed))
        text_file.write("\t{}\t\t\t{}\t\t\t{}\n".format(paybooth, departLine, departed))
        # print("====================================================================")
        text_file.write("====================================================================\n")

        time.sleep(0.1)
    # text_file.close()

text_file.close()
