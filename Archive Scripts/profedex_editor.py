import sqlite3
import requests
import datetime
db = '/var/jail/home/team3/profedex.db'

################################################
# Code for editing/accessing player profedexes #
################################################

def request_handler(request):
    if request['method']=='GET' :
        with sqlite3.connect(db) as c:
            c.execute('''CREATE TABLE IF NOT EXISTS {} (profemon_id text, attack int, hp int, move1 text, move2 text, latitude_captured real, longitude_captured real, timing timestamp);'''.format(request['values']['user_id']))
            data = c.execute('''SELECT profemon_id,attack,hp,move1,move2,latitude_captured,longitude_captured FROM {} ORDER BY timing DESC;'''.format(request['values']['user_id'])).fetchall()
        
        if len(data)==0 :
            response = "You have no Profemon! Go out and collect some!"
        
        else :
            response = ""
            limit = 1000
            if "limit" in request['values']:
                limit = int(request['values']['limit'])
            return_count = 0
            for d in data :
                return_count += 1
                response += (d[0] + " -> moves: " + d[3] + ", " + d[4] + " | attack:" + str(d[1]) + " | hp:" + str(d[2]) + "\n")
                if return_count >= limit:
                    break
        
        return response
        
    elif request['method']=='POST' :
        r = requests.post("http://608dev-2.net/sandbox/sc/team3/updating_locations.py?professor={}&lat={}&lon={}".format(request['values']['prof_id'],request['values']['lat_captured'],request['values']['lon_captured'])).text[1:-2]
        r = r.replace("\'","")
        data = r.split(", ")
        attack = int(data[7])
        hp = int(data[8])
        
        with sqlite3.connect(db) as c:
            c.execute('''CREATE TABLE IF NOT EXISTS {} (profemon_id text, attack int, hp int, move1 text, move2 text, latitude_captured real, longitude_captured real, timing timestamp);'''.format(request['values']['user_id'])) # run a CREATE TABLE command

            c.execute('''INSERT into {} VALUES (?,?,?,?,?,?,?,?);'''.format(request['values']['user_id']), (request['values']['prof_id'],attack,hp,data[5],data[6],request['values']['lat_captured'],request['values']['lon_captured'],datetime.datetime.now()))
            
        response = "NEW {} -> moves: {}, {} | attack:{} | hp:{}\n\n".format(request['values']['prof_id'],data[5],data[6],attack,hp)
        response += "Recently Caught:\n"
        profs = c.execute('''SELECT profemon_id,attack,hp FROM {} ORDER BY timing DESC;'''.format(request['values']['user_id'])).fetchall()
        limit = 1000
        if "limit" in request['values']:
            limit = int(request['values']['limit'])
        return_count = 0
        for p in profs :
            return_count += 1
            response += (p[0] + " \n-> attack:" + str(p[1]) + " | hp:" + str(p[2]) + "\n")
            if return_count >= limit:
                break
        
        return response