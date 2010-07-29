import sys

if len(sys.argv) != 4:
  print "Error: Replace.py requires [FileName] [PatternToReplace] [StringToInsert]"
  sys.exit(2)

print sys.argv[0] 
print sys.argv[1]

file = sys.argv[1]
fileHandle = open(file, 'r')
data = fileHandle.read().replace(sys.argv[2], sys.argv[3])
fileHandle.close()

fileHandle = open(file, 'w')
fileHandle.write(data)
fileHandle.close()
