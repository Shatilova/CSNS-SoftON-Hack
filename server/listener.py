import socket
import sqlite3
import datetime
import os
import random
import array_shuffler

STATIC_KEY = 'i9jpODi2hql2EpaiDr4WjF5s7Cw96QcDdGNRfkvo'
KEY_SIZE = 20

HEADER_SIZE = 256

# choose minimum value of rand key is 100'000 because
# the number of shuffled bytes depends on the value of the key
# so small values can't guarantee safe shuffle
MIN_RAND_KEY = 100000

# indexes of processing lists
COMMAND = 0
HWID = 1
NICKNAME = 2
LOADER_HASH = 2

# filenames
DB_NAME = 'users_info.sqlite'
HACKINFO_NAME = 'softon_info.txt'
LOADERDATA_NAME = 'Loader.dat'
DLLDATA_NAME = 'SoftON.dat'
DLL_NAME = 'dll.dll'

# complete header to 256 symbols
def complete_header(header):
    completed = header + ('0' * (HEADER_SIZE - len(header)))
    return completed

def get_raw(filename, flag):
    dat_file = open(filename, flag)
    raw = dat_file.read()
    dat_file.close()
    return raw

# simple XOR using string
def str_xor(msg, key):
    i = 0
    j = 0
    output = ""
    for c in msg:
        output = output + chr(ord(msg[i]) ^ ord(key[j]))

        i += 1
        j += 1
        if j >= len(key) : j = 0

    return output

def process(tcp_server):
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    try: 
        conn, addr = tcp_server.sock.accept()
    except socket.timeout as e: 
        if 'timed out' not in str(e):
            print('Accept a connection:', e)
        return

    try: 
        data = conn.recv(1024)
    except socket.error as e: 
        print('Receive data:', e)
        pass

    if data:
        # decode received data
        decoded_data = str(data.decode('utf-8', 'ignore'))

        # separate encrypted data into pieces
        auth_info = decoded_data[0 : len(decoded_data) - KEY_SIZE]
        private_key = decoded_data[len(decoded_data) - KEY_SIZE : len(decoded_data)]

        # decrypt dynamic key with static key
        # then decrypt sent message with dynamic key
        private_key = str_xor(private_key, STATIC_KEY)
        auth_info = str_xor(auth_info, private_key)

        # split message by spaces
        splitted = auth_info.split()

        tcp_server.update_logger()

        # check for correctness command
        if (splitted[COMMAND] != 'LoaderData' 
        and splitted[COMMAND] != 'HackInfo'
        and (splitted[COMMAND] == 'ProcessDll' and len(splitted) < 3)
        and (splitted[COMMAND] == "DllAuth" and len(splitted) < 3)
        and splitted[COMMAND] != "DllData"):
            print('Something went wrong: user sent', splitted)
            tcp_server.logger.info('Something went wrong: user sent %s' % splitted)
        else:
            # RETRIEVE LOADER DATA FILE
            if splitted[COMMAND] == "LoaderData":
                file_size = os.path.getsize(LOADERDATA_NAME)
                rand_key = random.randrange(MIN_RAND_KEY, file_size)

                info_header = 'dat=softon_data,' + str(file_size) + ',' + str(rand_key) + ','

                try:
                    conn.send(str_xor(complete_header(info_header), private_key).encode())
                    conn.send(shuffle_array(bytearray(get_raw(LOADERDATA_NAME, 'rb')), file_size, rand_key))
                except socket.error as e: 
                    print(e)
                    pass

            # RETRIEVE SOFTON HACK INFORMATION
            if splitted[COMMAND]  == 'HackInfo':
                info_sz = os.path.getsize(HACKINFO_NAME)
                info_header = 'msg=softon_info,' + str(info_sz) + ','

                try:
                    conn.send(str_xor(complete_header(info_header), private_key).encode())
                    conn.send(str_xor(get_raw(HACKINFO_NAME, 'r'), private_key).encode())
                except socket.error as e:
                    print(e)
                    pass

             # RETRIEVE DLL DATA FILE
            elif splitted[COMMAND] == 'DllData':
                data_size = os.path.getsize(DLLDATA_NAME)
                rand_key = random.randrange(100000, data_size)

                info_header = 'dat=softon_data,' + str(data_size) + ',' + str(rand_key) + ','
                
                try:
                    conn.send(str_xor(complete_header(info_header), private_key).encode())
                    conn.send(shuffle_array(bytearray(get_raw(DLLDATA_NAME, 'rb')), data_size, rand_key))
                except socket.error as e:
                    print(e)
                    pass

            # RETRIEVE SOFTON HACK DLL
            elif splitted[COMMAND]  == 'ProcessDll':
                print('{} connected from loader (loader hash - {})'.format(splitted[HWID], splitted[LOADER_HASH]))
                tcp_server.logger.info('%s connected from loader (loader hash - %s)' % (splitted[HWID], splitted[LOADER_HASH]))

                db_cur.execute('SELECT expire FROM Subscriber WHERE hwid = ( ? ) LIMIT 1', (splitted[HWID], ))
                expire = db_cur.fetchone()

                # check if entry found
                if expire:
                    expire_date = datetime.datetime.strptime(expire[0], '%Y-%m-%d %H:%M:%S')
                    now_date = datetime.datetime.now()

                    # check if users license is valid
                    if expire_date > now_date:
                        left_to_expire = (expire_date - now_date)

                        response = 'rsp=Your license will expire at ' + expire_date.strftime('%Y-%m-%d %H:%M:%S') + ' (' + str(left_to_expire.days) + 'd ' + str(left_to_expire.seconds // 3600) + 'h' + ')' + '\n,'
                        
                        try: 
                            conn.send(str_xor(complete_header(response), private_key).encode())
                        except socket.error as e: 
                            print(e)
                            pass

                        print('Sending dll to {}'.format(splitted[HWID]))
                        tcp_server.logger.info('Sending dll to %s' % (splitted[HWID])) 

                        dll_size = os.path.getsize(DLL_NAME)
                        rand_key = random.randrange(100000, dll_size)

                        info_header = 'dat=softon_hack,' + str(dll_size) + ',' + str(rand_key) + ','

                        try: 
                            conn.send(str_xor(complete_header(info_header), private_key).encode())
                        except socket.error as e: 
                            print(e)
                            pass

                        try: 
                            conn.send(array_shuffler.shuffle_array(bytearray(get_raw(DLL_NAME, 'rb')), dll_size, rand_key))
                        except socket.error as e:
                            print(e)
                            pass

                    else:
                        response = 'rsp=License is expired\n,'
                        
                        try: 
                            conn.send(str_xor(complete_header(response), private_key).encode())
                        except socket.error as e: 
                            print(e)
                            pass

                        db_cur.execute('DELETE FROM Subscriber WHERE hwid = ( ? )', (splitted[HWID], ))
                else:
                    try: 
                        conn.send(str_xor(complete_header('rsp=License is expired or not yet valid\n,'), private_key).encode())
                    except socket.error as e:
                        print(e)
                        pass

            # SEND SOFTON HACK PLAYER INFO
            elif splitted[COMMAND] == 'DllAuth':
                print('{} connected from dll'.format(splitted[HWID]))
                tcp_server.logger.info('%s connected from dll' % (splitted[HWID]) )

                db_cur.execute('SELECT expire FROM Subscriber WHERE hwid = ( ? ) LIMIT 1', (splitted[HWID], ))
                res = db_cur.fetchone()
                info_header = 'cmd=terminate,' if not res else 'cmd=pass,'
                
                try: 
                    conn.send(str_xor(complete_header(info_header), private_key).encode())
                except socket.error as e:
                    print(e)
                    pass

                if not res:
                    print('{} will be dropped from the game'.format(splitted[HWID]))
                    tcp_server.logger.info('%s will be dropped from the game' % (splitted[HWID]))
                else:
                    db_cur.execute('SELECT * FROM SN_User WHERE hwid = ? AND nickname = ?', (splitted[HWID], splitted[NICKNAME] ) )
                    if len(db_cur.fetchall()) > 0:
                        print('{} found in users list!'.format(splitted[NICKNAME]))
                        tcp_server.logger.info('[SN] %s found in users list!' % (splitted[NICKNAME]))
                    else:
                        print('Adding new user - {}'.format(splitted[NICKNAME]))
                        tcp_server.logger.info('Adding new user - %s' % (splitted[NICKNAME]) )

                        db_cur.execute('INSERT INTO SN_User (hwid, nickname) VALUES (?, ?)', (splitted[HWID], splitted[NICKNAME]) )
						
    conn.close()
    db_conn.commit()
    db_conn.close()