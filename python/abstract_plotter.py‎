from abc import ABC, abstractmethod
import json


class AbstractPlotter(ABC):
    def __init__(self, json_data_path, output_path):
        self.output_path = output_path

        with open(json_data_path) as json_file:
            self.data = json.load(json_file)

    @abstractmethod
    def plot(self):
        pass
