import pyaudio
import numpy as np
import curses
import sys
from modem import Modem


def main(ans=False):
    p = pyaudio.PyAudio()
    
    fs = 48000
    bufsz = fs//300
    uart = UartEmu()
    modem = Modem(fs, bufsz, ans)
    
    stream = p.open(format=pyaudio.paFloat32, channels=1,rate=fs, input=True,output=True,frames_per_buffer=2*bufsz)
    if stream :
        stream.start_stream()
        while True:
            # Modulação
            modem.put_bits(uart.get_bits())
            samples = modem.get_samples()
            stream.write(np.array(samples,dtype=np.float32).tobytes(), bufsz)

            # Demodulação
            data = np.frombuffer(stream.read(bufsz), dtype=np.float32)
            modem.put_samples(np.squeeze(data))
            uart.put_bits(modem.get_bits())


class UartEmu:
    """ 8-N-1 """

    def __init__(self):
        self.stdscr = curses.initscr()
        self.stdscr.nodelay(True)
        self.fsm_state = 'outside'
        self.fsm_value = 0
        self.fsm_num = 0

    def get_bits(self):
        bits = []
        for c in self.__get_chars():
            bits.append(0)  # start bit
            # LSB to MSB
            for _ in range(8):
                bits.append(c & 1)
                c >>= 1
            bits.append(1)  # stop bit
        return bits

    def put_bits(self, bits):
        for b in bits:
            if self.fsm_state == 'outside':
                if b == 0:
                    self.fsm_state = 'inside'
                    self.fsm_value = 0
                    self.fsm_num = 0
            elif self.fsm_state == 'inside':
                self.fsm_num += 1
                self.fsm_value >>= 1
                self.fsm_value |= 0x80 if b else 0x00
                if self.fsm_num == 8:
                    sys.stdout.write(chr(self.fsm_value))
                    sys.stdout.flush()
                    self.fsm_state = 'outside'
            else:
                raise ValueError('invalid state')

    def __get_chars(self):
        while True:
            c = self.stdscr.getch()
            if c == -1:
                break
            if c == 10:
                sys.stdout.write('\n')
                sys.stdout.flush()
                yield 13  # convert '\n' to '\r\n'
            yield c


def usage():
    print(f'{sys.argv[0]} [call|ans]', file=sys.stderr)
    sys.exit(1)


if __name__ == "__main__":
    ans = False
    if len(sys.argv) == 2:
        if sys.argv[1] == 'call':
            ans = False
        elif sys.argv[1] == 'ans':
            ans = True
        else:
            usage()
    elif len(sys.argv) > 2:
        usage()

    main(ans)