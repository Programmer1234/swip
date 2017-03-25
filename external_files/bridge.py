from scapy.all import *
conf.verb = 0 # not verbose
import time

'''
RTOS_IP = '10.0.0.111'
RTOS_MAC = '6c:71:d9:bf:8e:fb'
BPF_FILT = 'ip and ether dst %s'%RTOS_MAC

DF_GW = '10.0.0.138'
NETWORK = '10.0.0.0/24'
'''

RTOS_IP = '192.168.43.111'
RTOS_MAC = '6c:71:d9:bf:8e:fb' #over WIFI - cannot spoof source MAC. must use host pc mac address
BPF_FILT = 'ip dst %s and ether dst %s'%(RTOS_IP, RTOS_MAC)
DF_GW = '192.168.43.1'
NETWORK = '192.168.43.0/24'




####################################################################
import serial
import serial.tools.list_ports

def get_serial_port():
    for port in serial.tools.list_ports.comports():
        if 'User UART' in port.description:
            return port.device
    
    raise Exception('Failed to find UART port')

def getPort():
    return serial.Serial(
            port=get_serial_port(),
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            parity=serial.PARITY_NONE)
    
port = getPort()

####################################################################

#!/usr/bin/env python
import serial
import binascii

# SLIP decoder
class slip():

    def __init__(self):
        self.started = False
        self.escaped = False
        self.stream = ''
        self.packet = ''
        self.SLIP_END = '\xc0'		# dec: 192
        self.SLIP_ESC = '\xdb'		# dec: 219
        self.SLIP_ESC_END = '\xdc'	# dec: 220
        self.SLIP_ESC_ESC = '\xdd'	# dec: 221
        self.serialComm = None

    def appendData(self, chunk):
        self.stream += chunk

    def decode(self):
        packetlist = []
        for char in self.stream:
            # SLIP_END
            if char == self.SLIP_END:
                if self.started:
                    packetlist.append(self.packet)
                else:
                    self.started = True
                self.packet = ''
            # SLIP_ESC
            elif char == self.SLIP_ESC:
                self.escaped = True
            # SLIP_ESC_END
            elif char == self.SLIP_ESC_END:
                if self.escaped:
                    self.packet += self.SLIP_END
                    self.escaped = False
                else:
                    self.packet += char
            # SLIP_ESC_ESC
            elif char == self.SLIP_ESC_ESC:
                if self.escaped:
                    self.packet += self.SLIP_ESC
                    self.escaped = False
                else:
                    self.packet += char
            # all others
            else:
                if self.escaped:
                    raise Exception('SLIP Protocol Error')
                    self.packet = ''
                    self.escaped = False
                else:
                    self.packet += char
                    self.started = True
        self.stream = ''
        self.started = False
        return (packetlist)
        
    def encode(self, packet):
        # Encode an initial END character to flush out any data that 
        # may have accumulated in the receiver due to line noise
        encoded = self.SLIP_END
        for char in packet:
            # SLIP_END
            if char == self.SLIP_END:
                encoded +=  self.SLIP_ESC + self.SLIP_ESC_END
            # SLIP_ESC
            elif char == self.SLIP_ESC:
                encoded += self.SLIP_ESC + self.SLIP_ESC_ESC
            # the rest can simply be appended
            else:
                encoded += char
        encoded += self.SLIP_END
        return (encoded)

####################################################################

import threading

slipClass = slip()




def respondArp(p):
   print '[ARP RESPONDER] send arp respond to %s\n'%(p.payload.psrc)
   sendp( Ether(dst=p.src, src=RTOS_MAC) / ARP(op=ARP.is_at, hwsrc=RTOS_MAC, psrc=p.payload.pdst, hwdst=p.payload.hwsrc, pdst=p.payload.psrc) )

ARP_BPF_FILTER = 'arp and (arp[6:2]=1) and dst host %s'%RTOS_IP
def arp_sniff_responder():
    print 'start ARP-Responder %s   --   %s\n'%(RTOS_IP, RTOS_MAC)
    sniff(filter=ARP_BPF_FILTER, prn=respondArp)
    
t_arp_resonder = threading.Thread(target=arp_sniff_responder)
t_arp_resonder.start()
   

   
   
   

from netaddr import IPNetwork, IPAddress

MAC_DICT = {'1.1.1.1': '12:34:56:aa:bb:cc'} # example
# can be very blocking if no response in "sr"
def get_mac(ip):
    # if dst is not in lan, enumerate default gateway mac address as dest
    if IPAddress(ip) not in IPNetwork(NETWORK):
        ip = DF_GW
    #
    if MAC_DICT.has_key(ip):
        return MAC_DICT[ip]
    #
    print '%s mac is not in cache... sending arp who-is' % ip, '\n'
    result = sr(ARP(op=ARP.who_has, psrc=RTOS_IP, pdst=ip))[0] #[0] - resulted packets
    if len( result ) == 0:
        raise Exception('Failed to find %s mac dynamicly'%ip)
    MAC_DICT[ip] = result[0][1][ARP].hwsrc
    return MAC_DICT[ip]

def send_packet_to_ethernet(ipLayer):
    dstMac = get_mac(ipLayer.dst)
    sendp( Ether(src=RTOS_MAC, dst=dstMac)/ipLayer )
    if DEBUG:
        print '[from uart] send packet to ethernet: ', ipLayer.summary(), '\n'

DEBUG = True
def slipRecvWorker(slipClass):
    print 'uart listening thread started...\n'
    while True:
        c =  port.read()
        slipClass.appendData(c)
        
        if c == slipClass.SLIP_END:
            p_list = filter(lambda x: len(x)>0, slipClass.decode())
            if len(p_list) > 0:
                p_list = map(lambda x: IP(_pkt=x), p_list)
                for p in p_list:
                    if DEBUG:
                        print '[from uart] recv packet from uart: ', p.summary(), '\n'
                    send_packet_to_ethernet(p) 
    return

t_recv_uart = threading.Thread(target=slipRecvWorker, args=(slipClass,))
t_recv_uart.start()








SEND_FRAG_SLEEP = 0.05
SEND_FRAG_SIZE = 20
SEND_FRAGMENTIZE = True
def send_packet_to_uart(p):
    if not p.haslayer(IP):
        if DEBUG:
            print '[from ethernet] packet sniffed not IP: ', p.summary(), '\n'
        return
    p = p.getlayer(IP)
    if DEBUG:
        print '[from ethernet] packet sniffed: ', p.summary(), '\n'
    p_slip = slipClass.encode(str(p))
    #
    if SEND_FRAGMENTIZE:
        fragments = [p_slip[i:i+SEND_FRAG_SIZE] for i in range(0, len(p_slip), SEND_FRAG_SIZE)]
        for f in fragments:
            port.write(f)
            time.sleep(SEND_FRAG_SLEEP)
    else:
        port.write(p_slip)
    #
    if DEBUG:
        print '[from ethernet] packet was sent to uart: ', p.summary(), '\n'

def etherSniffer2uart(slipClass):
    print 'start Ethernet sniffer    --   "%s"\n'%(BPF_FILT)
    sniff(filter=BPF_FILT, prn=send_packet_to_uart)
    return
    
t_ether2uart = threading.Thread(target=etherSniffer2uart, args=(slipClass,))
t_ether2uart.start()