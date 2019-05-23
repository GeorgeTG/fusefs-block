import os
import os.path
import sys
import uuid
import random
import string
from random import randint

BLOCK_SIZE = 4096

def random_str(stringLength=10):
    """Generate a random string of fixed length """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(stringLength))

def main():
    mount = sys.argv[1]

    test_file = os.path.join(mount, "test" + uuid.uuid4().hex)
    print("File is : {}".format(test_file))

    first_offset = randint(0, BLOCK_SIZE-1)
    second_offset = randint(0, BLOCK_SIZE-1)
    print("First offset {}".format(first_offset))
    print("Second offset {}".format(second_offset))

    stuff = random_str(BLOCK_SIZE-first_offset + BLOCK_SIZE + second_offset)
    print("Buffer len: {}".format(len(stuff)))

    with open(test_file, 'w', buffering=20000) as f:
        f.seek(first_offset)
        f.write(stuff)
        f.flush()

    maybe_stuff = None

    with open(test_file, 'r') as f:
        f.seek(first_offset)
        maybe_stuff = f.read(len(stuff))
    
    print(len(stuff))
    print(len(maybe_stuff))
    if (stuff == maybe_stuff):
        print ("NICE! :)")
    else:
        print ("MEH! :(")


    



if __name__ == "__main__":
    main()
