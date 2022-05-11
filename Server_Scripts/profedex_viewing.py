# -*- coding: utf-8 -*-
"""
Created on Sun May  1 13:54:50 2022

@author: jjche
"""
import sqlite3

#####################################################################################
# Python script for viewing profemon in personal profedex in a user-friendly format #
#####################################################################################

database = "/var/jail/home/team3/profedex.db"

def request_handler(request):
    index = int(float(request['values']['index']))
    user = request['values']['user']
    
    conn = sqlite3.connect(database)
    c = conn.cursor()
    
    c.execute('''CREATE TABLE IF NOT EXISTS {} (profemon_id text, attack int, hp int, move1 text, move2 text, latitude_captured real, longitude_captured real, timing timestamp);'''.format(user))
    data = c.execute('''SELECT profemon_id, attack, hp, move1, move2, latitude_captured, longitude_captured FROM {} ORDER BY timing;'''.format(user)).fetchall()
    
    conn.commit()
    conn.close()
    
    response = ""
    
    if index < 0:
        index = 0
    
    length = len(data)
    starting = index*8
        
    if len(data) == 0:
        return "You have no Profemon! Go out and collect some!"
    elif starting > length:
        return "No more Profemon. Please go back!"
    else:
        if length > starting + 8:
            for i in range(8):
                d = data[starting + i]
                response += (d[0] + "\n" + "attack:" + str(d[1]) + " | hp:" + str(d[2]) + "\n")
        else:
            for i in range(starting, length):
                 d = data[i]
                 response += (d[0] + "\n" +"attack:" + str(d[1]) + " | hp:" + str(d[2]) + "\n")
       
    return response