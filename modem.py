import numpy as np

class Modem:
    def __init__(self, fs, bufsz):
        self.fs = fs  # taxa de amostragem
        self.bufsz = bufsz  # quantidade de amostas que devem ser moduladas por vez

    # Modulação
    
    def put_bits(self, bits):
        pass
    
    def get_samples(self):
        return np.zeros(self.bufsz)

    # Demodulação

    def put_samples(self, data):
        pass

    def get_bits(self):
        return []
