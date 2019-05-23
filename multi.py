#!/usr/bin/env python3
import os
import os.path
import sys
import uuid
import random
import string
from random import randint
from timeit import default_timer as timer

BLOCK_SIZE = 4096
BLOCKS = 100
TESTS = 150


def sizeof_fmt(num, suffix='B'):
    for unit in ['','Ki','Mi','Gi','Ti','Pi','Ei','Zi']:
        if abs(num) < 1024.0:
            return "%3.1f%s%s" % (num, unit, suffix)
        num /= 1024.0
    return "%.1f%s%s" % (num, 'Yi', suffix)

def prev_line():
    sys.stdout.write("\033[F")
    sys.stdout.write("\033[K")
    sys.stdout.flush()

def prog(text):
    prev_line()
    print(text)


def random_str(stringLength=10):
    """Generate a random string of fixed length """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(stringLength))


def main():
    mount = sys.argv[1]
    if (len(sys.argv) != 2):
        print("Usage {} <mount>.")
        sys.exit(1)    

    print('\n'* 2 + '*' * 80)
    print("This test opens {} files and writes data at random offsets(mid-block too).".format(TESTS))
    print("Then all {} files are opened, read and cross checked for correctness.".format(TESTS))
    print("In the end, all {} file descriptors are closed.".format(TESTS * 2))
    print('*' * 80 + '\n')


    file_names = []
    w_files = []
    r_files = []
    stuffs = []
    offsets = []
    results = []
    write_times = []
    read_times = []

    for i in range(TESTS):
        test_file = os.path.join(mount, "test" + uuid.uuid4().hex)
        file_names.append(test_file)

        first_offset = randint(0, BLOCK_SIZE-1)
        second_offset = randint(0, BLOCK_SIZE-1)
        offsets.append( (first_offset, second_offset) )

        stuff = random_str(BLOCK_SIZE-first_offset + BLOCK_SIZE * BLOCKS + second_offset)
        stuffs.append(stuff)

        prog("Writing file {}/{}".format(i+1, TESTS))

        f = open(test_file, 'w')
        t1 = timer()
        f.seek(first_offset)
        f.write(stuff)
        f.flush()
        w_files.append(f)

        write_times.append(len(stuff) / (timer() - t1))


    print("<->")


    for i in range(TESTS):

        prog("Reading file {}/{}".format(i+1, TESTS))

        first_offset, _ = offsets[i]
        
        
        f = open(file_names[i], 'r')
        t1 = timer()
        f.seek(first_offset)
        maybe_stuff = f.read(len(stuffs[i]))

        r_files.append(f)
        results.append(stuffs[i] == maybe_stuff)

        read_times.append( len(stuffs[i]) / (timer() - t1))

    while r_files: 
        r_files.pop().close()
        w_files.pop().close()

    print("Write: {}b/s Read: {}b/s".format( sizeof_fmt(sum(write_times) / TESTS),sizeof_fmt( sum(read_times) / TESTS) ) )
    trues = sum(results)
    print("Tests passed: {}/{}".format(trues, TESTS))

if __name__ == "__main__":
    main()
