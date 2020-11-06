#!/usr/bin/env python

import sys, os

if len(sys.argv) != 2:
  print("Usage: " + sys.argv[0] + " <asm>")
  sys.exit(0)

out = open("/tmp/test.asm", "w")
out.write(".arm\n")
out.write("  " + sys.argv[1] + "\n\n")
out.close()

os.system("naken_asm -b -o /tmp/test.bin /tmp/test.asm")

code = []
fp = open("/tmp/test.bin", "rb")
while True:
  a = fp.read(1)
  if not a: break
  code.append("0x%02x" % ord(a))
fp.close()

print("  // " + sys.argv[1] + ": " + ",".join(code))
print("  generate_code(generate, " + str(len(code)) + ", " + ", ".join(code) + ");\n")

