import sqlite3

database = "/var/jail/home/team3/profedex.db"

#############################################################################################
# Python script for viewing profemon in personal profedex in order to select one for battle #
#############################################################################################

def request_handler(request):

    if request['method'] == 'GET' and 'select' in request['args']:
        index = int(float(request['values']['index']))
        user = request['values']['user']
        
        conn = sqlite3.connect(database)
        c = conn.cursor()
        
        c.execute('''CREATE TABLE IF NOT EXISTS {} (profemon_id text, attack int, hp int, move1 text, move2 text, latitude_captured real, longitude_captured real, timing timestamp);'''.format(user))
        data = c.execute('''SELECT profemon_id, attack, hp, move1, move2, latitude_captured, longitude_captured FROM {} ORDER BY timing;'''.format(user)).fetchall()
        
        conn.commit()
        conn.close()

        
        if index < 0:
            index = 0
        length = len(data)
            
        if len(data) == 0:
            return "You have no Profemon! Go out and collect some!"
        elif index >= length:
            return "No more Profemon. Please go back!"
        else:
            d = data[index]
            return str(d[0])  


    if request['method'] == 'GET':
        index = int(float(request['values']['index']))
        user = request['values']['user']
        
        conn = sqlite3.connect(database)
        c = conn.cursor()
        
        c.execute('''CREATE TABLE IF NOT EXISTS {} (profemon_id text, attack int, hp int, move1 text, move2 text, latitude_captured real, longitude_captured real, timing timestamp);'''.format(user))
        data = c.execute('''SELECT profemon_id, attack, hp, move1, move2, latitude_captured, longitude_captured FROM {} ORDER BY timing;'''.format(user)).fetchall()
        
        conn.commit()
        conn.close()
        
        
        if index < 0:
            index = 0
        length = len(data)
            
        if len(data) == 0:
            return "You have no Profemon! Go out and collect some!"
        elif index >= length:
            return "No more Profemon. Please go back!"
        else:
            d = data[index]
            response = d[0] + "\n" + "attack:" + str(d[1]) + " | hp:" + str(d[2])  
            return response