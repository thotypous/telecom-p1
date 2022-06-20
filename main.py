import soundcard as sc
from modem import Modem

def main():
    default_mic = sc.default_microphone()
    default_speaker = sc.default_speaker()

    fs = 48000
    bufsz = fs//300
    modem = Modem(fs, bufsz)
    with default_mic.recorder(samplerate=fs,blocksize=2*bufsz) as mic, \
            default_speaker.player(samplerate=fs,blocksize=2*bufsz) as sp:
        while True:
            data = mic.record(numframes=bufsz)
            #data = data[:,0]   # escolha um canal se for stereo
            modem.put_samples(data)
            sp.play(modem.get_samples())

if __name__ == "__main__":
    main()
