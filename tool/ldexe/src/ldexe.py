import sys
import os

if os.path.isfile("./ldexe"):
    exename = "./ldexe"
else:
    print("ERROR: cannot locate the 'ldexe' executable file in the current directory.")
    exit()

if len(sys.argv) == 1 or "--help" in sys.argv:
    os.system(exename + " --help")
    print("--root-dir=<directory>")
    print("        It is a root directory of a hierarchy of binary files to be loaded.")
    print("        This option is converted to several --binary options such that for")
    print("        each file <directory>[/<path>]/<name> inside the hierarchy there")
    print("        is generated an option --binary=<directory>[/<path>]/<name>. Note")
    print("        that this option cannot be combined with --dump option.")
    exit()

if "--root-dir=" in " ".join(sys.argv) and "--dump=" in " ".join(sys.argv):
    print("ERROR: Options --root-dir and --dump cannot be passed together.")
    exit()

root_dir = None
for opt in sys.argv:
    if opt.startswith("--root-dir="):
        root_dir = opt[len("--root-dir="):]
        break

if root_dir is None:
    os.system(exename + " " + (" ".join(sys.argv[1:])))
    exit()

if not os.path.exists(root_dir):
    print("The path passed to --root-dir does not exist.")
    exit()

if not os.path.isdir(root_dir):
    print("The path passed to --root-dir does not represent a directory.")
    exit()

pathnames = []
for root, dirnames, filenames in os.walk(root_dir):
    for filename in filenames:
        pathname = os.path.join(root, filename)
        pathnames.append("--binary=\"" + pathname + "\"")

if len(pathnames) == 0:
    print("There is no file under the directory passed to --root-dir.")
    exit()

os.system(exename + " " + (" ".join(pathnames)))

