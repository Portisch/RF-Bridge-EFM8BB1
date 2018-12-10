#!/usr/bin/python3

#-------------------------------------------------------------------------------
# Name:        BitBucketConverter.py
# Purpose:     Generate 'B0' message from received 'B1' data.
#
# Author:      gerardovf
#
# Created:     05/09/2018
# Editor:      Portisch
#-------------------------------------------------------------------------------

from optparse import OptionParser
import pycurl
from sys import exit
from optparse import OptionParser
#from io import StringIO
from io import BytesIO
from PIL import Image, ImageFont, ImageDraw

# Output:
# Example: AA B0 23 04 14 0224 03FB 0BF4 1CAC 23011010011001010110101001100101011010100101100110 55
# 0xAA: sync start
# 0xB0: command
# 0x23: len command
# 0x04: bucket count
# 0x14: repeats
# buckets 0-4
# data
# 0x55: sync end

def getInputStr():
    #auxStr = '18:30:23 MQT: /sonoff/bridge/RESULT = {"RfRaw":{"Data":"AAB104022403FB0BF41CAC0110100110010101101010011001010110101001011001102355"}}'
    auxStr = raw_input("Enter B1 line: ")
    auxStr = auxStr.replace(' ', '')
    iPosEnd = auxStr.rfind('55')
    if (iPosEnd > -1):
        iPosStart = auxStr.find('AAB1')
        if iPosStart > -1:
            auxStr = auxStr[iPosStart:iPosEnd+len('55')]
            print('Filtered 0xB1 data: ' + auxStr)
        else:
            auxStr = ""
    else:
        auxStr = ""
    return auxStr

def sendCommand(szOutFinal, mydevice):
    #buffer = StringIO()
    buffer = BytesIO()
    url = str("http://{}/ax?c2=191&c1=RfRaw%20".format(mydevice))
    url += szOutFinal
    print(url)
    print("Sending command to bridge")
    c = pycurl.Curl()
    c.setopt(c.URL, url)
    c.setopt(c.WRITEDATA, buffer)
    c.perform()
    c.close()
    body = buffer.getvalue()
    #print(body)

def DrawImage(buckets, data):
    lines = []
    font = ImageFont.load_default()
    Bucket_High_Low_Marking = False
    repeats = 2

    if (options.debug):
        print("DrawImage, try to draw a image of: " + data)

    for i in range(0, len(data)):
        if ((int(data[i:i+1], 16) & 0x08) != 0):
            if (options.debug):
                print("DrawImage, bucket high/low marking is included")
            Bucket_High_Low_Marking = True
            break;

    for i in range(0, len(data)):
        if (data[i:i+1] != 'F'):
            lines.append(buckets[int(data[i:i+1], 16) & 0x07])

    f = 0.03

    import math
    picture = Image.new("L", (int(math.ceil(sum(lines * repeats) * f)) + 10, 150))
    draw = ImageDraw.Draw(picture)

    draw.rectangle(((0, 0), (picture.width - 1, picture.height - 1)), fill="white", outline="black")

    x = 0
    if Bucket_High_Low_Marking:
        inverted = not ((int(data[0:1], 16) & 0x08) >> 3)
    else:
        inverted = 0

    y = inverted
    lastX = x
    lastY = y

    for a in range(0, repeats):
        y = inverted
        for i in range(len(lines)):
            lastX = x
            x += lines[i]

            # horizontal line
            draw.line(((lastX * f, y * 50 + 25), (x * f, y * 50 + 25)), fill="black")

            # vertical line
            if not (i == 0 and lastY == y):
                draw.line(((lastX * f, y * 50 + 25), (lastX * f, (not y) * 50 + 25)), fill="black")
                
            draw.line(((lastX * f, picture.height - 70), (lastX * f, picture.height - 30)), fill="grey")

            img_txt = Image.new("L", font.getsize(str(lines[i])))
            draw_txt = ImageDraw.Draw(img_txt)
            draw_txt.rectangle(((0, 0), (img_txt.width, img_txt.height)), fill="white")
            draw_txt.text((0,0), str(lines[i]),  font=font, fill="black")
            picture.paste(img_txt.rotate(90, expand=1), (int(math.ceil(lastX * f)) + 2, 10 + 50 + 25))         
            lastY = y
            y = not y

    filename = "oscilloscope.jpg"

    picture.save(filename)
    try:
        import webbrowser
        webbrowser.open(filename)
    except:
        print("Saved oscilloscope screen to " + filename)

def findSyncPattern(szData):    
    syncData = None

    if ((len(szData) % 2) != 0):
        print("Missing bucket in data...")
        sys.exit()

    # try first if the second sync bucket is on the end of the data
    bucketAtEnd = True
    for i in range(0, len(szData) - 2, 2):
        if ((int(szData[i:i+1], 16) & 0x07) == (int(szData[i+1:i+2], 16) & 0x07)):
            bucketAtEnd = False
            break

    # try next if the second sync bucket is on the front of the data
    bucketAtFront = True
    for i in range(1, len(szData) - 1, 2):
        if ((int(szData[i:i+1], 16) & 0x07) == (int(szData[i+1:i+2], 16) & 0x07)):
            bucketAtFront = False
            break

    if (bucketAtEnd and bucketAtFront):
        print("Something is wrong with the RF data!")
        #sys.exit()
    elif bucketAtEnd:
        syncData = szData[-2:] + szData[0:-2]

        if (options.debug):
            print("Second sync bucket is on end")
            print("Sync buckets: " + szData[-2:])   
            print("New data: " + syncData)

    elif bucketAtFront:
        syncData = szData[-1:] + szData[0:-1]

        if (options.debug):        
            print("Second sync bucket is in front")
            print("Sync buckets: " + szData[-1:] + szData[0:1])        
            print("New data: " + syncData)        

    return syncData

def decodeBuckets(buckets, data):
    # find used buckets
    arrBuckets = [0] * len(buckets)
    for i in range(0, len(data)):
        if (int(data[i:i+1], 16) != 0x0F):
            arrBuckets[int(data[i:i+1], 16) & 0x07] = buckets[int(data[i:i+1], 16) & 0x07]

    # remove unused buckets
    i = 0
    while i < len(arrBuckets):
        if (arrBuckets[i] == 0):
            arrBuckets.pop(i);
        else:
            i += 1

    if (len(arrBuckets) > 2):
        print("Failed to decode buckets...")
        return

    if (arrBuckets[0] > arrBuckets[1]):
        bit1_mask = "01"
    else:
        bit1_mask = "10"

    print("Bitcount: %d" % ((len(data)) / 2))

    code = ""

    for i in range(0, len(data), 2):
        if (data[i:i+2] == bit1_mask):
            code += "1"
        else:
            code += "0"

    return "0x%X" % int(code, 2)

def main(szInpStr, repVal):
    if (options.debug):
        print("%s" % szInpStr)

    if (options.verbose):
        print("Repeat: %d" % repVal)

    iNbrOfBuckets = int(szInpStr[4:6], 16)
    arrBuckets = []

    if (options.verbose):
        print("Number of buckets: %d" % iNbrOfBuckets)

    # Start packing
    szOutAux = " %0.2X " % iNbrOfBuckets
    szOutAux += "%0.2X " % repVal

    for i in range(0, iNbrOfBuckets):
        szOutAux += szInpStr[6+i*4:10+i*4] + " "
        arrBuckets.append(int(szInpStr[6+i*4:10+i*4], 16))    

    syncData = findSyncPattern(szInpStr[10+i*4:-2])

    if (syncData != None):
        szOutAux += syncData
        DrawImage(arrBuckets, syncData)
        szBits = decodeBuckets(arrBuckets, syncData[2:])
        if (szBits != None):
            print("Decoded value: " + szBits)
    else:
        DrawImage(arrBuckets, szInpStr[10+i*4:-2])
        szOutAux += szInpStr[10+i*4:-2]

    szDataStr = szOutAux.replace(' ', '')
    szOutAux += " 55"
    iLength = int(len(szDataStr) / 2)
    szOutAux = "AA B0 " + "%0.2X" % iLength + szOutAux

    if (options.device):
        sendCommand(szOutAux, options.device)

    print("Resulting 0xB0 data: " + szOutAux)

usage = "usage: %prog [options]"
parser = OptionParser(usage=usage, version="%prog 0.3")
parser.add_option("-e", "--dev", action="store", type="string",
                  dest="device", help="device to send RfRaw B0 command")
parser.add_option("-r", "--repeat", action="store",
                  dest="repeat", default=20, help="number of times to repeat")
parser.add_option("-d", "--debug", action="store_true",
                  dest="debug", default=False, help="show debug info")
parser.add_option("-v", "--verbose", action="store_true",
                  dest="verbose", default=False, help="show more detailed info")
(options, args) = parser.parse_args()

# In program command line put two values (received Raw between '"' and desired repeats)
# Example: "AA B1 04 0224 03FB 0BF4 1CAC 01101001100101011010100110010101101010010110011023 55" 20
if __name__ == '__main__':
    '''
    print(len(args))
    if len(args) < 1:
        #parser.error("incorrect number of arguments. Use -h or --help")
        print(parser.print_help())
        exit(1)
    '''
    print(parser.print_help())

    while(True):    
        strInput = getInputStr()
        if (len(strInput) > 0):
            main(strInput, options.repeat)
        else:
            break
        print(parser.print_help())
