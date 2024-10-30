import copy
import logging
import os
import sys
from tqdm import tqdm

class ColoredConsoleHandler(logging.StreamHandler):
    def emit(self, record):
        # Need to make a actual copy of the record
        # to prevent altering the message for other loggers
        myrecord = copy.copy(record)
        levelno = myrecord.levelno
        if(levelno >= 50):  # CRITICAL / FATAL
            color = '\x1b[31m'  # red
        elif(levelno >= 40):  # ERROR
            color = '\x1b[31m'  # red
        elif(levelno >= 30):  # WARNING
            color = '\x1b[33m'  # yellow
        elif(levelno >= 20):  # INFO
            color = '\x1b[36m'  # cyan
        elif(levelno >= 15):  # DEBUG
            color = '\x1b[35m'  # pink
        elif(levelno == 10):  # notification
            color = '\x1b[32m'  # green
        else:  # NOTSET and anything else
            color = '\x1b[0m'  # normal
        myrecord.msg = color + str(myrecord.msg) + '\x1b[0m'  # normal
        logging.StreamHandler.emit(self, myrecord)


def setup_logger(log_file_path):
    formatter = logging.Formatter(
        '%(asctime)s : %(levelname)s : %(message)s',
        datefmt='%m/%d/%y %I:%M:%S %p'
    )

    file_handler = logging.FileHandler(
        log_file_path,
        mode='w'
    )
    file_handler.setFormatter(formatter)
    file_handler.setLevel(logging.DEBUG)

    def write(self, x):
        if len(x.rstrip()) > 0:
            tqdm.write(x, file=self.file)
    out_stream = type("TqdmStream", (), {'file': sys.stdout, 'write':write})()

    stdout_handler = ColoredConsoleHandler(out_stream)
    stdout_handler.setFormatter(formatter)
    stdout_handler.setLevel(logging.INFO)

    logger = logging.getLogger()
    logger.addHandler(file_handler)
    logger.addHandler(stdout_handler)
    logger.setLevel(logging.INFO)

    # logging.addLevelName(15, "Inside Iteration")     # pink
    # logging.addLevelName(35, "Epoch End")            # yellow
    # logging.addLevelName(45, "Model Save")           # red
    # logging.addLevelName(25, "Log saved")            # cyan
    # logging.addLevelName(12, "Notification")            # cyan

    return logger