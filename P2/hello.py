import sys


#Call this from our bash mysh>hello.py arg1 arg2
print(sys.argv);
if len(sys.argv) != 3:
    sys.exit("Not enough args")
first = str(sys.argv[1])
sec = str(sys.argv[2])

print("hello, world, " + first + " " + sec)