import sqlite3
import datetime
import random

available = ['Peter Dourmashkin', 'Donald Sadoway', 'Adam Willard', 'Brad Pentelute', 'Rick Danheiser', 'John Bush', 'Larry Guth', 'David Jerison', 'Bjorn Poonen', 'Tristan Collins', 'John Guttag', 'Adam Hartz', 'Max Goldman', 'Silvina Hinono Wachman', 'Katrina LaCurts', 'Steven Leeb', 'Joe Steinmeyer', 'Erik Demaine', 'Mauricio Karchmer', 'David Karger']
clone = available[:]
game_id = 1

def request_handler(request):

    # start a new game
    if request['method'] == 'GET':

        player = request['values']['user']
        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        
        prof1 = random.choice(available) ## later choose from player's profedex 
        clone.remove(prof1) 
        prof2 = random.choice(clone) ## random choice for CPU
        stats1 = c.execute('''SELECT * from profedex WHERE prof = (?);''', (prof1, )).fetchone()
        stats2 = c.execute('''SELECT * from profedex WHERE prof = (?);''', (prof2, )).fetchone()
        attack1, health1 = stats1[7], stats1[8]
        attack2, health2 = stats2[7], stats2[8]

        c.execute('''CREATE TABLE IF NOT EXISTS battling (player int, prof text, attack int, hp int, timing timestamp);''')
        c.execute('''INSERT into battling VALUES (?,?,?,?,?);''', (player, prof1, attack1, health1, datetime.datetime.now()))
        # game_id += 1
        c.execute('''INSERT into battling VALUES (?,?,?,?,?);''', ('CPU', prof2, attack2, health2, datetime.datetime.now()))
        # game_id += 1

        conn.commit()
        conn.close()
        return 'Player({}, Attack: {}, HP: {}) vs. ({}, Attack: {}, HP: {})'.format(prof1, attack1, health1, prof2, attack2, health2)
    

    elif request['method'] == 'POST':

        player = request['values']['user']
        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()

        player_stats = c.execute('''SELECT * from battling WHERE player = (?) ORDER BY timing DESC;''', (player, )).fetchone()
        cpu_stats = c.execute('''SELECT * from battling WHERE player = (?) ORDER BY timing DESC;''', ('CPU', )).fetchone()
        p_attack, p_hp, p_time = player_stats[2], player_stats[3], player_stats[4]
        c_attack, c_hp, c_time = cpu_stats[2], cpu_stats[3], cpu_stats[4]
        
        # player attacks
        newc_hp = c_hp - p_attack
        if newc_hp < 0:
            newc_hp = 0
        c.execute('''UPDATE battling SET hp = (?) WHERE (player = (?) AND timing = (?));''', (newc_hp, 'CPU', c_time))
        c.execute('''UPDATE battling SET timing = (?) WHERE (player = (?) AND hp = (?));''', (datetime.datetime.now(), 'CPU', newc_hp))

        if newc_hp == 0:
            conn.commit()
            conn.close()
            return 'Victory!'

        # computer attacks        
        newp_hp = p_hp - c_attack
        if newp_hp < 0:
            newp_hp = 0
        c.execute('''UPDATE battling SET hp = (?) WHERE (player = (?) AND timing = (?));''', (newp_hp, player, p_time))
        c.execute('''UPDATE battling SET timing = (?) WHERE (player = (?) AND hp = (?));''', (datetime.datetime.now(), player, newp_hp))

        if newp_hp == 0:
            conn.commit()
            conn.close()
            return 'Defeat!'
        
        conn.commit()
        conn.close()
        return '{} HP: {}, CPU HP: {}'.format(player, newp_hp, newc_hp)
