#!/usr/bin/python3

#-------------------------------------------------------------------------------
# Name:        Check_Size.py
# Purpose:     Check if compiled data fits into the EFM8BB1 flash
#
# Author:      Portisch
#
# Created:     19/02/2019
#-------------------------------------------------------------------------------

from optparse import OptionParser

def parse_file(fn):
    with open(fn) as f:
        for line in f:
            if 'Program Size: data=' in line:
                data = line.split()
                if len(data) == 6:
                    const = data[4].split('=')[1]
                    code = data[5].split('=')[1]
                    if (int(const, 10) + int(code, 10)) > options.size:
                        print 'Check_Size: Error!'
                        print 'Check_Size: Data of ' + str(int(const, 10) + int(code, 10)) + ' will not fit into ' + str(options.size) + ' byte'
                        exit(1)
                    print 'Check_Size: Data of ' + str(int(const, 10) + int(code, 10)) + ' will fit into ' + str(options.size) + ' byte'
                    print 'Check_Size: ' + str(options.size - (int(const, 10) + int(code, 10))) + ' bytes are left'
                    exit(0)

usage = "usage: %prog [options]"
parser = OptionParser(usage=usage, version="%prog 0.0")
parser.add_option("-s", "--size", action="store", type="int",
                  dest="size", default=8192, help="maximum of flash storage")
parser.add_option("-p", "--path", action="store",
                  dest="path", help="path where this script is located")
(options, args) = parser.parse_args()

if __name__ == '__main__':
    '''
    print(len(args))
    if len(args) < 1:
        #parser.error("incorrect number of arguments. Use -h or --help")
        print(parser.print_help())
        exit(1)
    '''
    parse_file(options.path)
