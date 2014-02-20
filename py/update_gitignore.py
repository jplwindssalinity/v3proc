#!/usr/bin/env python2.7
""""
Usage: ./update_git_ignore.py directory extension1 extension2 ..etc

For example: ./py/update_git_ignore.py programs C
"""
import os
import sys
import util.file

def main(directory, extensions):
    with open(os.path.join(directory,'.gitignore'),'w') as ofp:
        for extension in extensions:
            print extension
            for file in util.file.find(directory, '*.%s'%extension):
                print>>ofp, os.path.basename(file).rstrip('.%s'%extension)

if __name__ == '__main__':
    directory = sys.argv[1]
    extensions = sys.argv[2:]
    main(directory, extensions)
    