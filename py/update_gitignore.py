#!/usr/bin/env python2.7

import os
import sys
import util.file

def main(directory, extension):
    outfile = os.path.join(directory,'.gitignore')
    print outfile
    with open(os.path.join(directory,'.gitignore'),'w') as ofp:
        for file in util.file.find(directory, '*.%s'%extension):
            print>>ofp, os.path.basename(file).rstrip('.%s'%extension)

if __name__ == '__main__':
    directory = sys.argv[1]
    extension = sys.argv[2]
    main(directory, extension)
    