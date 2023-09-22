import glob
from os.path import splitext, basename

SEPARATOR = '''
/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */
'''

def createSingelHeader(name):
    with open(f"./src/{name}.h") as f:
        header = f.read()

    with open(f"./src/{name}.c") as f:
        source = f.readlines()

    with open(f"./LICENSE") as f:
        license = f.read()

    define = f"{name.upper()}_IMPLEMENTATION"

    with open(f"./{name}.h", 'w') as f:
        f.write(header)

        f.write(SEPARATOR)

        f.write(f"#ifdef {define}\n")

        f.writelines(source[1:])

        f.write(f"#endif /* !{define} */\n\n")

        f.write("/*\n")
        f.write(license)
        f.write("*/")

if __name__ == '__main__':
    for file in glob.glob("./src/*.h"):
        createSingelHeader(splitext(basename(file))[0])