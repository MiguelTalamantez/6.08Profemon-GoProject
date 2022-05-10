import sqlite3
import datetime
import random
import requests
db = '/var/jail/home/team3/prof_info.db'

def request_handler(request):
    if request['method'] == 'GET' :
        #label "turn". info the arduino provides: game_id, player_id
        if request['values']['label']=='turn' :
            with sqlite3.connect(db) as c:
                #first we need the game id
                game_id = int(request['values']['game_id'])
                
                c.execute('''CREATE TABLE IF NOT EXISTS game_list (game_id int, player_id text, prof text, hp int, timing timestamp);''')
                player_list = listify(c.execute('''SELECT player_id FROM game_list WHERE game_id = ? LIMIT 2;''',(game_id,)).fetchall())
            
                #if both players still haven't entered the game, we can't do anything
                if len(player_list)<2 :
                    return -1
                
                #next, we check who did the last attack                
                c.execute('''CREATE TABLE IF NOT EXISTS attack_log (game_id int, player_id text, move text, attack int, timing timestamp);''')
                last_move = listify(c.execute('''SELECT player_id FROM attack_log WHERE game_id = ? ORDER BY timing DESC LIMIT 1;''', (game_id,)).fetchall())
                
                #if there was no last attack, we're still at the beginning of the game
                if len(last_move) == 0 :
                    #randomly choose the player who WON'T be first and assign them as the ""last"" move (that did no damage)
                    second_player = random.choice(player_list)
                    c.execute('''INSERT into attack_log VALUES (?,?,?,?,?);''',(game_id,second_player,'',0,datetime.datetime.now()))
                    last_move = listify(c.execute('''SELECT player_id FROM attack_log WHERE game_id = ? ORDER BY timing DESC LIMIT 1;''', (game_id,)).fetchall())
                
                #so now we need to return all relevant info -- whose turn it is, hp of both players, and name of last move
                last_player = last_move[0]
                current_player = other(player_list,last_player)
                
                c.execute('''CREATE TABLE IF NOT EXISTS hp_log (game_id int, player_id text, hp int, timing timestamp);''')
                hp_of_requester = listify(c.execute('''SELECT hp FROM hp_log WHERE game_id = ? AND player_id = ? ORDER BY timing DESC LIMIT 1;''',(game_id,request['values']['player_id'])).fetchall())[0]
                hp_of_other = listify(c.execute('''SELECT hp FROM hp_log WHERE game_id = ? AND player_id = ? ORDER BY timing DESC LIMIT 1;''',(game_id,other(player_list,request['values']['player_id']))).fetchall())[0]
                
                move = listify(c.execute('''SELECT move FROM attack_log WHERE game_id = ? ORDER BY timing DESC LIMIT 1;''', (game_id,)).fetchall())[0]
                
                #info returned is: current player, hp of person who made request, hp of other person, and name of last move
                return '''{},{},{},{}'''.format(current_player,hp_of_requester,hp_of_other,move)
  
        
    elif request['method'] == 'POST' :
        #label "start". info the arduino provides: its own player id, and the prof it's using
        if request['values']['label']=='start' :
            with sqlite3.connect(db) as c:
                c.execute('''CREATE TABLE IF NOT EXISTS game_list (game_id int, player_id text, prof text, hp int, timing timestamp);''')
                prevgame = listify(c.execute('''SELECT game_id FROM game_list ORDER BY timing DESC LIMIT 2;''').fetchall())
                new_id = 0
                if len(prevgame)==1 :
                    new_id = int(prevgame[0])
                elif len(prevgame)>1 and len(prevgame)%2==0 :
                    new_id = int(prevgame[1])+1
                elif len(prevgame)>1 and len(prevgame)%2==1 :
                    new_id = int(prevgame[1])
                
                hp = int(requests.get("http://608dev-2.net/sandbox/sc/team3/profedex.py?professor={}&item={}".format(request['values']['prof_id'],'hp')).text)
                
                c.execute('''INSERT into game_list VALUES (?,?,?,?,?);''',(new_id,request['values']['player_id'],request['values']['prof_id'],hp,datetime.datetime.now()))
                
                c.execute('''CREATE TABLE IF NOT EXISTS hp_log (game_id int, player_id text, hp int, timing timestamp);''')
                c.execute('''INSERT into hp_log VALUES (?,?,?,?);''',(new_id,request['values']['player_id'],hp,datetime.datetime.now()))
                
                gesture = random.randint(0,2)
                
                #info returned is: game_id, which gesture will be used, the user's prof's hp
                return "{},{},{}".format(new_id, gesture, hp)
            
        elif request['values']['label']=='attack' :
            #label "attack". info the arduino provides: game_id, its own player id, the move that happened, and how much damage the attack did
            with sqlite3.connect(db) as c:
                #first we need the game id
                game_id = int(request['values']['game_id'])
                
                c.execute('''CREATE TABLE IF NOT EXISTS game_list (game_id int, player_id text, prof text, hp int, timing timestamp);''')
                player_list = listify(c.execute('''SELECT player_id FROM game_list WHERE game_id = ? ORDER BY timing DESC LIMIT 2;''',(game_id,)).fetchall())
            
                #if both players still haven't entered the game, we can't do anything
                if len(player_list)<2 :
                    return -1
                
                damage = int(request['values']['damage'])
                
                c.execute('''CREATE TABLE IF NOT EXISTS attack_log (game_id int, player_id text, move text, attack int, timing timestamp);''')
                c.execute('''INSERT into attack_log VALUES (?,?,?,?,?);''',(game_id,request['values']['player_id'],request['values']['move'],damage,datetime.datetime.now()))
                
                other_player = other(player_list,request['values']['player_id'])
                old_hp = int(listify(c.execute('''SELECT hp FROM hp_log WHERE game_id = ? AND player_id = ? ORDER BY timing DESC LIMIT 1;''',(game_id,other_player)).fetchall())[0])
                
                c.execute('''INSERT into hp_log VALUES (?,?,?,?)''',(game_id,other_player,max(0,old_hp-damage),datetime.datetime.now()))
                
                return '''attack successfully inputted. old hp value was {} and new one is {}'''.format(old_hp,old_hp-damage)
            
        elif request['values']['label']=='forfeit' :
             #label "forfeit". info the arduino provides: game id, its own player id
             with sqlite3.connect(db) as c:
                #first we need the game id
                game_id = int(request['values']['game_id'])
                
                c.execute('''CREATE TABLE IF NOT EXISTS game_list (game_id int, player_id text, prof text, hp int, timing timestamp);''')
                player_list = listify(c.execute('''SELECT player_id FROM game_list WHERE game_id = ? ORDER BY timing DESC LIMIT 2;''',(game_id,)).fetchall())
            
                #if both players still haven't entered the game, we can't do anything
                if len(player_list)<2 :
                    return -1
                
                player = request['values']['player_id']
                c.execute('''INSERT into hp_log VALUES (?,?,?,?)''',(game_id,player,0,datetime.datetime.now()))
                
                c.execute('''CREATE TABLE IF NOT EXISTS attack_log (game_id int, player_id text, move text, attack int, timing timestamp);''')
                c.execute('''INSERT into attack_log VALUES (?,?,?,?,?);''',(game_id,player,'',0,datetime.datetime.now()))
                
                return '''you have lost the game :('''
                
def other(players, cur) :
    copy = players[:]
    copy.remove(cur)
    return copy[0]

def listify(fuckeduplist) :
    unfuckedlist = []
    for fuck in fuckeduplist :
        unfuckedlist.append(fuck[0])
        
    return unfuckedlist