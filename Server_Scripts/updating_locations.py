import sqlite3
import datetime
import random
# import requests
from bs4 import BeautifulSoup

############################
# Randomly spawns Profemon #
############################

def request_handler(request):

    if request['method'] == 'GET' and 'start' not in request['args']:
        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        on_deck = c.execute('''SELECT * from locations WHERE location != (?);''', ('None', )).fetchall()
        conn.commit()
        conn.close()
        return on_deck

    if request['method'] == 'POST':
        captured = request['values']['professor'] 
        found = (request['values']['lat'], request['values']['lon'])
        
        deck = 4*['Peter Dourmashkin'] + 9*['Donald Sadoway'] + 7*['Adam Willard'] + 5*['Brad Pentelute'] + 3*['Rick Danheiser'] + 8*['John Bush'] + 7*['Larry Guth'] + 5*['David Jerison'] + 3*['Bjorn Poonen'] + 1*['Tristan Collins'] + 9*['John Guttag'] + 6*['Adam Hartz'] + 3*['Max Goldman'] + 7*['Silvina Hinono Wachman'] + 5*['Katrina LaCurts'] + 3*['Steven Leeb'] + 1*['Joe Steinmeyer'] + 7*['Erik Demaine'] + 5*['Mauricio Karchmer'] + 2*['David Karger']
        clone = deck[:]

        removed = {captured:found} # dictionary with all things to be removed from clone deck
        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        located = c.execute('''SELECT * from locations WHERE location != (?) ORDER BY timing DESC;''', ('None', )).fetchall()

        for prof in located:
            removed[prof[0]] = prof[1]
        
        for profs in clone:
            if profs in removed:
                clone.remove(profs)
        new = random.choice(clone)
        new_info = c.execute('''SELECT * from profedex WHERE prof = (?);''', (new, )).fetchall() # info about the new professor on deck
        possible = new_info[0][4].split(';')
        loc = random.choice(possible)

        c.execute('''UPDATE locations SET location = (?), timing = (?) WHERE prof = (?);''', ('None', datetime.datetime.now(), captured))
        c.execute('''UPDATE locations SET location = (?), timing = (?) WHERE prof = (?);''', (loc, datetime.datetime.now(), new))

        del removed[captured]
        removed[new] = loc
        prof_info = c.execute('''SELECT * from profedex WHERE prof = (?);''', (captured, )).fetchone()
        
        conn.commit()
        conn.close()
        return prof_info


    elif request['method'] == 'GET' and 'start' in request['args']:
        deck = 4*['Peter Dourmashkin'] + 9*['Donald Sadoway'] + 7*['Adam Willard'] + 5*['Brad Pentelute'] + 3*['Rick Danheiser'] + 8*['John Bush'] + 7*['Larry Guth'] + 5*['David Jerison'] + 3*['Bjorn Poonen'] + 1*['Tristan Collins'] + 9*['John Guttag'] + 6*['Adam Hartz'] + 3*['Max Goldman'] + 7*['Silvina Hinono Wachman'] + 5*['Katrina LaCurts'] + 3*['Steven Leeb'] + 1*['Joe Steinmeyer'] + 7*['Erik Demaine'] + 5*['Mauricio Karchmer'] + 2*['David Karger'] + 15*['David Perreault']
        clone = deck[:]
        random.shuffle(clone)
        located = []
        for i in range(5):
            prof = random.choice(clone)
            located.append(prof)
            for profs in deck:
                if prof == profs:
                    clone.remove(profs)
                    #i -= 1

        available = ['Peter Dourmashkin', 'Donald Sadoway', 'Adam Willard', 'Brad Pentelute', 'Rick Danheiser', 'John Bush', 'Larry Guth', 'David Jerison', 'Bjorn Poonen', 'Tristan Collins', 'John Guttag', 'Adam Hartz', 'Max Goldman', 'Silvina Hinono Wachman', 'Katrina LaCurts', 'Steven Leeb', 'Joe Steinmeyer', 'Erik Demaine', 'Mauricio Karchmer', 'David Karger', 'David Perreault']
        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()

        locations = {}
        for professor in located:
            info = c.execute('''SELECT * from profedex WHERE prof = (?);''', (professor, )).fetchall()
            #return (professor, info)
            possible = info[0][4].split(';')
            loc = random.choice(possible)
            locations[professor] = loc

        c.execute('''CREATE TABLE IF NOT EXISTS locations (prof text, location text, timing timestamp);''')
        for professor in available:
            in_table = c.execute('''SELECT * from locations WHERE prof = (?);''', (professor,)).fetchall()
            if len(in_table) == 0:
                c.execute('''INSERT into locations VALUES (?,?,?)''', (professor, 'None', datetime.datetime.now()))
            else:
                c.execute('''UPDATE locations SET location = (?), timing = (?) WHERE prof = (?);''', ('None', datetime.datetime.now(), professor))
        for professor in locations:
            c.execute('''UPDATE locations SET location = (?), timing = (?) WHERE prof = (?);''', (locations[professor], datetime.datetime.now(), professor))
        on_deck = c.execute('''SELECT * from locations WHERE location != (?);''', ('None', )).fetchall()
        
        conn.commit()
        conn.close()
        return on_deck

