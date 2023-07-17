def validate():
    f = open("serverlog.txt", "r")
    lines = f.readlines()
    requests = []
    grants = []
    releases = []
    for index,line in enumerate(lines):
        if(index == 0): continue
        print(index)
        if ("REQUEST" in line):
            requests.append(int(line.split("-")[1]))
            continue
        if ("GRANTED" in line):
            if (len(grants) != len(releases)):
                print(line)
                print(len(grants))
                print(len(releases))
                raise Exception("Invalid log file: invalid grants and releases sequence")
            grants.append(int(line.split("-")[1]))
            continue
        if ("RELEASED" in line):
            if (len(releases) != len(grants) - 1):
                raise Exception("Invalid log file: invalid grants and releases sequence")
            releases.append(int(line.split("-")[1]))
            continue
        print(requests,grants,releases)
    for i in range(len(requests)):
        if (requests[i] != grants[i] or grants[i] != releases[i]):
            raise Exception("Invalid log file: invalid grants and releases sequence")

    print("Log file was successfully validated")

if __name__ == "__main__":
    validate()