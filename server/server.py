import socket
import sqlite3
import datetime
import logging
from threading import Thread, Lock

import cmd_interpreter
import listener

IP = '0.0.0.0'
PORT = 12053

class TcpServer(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.lock = Lock()
        self.is_listening = True
        self.last_date = str(datetime.date.today())

        self.update_logger(force=True)
    
    # function to update log file
    # one day - one file named like "YYYY_Mon_DD.log"
    # if "forse" is True then log is updated anyway
    def update_logger(self, force=False):
        if force == True or self.last_date != str(datetime.date.today()):
            if force == False:
                self.last_date = str(datetime.date.today())

            self.logger = logging.getLogger('softon')
            self.logger.setLevel(logging.INFO)
            fh = logging.FileHandler(datetime.date.today().strftime("%Y_%b_%d") + '.log')
            fh.setFormatter(logging.Formatter('%(asctime)s: %(message)s', '%H:%M:%S'))
            self.logger.addHandler(fh)

            print('UPDATING LOGGER...')

    def run(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            self.sock.bind((IP, PORT))
        except socket.error as e:
            print('Bind address:', e)
            return

        self.sock.setblocking(0)
        self.sock.settimeout(5)
        self.sock.listen(3)

        while self.is_listening:
            listener.process(self)

        self.stop()

    def stop(self):
        self.sock.close()

if __name__ == "__main__":
    tcp_server = TcpServer()
    tcp_server.start()
    is_running = True

    print("SoftON Server")
    print("--------------------------------------")
    print("help : hack commands")
    print("q    : quit server")
    print("--------------------------------------")

    # a kind of retrograd method to work with users database - console commands
    while is_running:
        cmd = input('')

        if cmd == 'help' : cmd_interpreter.help()
        elif cmd == 'users' : cmd_interpreter.users()
        elif cmd == 'expire' : cmd_interpreter.expire()
        elif cmd == 'update' : cmd_interpreter.update()
        elif cmd == 'updateall' : cmd_interpreter.updateall()
        elif cmd == 'add' : cmd_interpreter.add()
        elif cmd == 'remove' : cmd_interpreter.remove()
        elif cmd == 'comment' : cmd_interpreter.comment()
        elif cmd == 'q':
            print("Shutting down server...")
            tcp_server.is_listening = False
            is_running = False
        else: print('Unknown command')
