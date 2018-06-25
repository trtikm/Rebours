import argparse
import os
import time

def parse_cmd_line():
    parser = argparse.ArgumentParser()
    parser.add_argument("-I","--input-file", type=str,
                        help="A pathname of a source .ASM file to be build.")
    parser.add_argument("-O", "--output-file", type=str,
                        help="A pathname of a resulting executable file. If not specified, the executable will be "
                             "stored into the directory '../bin' relative to the directory of the source file.")
    args = parser.parse_args()
    return args


def scriptMain():
    args = parse_cmd_line()

    if args.input_file is None:
        print("ERROR: no source .ASM file was specified.")
        return
    if not os.path.exists(args.input_file):
        print("ERROR: the source file '" + args.input_file + "' does not exists.")
        return
    if not (os.path.isfile(args.input_file) and os.access(args.input_file,os.R_OK)):
        print("ERROR: the source file '" + args.input_file + "' is not accessible.")
        return

    source_pathname = os.path.abspath(args.input_file)
    source_dir = os.path.dirname(source_pathname)

    if args.output_file is None:
        output_dir = os.path.normpath(os.path.join(source_dir,"../bin"))
        output_file_name = os.path.splitext(os.path.basename(source_pathname))[0]
        output_file_ext = ""
    else:
        output_dir = os.path.dirname(args.output_file)
        output_file_name = os.path.splitext(os.path.basename(args.output_file))[0]
        output_file_ext = os.path.splitext(os.path.basename(args.output_file))[1]

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    out_pathname = os.path.join(output_dir,output_file_name + output_file_ext)

    obj_dir = output_dir
    obj_name = output_file_name
    obj_ext = ".o"

    obj_pathname = os.path.join(obj_dir,obj_name + obj_ext)

    command = "nasm -f elf64 -O0 \"" + source_pathname + "\" -o \"" + obj_pathname + "\""
    os.system(command)

    start_time = time.time()
    while not os.path.exists(obj_pathname) and time.time() - start_time < 2.0:
        pass

    if os.path.exists(obj_pathname):
        command = "ld \"" + obj_pathname + "\" -o \"" + out_pathname + "\""
        os.system(command)
        if os.path.exists(out_pathname):
            os.remove(obj_pathname)


if __name__ == "__main__":
    scriptMain()
