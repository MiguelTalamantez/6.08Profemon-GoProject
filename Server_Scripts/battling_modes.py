# from datetime import datetime
import sqlite3
import datetime
import time

def request_handler(request):

    if request['method'] == 'GET':
        user = request['values']['user']
        opp = request['values']['opp'].strip()

        conn = sqlite3.connect('/var/jail/home/team3/realtime_loc.db')
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS ongoing_battles (user text, opponent text, mode text, timing timestamp);''')
        # user_info = c.execute('''SELECT * from ongoing_battles WHERE EXISTS(SELECT * FROM ongoing_battles WHERE (user = (?) AND opponent = (?)) ) ORDER BY timing DESC;''', (user, opp)).fetchone()
        user_info = c.execute('''SELECT * from ongoing_battles WHERE (user = (?) AND opponent = (?)) ORDER BY timing DESC;''', (user, opp)).fetchone()

        if user_info != None: # if the entry exists in the table
            
            # user_info = c.execute('''SELECT * from ongoing_battles WHERE (user = (?) AND opponent = (?)) ORDER BY timing DESC;''', (user, opp)).fetchone()
            mode, timed = user_info[2], user_info[3]
            datetimed = datetime.datetime.strptime(timed, '%Y-%m-%d %H:%M:%S.%f')
            
            if mode == 'no': # if last entry was no, then see if it has passed 10 minutes 
                ten_min_ago = datetime.datetime.now() - datetime.timedelta(minutes=10)
                
                if datetimed > ten_min_ago:
                    conn.commit()
                    conn.close()
                    return "not ready"
        
        c.execute('''INSERT into ongoing_battles VALUES (?,?,?,?);''', (user, opp, 'yes', datetime.datetime.now()))
        conn.commit()
        conn.close()
        return "ready"


    elif request['method'] == 'POST':
        user = request['form']['user']
        opp = request['form']['opp']
        play = request['form']['play']
        
        conn = sqlite3.connect('/var/jail/home/team3/realtime_loc.db')
        c = conn.cursor()
        
        if play == 'no':
            c.execute('''INSERT into ongoing_battles VALUES (?,?,?,?);''', (user, opp, 'no', datetime.datetime.now()))
            conn.commit()
            conn.close()
            return 'no'
        
        elif play == 'yes':
            c.execute('''INSERT into ongoing_battles VALUES (?,?,?,?);''', (user, opp, 'yes', datetime.datetime.now()))
            time.sleep(2)
            # response = None
            # while response is None:
            response = c.execute('''SELECT * from ongoing_battles WHERE (user = (?) AND opponent = (?)) ORDER BY timing DESC;''', (opp, user)).fetchone()
            # conn.commit()
            # conn.close()
            if response != None:
                agreement = response[2]
                conn.commit()
                conn.close()
                return agreement
            # if agreement == 'no':
            #     c.execute('''INSERT into ongoing_battles VALUES (?,?,?,?);''', (user, opp, 'no', datetime.datetime.now()))
            #     conn.commit()
            #     conn.close()
            #     return 'lol'

            conn.commit()
            conn.close()
            return 'no'

            agreement = response[2]

            if agreement == 'no':
                c.execute('''INSERT into ongoing_battles VALUES (?,?,?,?);''', (user, opp, 'no', datetime.datetime.now()))
                conn.commit()
                conn.close()
                return 'no'
            conn.commit()
            conn.close()
            return 'yes'
