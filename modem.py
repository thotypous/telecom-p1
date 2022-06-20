import numpy as np

class Modem:
    def __init__(self, fs):
        self.fs = fs   # taxa de amostragem

    # Modulação
    
    def put_bits(self, bits):
        pass
    
    def get_samples(self):
        return np.zeros(160)

    # Demodulação

    def put_samples(self, _data):
        pass

    def get_bits(self):
        return []
