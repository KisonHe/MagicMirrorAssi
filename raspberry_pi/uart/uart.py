import serial
import time
import subprocess
import logging
# from systemd.journal import JournalHandler


START_WORD:int = 0xAF
END_WORD:int = 0xFF

FW_HB:int = 0
FW_OPEN:int = 1
FW_CLOSE:int = 2

SFW_RESQUEST:int = 0
SFW_RESPOND:int = 1


mainSer = 0
flagRightSer = False

def closeScreen():
    subprocess.run(['vcgencmd display_power 0'],shell=True)
    pass

def openScreen():
    subprocess.run(['vcgencmd display_power 1'],shell=True)
    pass


def recieve(income:bytes)->tuple:
    print(income)
    # print("income lenth" + str(len(income)))
    tmp_sum = 0
    if (len(income) < 21 or income[0] != START_WORD or income[20] != END_WORD):
        return (-1,)
    for i in range(1,19):
        tmp_sum = tmp_sum + income[i]
        pass
    tmp_sum = tmp_sum & 0xFF
    if (tmp_sum != income[19]):
        return (-2,)

    if (income[1] == FW_OPEN):
        if (income[2] == SFW_RESQUEST):
            print("Got req 2 open")
            openScreen()
            send(mainSer,FW_OPEN,SFW_RESPOND)
    if (income[1] == FW_CLOSE):
        if (income[2] == SFW_RESQUEST):
            print("Got req 2 close")
            closeScreen()
            send(mainSer,FW_OPEN,SFW_RESPOND)

    return (income[1],income[2],income[3:19])
    pass


def send(ser:serial.Serial,FW:int,SFW:int,data:list):
    tmp_sum = 0
    sendList:list = [0] * 21
    if (len(data) != 16 or FW>265 or SFW > 256):
        print("Len of data is " + len(data))
        raise ValueError
    sendList[0] = START_WORD
    sendList[20] = END_WORD
    sendList[1] = FW
    sendList[2] = SFW
    for i in range(16):
        sendList[3+i] = data[i]
        pass
    for j in range(1,19):
        tmp_sum = tmp_sum + sendList[j]
        pass
    
    tmp_sum = tmp_sum & 0xFF
    sendList[19] = tmp_sum
    ser.write(bytearray(sendList))
    pass






# main
# log = logging
# log = logging.getLogger('main')
# log.addHandler(JournalHandler())
# log.setLevel(logging.WARN)

# ----------------------------------------------------

# result = subprocess.run(['ls /dev | grep ttyUSB'],shell=True, stdout=subprocess.PIPE)
# result = result.stdout.decode("utf-8")
# result = result.split()
# for i in result:
#     try:
#         mainSer = serial.Serial("/dev/" + i,baudrate=115200)
#         pass
#     except expression as identifier:
#         pass
#     else:
#         print("Openned " + "/dev/" + i)
#         mainSer.timeout = 0.5
#         send(mainSer,FW_HB,SFW_RESQUEST,[0]*16)
#         flagRightSer = False
#         for j in range(10):
#             tup = recieve(mainSer.read(100))
#             if (tup[0] == FW_HB and tup[1] == SFW_RESPOND):
#                 flagRightSer = True
#                 print("Get Correct Respond")
#                 send(mainSer,FW_HB,SFW_RESQUEST,[0]*16)
#                 break
#             pass
#         if (flagRightSer):
#             break
#         print(i+" didnt respond")
#         # [todo] try to get a heart beat here. if not respond right, try next serial

#         break
# if (not flagRightSer):
#     # log:fail to open any port
#     # log.warn('fail to open any effective UART')
#     pass
# print(result)

# ----------------------------------------------------

try:
    mainSer = serial.Serial("/dev/ttyS0",baudrate=115200)
    pass
except expression as identifier:
    pass
else:
    # print("Openned " + "/dev/ttyS0")
    mainSer.timeout = 0.5
    send(mainSer,FW_HB,SFW_RESQUEST,[0]*16)
    flagRightSer = False
    for j in range(10):
        tup = recieve(mainSer.read(100))
        if (tup[0] == FW_HB and tup[1] == SFW_RESPOND):
            flagRightSer = True
            print("Get Correct Respond")
            send(mainSer,FW_HB,SFW_RESQUEST,[0]*16)
            break
        pass
    if (not flagRightSer):
        print("/dev/ttyS0 didnt respond")
    # [todo] try to get a heart beat here. if not respond right, try next serial


# ----------------------------------------------------

# ser = serial.Serial("/dev/ttyS0")
# # ser = serial.Serial("/dev/ttyUSB0")
# ser.baudrate = 115200
# ser.timeout = 0.01
# ser.close()
# ser.open()
# data=[1,2,3,4,
#         0,0,0,0,
#         0,0,0,0,
#         0,0,0,0]
# while (1):
#     recieve(ser.read(100))
#     # s = ser.read(100)
#     # print(type(s))


# # if (not ser.is_open):
    

