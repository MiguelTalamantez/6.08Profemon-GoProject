import sqlite3
import datetime
# import requests
from bs4 import BeautifulSoup

############################################
# Code for editing/accessing full profedex #
############################################

def request_handler(request):
    # edited to get specific items if requested

    if request['method'] == 'GET':
        if 'professor' not in request['values']:
            return 'Please enter a query.'

        item = None
        if 'item' in request['values']:
            item = request['values']['item']
        
        professor = request['values']['professor']

        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS profedex (prof text, type int, course float, evolutions text, locations text, 
            move1 text, move2 text, attack int, hp int, timing timestamp);''')
        if item == "type":
            info = c.execute('''SELECT type FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
        elif item == "course":
            info = c.execute('''SELECT course FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
        elif item == "attack":
            info = c.execute('''SELECT attack FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
        elif item == "hp":
            info = c.execute('''SELECT hp FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
        elif item == "moves":
            info = c.execute('''SELECT move1, move2 FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
            info = [','.join(info)]
        else:
            info = c.execute('''SELECT * FROM profedex WHERE prof = (?) ORDER BY timing ASC;''', (professor,)).fetchone()
        conn.commit()
        conn.close()
        return info[0]

    elif request['method'] == 'POST' and 'professor' in request['values']:
        professor = request['values']['professor'] if 'professor' in request['values'] else 'n/a'
        dept = int(request['values']['dept']) if 'dept' in request['values'] else 0
        course = request['values']['course'] if 'course' in request['values'] else 'n/a'
        evol = request['values']['evol'] if 'evol' in request['values'] else 'n/a'
        loc = request['values']['loc'] if 'loc' in request['values'] else 'n/a'
        move1 = request['values']['move1'] if 'move1' in request['values'] else 'n/a'
        move2 = request['values']['move2'] if 'move2' in request['values'] else 'n/a'
        attack = int(request['values']['attack']) if 'attack' in request['values'] else 0
        hp = int(request['values']['hp']) if 'hp' in request['values'] else 0

        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS profedex (prof text, type int, course float, evolutions text, locations text, 
            move1 text, move2 text, attack int, hp int, timing timestamp);''')

        c.execute('''INSERT into profedex VALUES (?,?,?,?,?,?,?,?,?,?);''', (professor, dept, course, evol, loc, move1, 
        move2, attack, hp, datetime.datetime.now()))

        data = c.execute('''SELECT * FROM profedex ORDER BY timing ASC;''').fetchall()
        conn.commit()
        conn.close()
        return data



    elif request['method'] == 'POST':
        professor = ['Peter Dourmashkin', 'Donald Sadoway', 'Adam Willard', 'Brad Pentelute', 'Rick Danheiser', 'John Bush', 'Larry Guth', 'David Jerison', 'Bjorn Poonen', 'Tristan Collins', 'John Guttag', 'Adam Hartz', 'Max Goldman', 'Silvina Hinono Wachman', 'Katrina LaCurts', 'Steven Leeb', 'Joe Steinmeyer', 'Erik Demaine', 'Mauricio Karchmer', 'David Karger']
        dept = [8, 3, 5, 5, 5, 18 ,18 ,18, 18, 18, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6]
        course = ['8.01/2', '3.091', '5.111', '5.111', '5.12', '18.02A', '18.01', '18.01A', '18.02', '18.03', '6.0001', '6.009', '6.031', '6.004', '6.033', '6.115', '6.08', '6.042', '6.006', '6.854']
        evol = ['n/a', 'Adam Willard', 'Brad Pentelute', 'Rick Danheiser', 'n/a', 'Larry Guth', 'David Jerison', 'Bjorn Poonen', 'Tristan Collins', 'n/a', 'Adam Hartz', 'Max Goldman', 'Joe Steinmeyer', 'Katrina LaCurts', 'Steven Leeb', 'Joe Steinmeyer', 'n/a', 'Mauricio Karchmer', 'David Karger', 'n/a']
        loc = ['26-100;Bld4', '10-250;Bld13', '2-131;32-123;Bld2;Bld36;Bld56', '32-123;Bld36;Bld56', '2-105;Bld4', '26-100', '2-135;Bld2', '26-100;2-131;Bld2', '32-123;2-142;Bld2', '26-100;Bld2', '26-100', '26-100;34-501', '34-101;34-501;Bld36;Bld38', '32-123;Bld8;Bld34;Bld35', '26-100;26-142;26-314;35-308;Bld4;Bld5;Bld24;Bld34;Bld36;Bld38', '34-101;Bld36', '10-250;38-530;Bld34', '34-101', '26-100;Bld34;Bld36', '34-101;Bld34;Bld36']
        move1 = ['The Force', 'Not to Scale', 'Chemistry History', 'Chemistry Trivia', 'Chemistry Jokes', 'Carbon Dioxide', 'U-Substitution', '2-Day Derivatives', 'Matrices', 'Linear Algebra', 'CS Memes', 'Environment Diagrams', 'Style', '', '', '', 'Arduino', 'High School Prodigy', '', 'Stuffed Animals']
        move2 = ['Electric Strike', 'Celebration of Learning', 'Awkward Smile', 'Suit and Tie', 'Rhino', 'Evil Laughter', 'Taylor Series', 'Riemann Sums', 'Jacobian Determinant', 'Fish', 'Archery', 'Recursion', 'Test Cases', '', '', '', 'POINTERS', 'Sarcasm', '', 'Induction']
        attack = [25, 1, 3, 10, 30, 0, 3, 8, 24, 60, 2, 8, 30, 4, 15, 40, 100, 3, 12, 35]
        hp = [75, 12, 25, 50, 100, 12, 25, 50, 100, 200, 12, 30, 80, 18, 40, 80, 250, 20, 50, 120]

        conn = sqlite3.connect('/var/jail/home/team3/prof_info.db')
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS profedex (prof text, type int, course float, evolutions text, locations text, 
            move1 text, move2 text, attack int, hp int, timing timestamp);''')

        for index in range(20):
            c.execute('''INSERT into profedex VALUES (?,?,?,?,?,?,?,?,?,?);''', (professor[index], dept[index], course[index], evol[index], loc[index], move1[index], 
            move2[index], attack[index], hp[index], datetime.datetime.now()))

        data = c.execute('''SELECT * FROM profedex ORDER BY timing ASC;''').fetchall()
        conn.commit()
        conn.close()
        return data

    

