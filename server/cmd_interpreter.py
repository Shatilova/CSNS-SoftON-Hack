import sqlite3
import datetime
import logging

HWID = 0
EXPIRE_DATE = 1
COMMENT = 2

DB_NAME = 'users_info.sqlite'

def help():
    print(
'''
users     - get users info
expire    - expiration date of all licenses
add       - add user
remove    - remove user's license
update    - update user's license
updateall - update licenses of all users
comment   - add or change comment to user
''')

def users():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    db_cur.execute('SELECT * FROM Subscriber')
    subs = db_cur.fetchall()
    for i in range(len(subs)):
        expire_time = datetime.datetime.strptime(subs[i][EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
        now_time = datetime.datetime.now()
        left_time = (expire_time - now_time)

        print('{}. {} - {} ({}d {}h): {}'.format(i + 1, subs[i][HWID], subs[i][EXPIRE_DATE], left_time.days, left_time.seconds // 3600, subs[i][COMMENT]))

    db_conn.close()

def expire():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    db_cur.execute('SELECT * FROM Subscriber ORDER BY expire DESC LIMIT 1')
    expire_sub = db_cur.fetchone()
    if expire_sub:
        expire_time = datetime.datetime.strptime(expire_sub[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
        now_time = datetime.datetime.now()
        left_time = (expire_time - now_time)

        print('Last license will expire:')
        print('{} - {} ({}d {}h): {}'.format(expire_sub[HWID], expire_sub[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, expire_sub[COMMENT]))

    db_conn.close()

def update():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    user_hwid = input('\nEnter a user HWID to work with:\n> ')
    db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user_hwid, ) )
    user = db_cur.fetchone()

    if user:
        expire_time = datetime.datetime.strptime(user[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
        now_time = datetime.datetime.now()
        left_time = (expire_time - now_time)

        print('\nUser info:')
        print('{} - {} ({}d {}h): {}'.format(user[HWID], user[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, user[COMMENT]))

        days_cnt = int(input('\nEnter number of days to add (negative number for removing):\n> '))
        new_expire_time = expire_time + datetime.timedelta(days=days_cnt)
        new_left_time = (new_expire_time - now_time)
        
        print('\nYou are goind to {} {} days {} {} user license. New count day will be {} ({}d {}h)'.format("add" if days_cnt > 0 else "remove", abs(days_cnt), "to" if days_cnt > 0 else "from", user[HWID], new_expire_time, new_left_time.days, new_left_time.seconds // 3600))

        while True:
            confirm = input('\nAre you sure? [Yes/No]\n> ')
            if confirm == 'Yes':
                db_cur.execute('UPDATE Subscriber SET expire = ( ? ) WHERE hwid = ( ? )', (new_expire_time, user[HWID]))

                db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user[HWID], ))
                new_user = db_cur.fetchone()
                new_expire_time = datetime.datetime.strptime(new_user[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
                new_now_time = datetime.datetime.now()
                new_left_time = (new_expire_time - new_now_time)

                print('\nOperation complete. Updated user info:')
                print('{} - {} ({}d {}h): {}'.format(new_user[HWID], new_user[EXPIRE_DATE], new_left_time.days, new_left_time.seconds // 3600, new_user[COMMENT]))

                sn_logger.info('''
Updated %s license:
From: %s (%id %ih)
To: %s (%id %ih)
''' % (new_user[HWID], user[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, new_user[EXPIRE_DATE], new_left_time.days, new_left_time.seconds // 3600))
                break
            elif confirm == 'No':
                print('\nOperation is reset')
                break
            else:
                pass
    else:
        print('User {} not found!'.format(user_hwid))

    db_conn.commit()
    db_conn.close()

def updateall():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    days_cnt = int(input('\nEnter a days number:\n> '))

    print('\nYou are going to {} {} {} all subscibers licenses'.format('add' if days_cnt >= 0 else 'remove', abs(days_cnt), 'to' if days_cnt >= 0 else 'from'))
    while True:
        confirm = input('\nAre you sure? [Yes/No]\n> ')
        if confirm == 'Yes':
            db_cur.execute('SELECT * FROM Subscriber')
            users_list = db_cur.fetchall()

            for user in users_list:
                expire_time = datetime.datetime.strptime(user[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
                new_expire_time = expire_time + datetime.timedelta(days=days_cnt)

                db_cur.execute('UPDATE Subscriber SET expire = ( ? ) WHERE hwid = ( ? )', (new_expire_time, user[HWID]))

            print('\nOperation completed!')

            break
        elif confirm == 'No':
            print('\nOperation is reset')
            break
        else : pass

    db_conn.commit()
    db_conn.close()

def add():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    user_hwid = input('\nEnter user HWID to add:\n> ')
    db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user_hwid,))

    already_exists = db_cur.fetchone()
    if already_exists:
        expire_time = datetime.datetime.strptime(already_exists[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
        now_time = datetime.datetime.now()
        left_time = (expire_time - now_time)

        print('\nUser already exists:')
        print('{} - {} ({}d {}h): {}'.format(already_exists[HWID], already_exists[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, already_exists[COMMENT]))
    else:
        days_cnt = int(input('\nEnter days count:\n> '))
        now_time = datetime.datetime.now()
        expire_time = now_time + datetime.timedelta(days=days_cnt)
        expire_time_str = expire_time.strftime('%Y-%m-%d %H:%M:%S')
        left_time = expire_time - now_time

        comment = input('\nEnter a comment:\n> ')

        print('\nCheck the correctness of the entered data:')
        print('{} - {} ({}d {}h): {}'.format(user_hwid, expire_time_str, left_time.days, left_time.seconds // 3600, comment))


        while True:
            confirm = input('\nConfirm operation?[Yes/No]\n> ')

            if confirm == 'Yes':
                db_cur.execute('INSERT INTO Subscriber (hwid, expire, comment) VALUES ( ?, ?, ? )', (user_hwid, expire_time_str, comment) )
                db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? ) LIMIT 1', (user_hwid,) )
               
                is_ok = db_cur.fetchone()
                if is_ok:
                    expire_time = datetime.datetime.strptime(is_ok[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
                    now_time = datetime.datetime.now()
                    left_time = (expire_time - now_time)

                    print('\nUser added succesfully:')
                    print('{} - {} ({}d {}h): {}'.format(is_ok[HWID], is_ok[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, is_ok[COMMENT]))
                    sn_logger.info('''
Added %s - %s (%id %ih): %s
''' % (is_ok[HWID], is_ok[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, is_ok[COMMENT]))
                else:
                    print('\nSomething went wrong. User was not added...')
                break
            elif confirm == 'No':
                print('\nOperation is reset')
                break
            else:
                pass

    db_conn.commit()
    db_conn.close()

def remove():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    user_hwid = input('\nEnter user HWID to remove:\n> ')

    db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user_hwid, ))
    user_exists = db_cur.fetchone()
    if user_exists:
        expire_time = datetime.datetime.strptime(user_exists[EXPIRE_DATE], '%Y-%m-%d %H:%M:%S')
        now_time = datetime.datetime.now()
        left_time = (expire_time - now_time)

        print('\nUser info:')
        print('{} - {} ({}d {}h): {}'.format(user_exists[HWID], user_exists[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, user_exists[COMMENT]))

        while True:
            confirm = input('\nAre you sure? [Yes/No]\n> ')
            if confirm == 'Yes':
                db_cur.execute('DELETE FROM Subscriber WHERE hwid = ( ? )', (user_hwid, ))

                db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user_hwid,))
                is_found = db_cur.fetchone()
                if is_found:
                    print('Something went wrong. User has not been removed')
                    break
                else:
                    print('User was removed successfully')
                    sn_logger.info('''
Removed %s - %s (%id %ih): %s
''' % (user_exists[HWID], user_exists[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, user_exists[COMMENT]))
                    break

        db_conn.commit()
        db_conn.close()

def comment():
    db_conn = sqlite3.connect(DB_NAME)
    db_cur = db_conn.cursor()

    user_hwid = input('\nEnter user HWID:\n> ')
    db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? ) LIMIT 1', (user_hwid,))
    user_exists = db_cur.fetchone()

    if user_exists:
        print('User comment: {}'.format(user_exists[COMMENT]))
        new_comment = input('\nEnter new comment:\n> ')
        while True:
            confirm = input('\nYou are going to change user comment. Are you sure? [Yes/No]\n> ')

            if confirm == 'Yes':
                db_cur.execute('UPDATE Subscriber SET comment = ( ? ) WHERE hwid = ( ? )', (new_comment, user_hwid))
                db_cur.execute('SELECT * FROM Subscriber WHERE hwid = ( ? )', (user_hwid, ))

                updated_user = db_cur.fetchone()
                if updated_user:
                    expire_time = datetime.datetime.strptime(updated_user[1], '%Y-%m-%d %H:%M:%S')
                    now_time = datetime.datetime.now()
                    left_time = (expire_time - now_time)

                    print('\nUpdated user info:')
                    print('{} - {} ({}d {}h): {}'.format(updated_user[HWID], updated_user[EXPIRE_DATE], left_time.days, left_time.seconds // 3600, updated_user[COMMENT]))
                    sn_logger.info('''
Updated %s comment:
From: %s
To: %s
''' % (user_hwid, user_exists[COMMENT], new_comment))
                else:
                    print('Something went wrong. User not found')
                break
            elif confirm == 'No':
                print('\nOperation is reset')
                break
            else:
                pass

    db_conn.commit()
    db_conn.close()

sn_logger = logging.getLogger('softon_cmd_history')
sn_logger.setLevel(logging.INFO)
fh = logging.FileHandler('softon_cmd_history.log')
fh.setFormatter(logging.Formatter('%(asctime)s: %(message)s', '%Y-%m-%d %H:%M:%S'))
sn_logger.addHandler(fh)    