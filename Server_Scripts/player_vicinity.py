# -*- coding: utf-8 -*-
"""
Created on Wed Apr 20 18:45:25 2022

@author: jjche
"""
import math
import sqlite3
import string
import datetime
import time

database = "/var/jail/home/team3/player_locations.db"

locations = {
    '10-250' : {'center' : (42.35976089313539, -71.0920464184185),
                'vertices' : [(42.35953396639398, -71.09168163062562), (42.360004913651935, -71.09196068129873), (42.359864902673664, -71.09238442491343), (42.35937613463137, -71.09212776719559)]
                },
    '38-530' : {'center' : (42.36107848722357, -71.09238371354427),
                'vertices' : [(42.361112472062636, -71.09260742447664), (42.36126867343155, -71.09215156173612), (42.36116135144226, -71.0920868493583), (42.36097752226592, -71.09218319889652), (42.36087976317853, -71.09245499087747)]
                },
    '34-101' : {'center' : (42.36111525063663, -71.09195185430151),
                'vertices' : [(42.36119110409471, -71.09209979182839), (42.36127186121249, -71.09191284496319), (42.361194291877645, -71.09187401753734), (42.36115816366163, -71.09175609720698), (42.36106678043423, -71.09179923879125), (42.36101152539506, -71.09198187149804), (42.361041278114506, -71.09209979182839), (42.36113159878364, -71.09206096440253)]
                },
    '26-100' : {'center' : (42.36067105488153, -71.09079993860773),
                'vertices' : [(42.36070404681639, -71.09097980946427), (42.36081243218551, -71.09064618121252), (42.360639228025946, -71.09054264141024), (42.36052659193591, -71.090876269662)]
                },
    '26-152' : {'center' : (42.361083293121894, -71.09150337301179),
                'vertices' : [(42.36112422381327, -71.09166194033055), (42.36094251972543, -71.09154689610581), (42.361047716892955, -71.09119744927317), (42.361226232895454, -71.09130674128667)]
                },
    '32-123' : {'center' : (42.36169069237554, -71.09060049742999),
                'vertices' : [(42.361666794811384, -71.09145216513758), (42.361206764930216, -71.09078107254521), (42.36140790395066, -71.0899806127061), (42.36204517184286, -71.08998600300804), (42.36210093247588, -71.09048999624007)]
                },
    '2-190' : {'center' : (42.358451756471666, -71.09043443492067),
                'vertices' : [(42.35847744464004, -71.09056769443245), (42.358344722323736, -71.09048513147407), (42.358404661469024, -71.0903040723548), (42.35853738365876, -71.09039098073204)]
                },
    '8-203' : {'center' : (42.36001523447265, -71.09073984124865),
                'vertices' : [(42.35993340571311, -71.0911711447799), (42.3597912818248, -71.09108566119713), (42.35995781079289, -71.0905649884657), (42.36022195928529, -71.09072624158775), (42.36015017991354, -71.09094577897078), (42.36003820392987, -71.09088749470979  )]
                },
    'Simmons' : {'center' : (42.35711336193982, -71.10154279762585),
                'vertices' : [(42.356918989422866, -71.1023791023249), (42.35654512256856, -71.10200715117462), (42.35723817834913, -71.10052534575965), (42.35764307306465, -71.10092929270779)]
                },
    '3-208' : {'center' : (42.359031465100784, -71.0924372254359),   
                'vertices' : [(42.359173426676676, -71.09269272001445), (42.35923928503078, -71.09248475931099), (42.358967070053424, -71.0925699241705), (42.359005121680305, -71.09234413826387)]
                }, 
    '35-225' : {'center' : (42.36040223933926, -71.0940402795447),
                'vertices' : [(42.36036725755652, -71.0938015127905), (42.360286647287325, -71.09401352120156), (42.360429616373054, -71.0941061462355), (42.36052543589734, -71.09390854616308)]
                },
    '4-409' : {'center' : (42.35947224986276, -71.09108376127318),
                'vertices' : [(42.35952297604812, -71.091199318726), (42.359547493689696, -71.09094417702326), (42.359440968694706, -71.09087896143107), (42.359366570178416, -71.09110206740432)]
                }
    }

def within_area(point_coord,poly):
    def sign(x):
        if x > 0:
            return 1
        elif x == 0:
            return 0
        else:
            return -1
        
    new_list = []
    counter = 0
    for point in poly:
        new_list.append((point[0]-point_coord[0],point[1]-point_coord[1]))
    
    for i in range(len(new_list)-1):
        if sign(new_list[i][1]) >= 0 and sign(new_list[i+1][1]) >= 0:
            continue
        elif sign(new_list[i][1]) == -1 and sign(new_list[i+1][1]) == -1:
            continue
        else:
            if sign(new_list[i][0]) >= 0 and sign(new_list[i+1][0]) >= 0:
                counter += 1
            else:
                p = (new_list[i][0]*new_list[i+1][1]-new_list[i][1]*new_list[i+1][0])/(new_list[i+1][1]-new_list[i][1])
                if sign(p) == 1:
                    counter += 1
                    
    if sign(new_list[0][1]) >= 0 and sign(new_list[-1][1]) >= 0:
        counter += 0
    elif sign(new_list[-1][1]) == -1 and sign(new_list[0][1]) == -1:
        counter += 0
    else:
        if sign(new_list[0][0]) >= 0 and sign(new_list[-1][0]) >= 0:
            counter += 1
        else:
            p = (new_list[-1][0]*new_list[0][1]-new_list[-1][1]*new_list[0][0])/(new_list[0][1]-new_list[-1][1])
            if sign(p) == 1:
                counter += 1
                
    if counter % 2 == 0:
        return False
    
    return True
            
def get_area(point_coord,locations): 
    for name in locations:
        if (within_area(point_coord, locations[name]['vertices'])):
            return name
    return "Off Campus"      
  
def distance(p1, p2):
    re = 6371.0088
    angle_ratio = math.pi/180

    dlat = abs(p2[0]-p1[0])*re*angle_ratio

    dlon = abs(p2[1]-p1[1])*angle_ratio*re*abs(math.cos(max((p1[0],p2[0]))*angle_ratio))
    
    d = math.sqrt(dlat*dlat + dlon*dlon)
    
    return d

def request_handler(request):
    lat = float(request['form']['lat'])
    lon = float(request['form']['lon'])
    user = request['form']['user']
    
    point = (lat, lon)
    
    conn = sqlite3.connect(database)
    c = conn.cursor()
    
    mit_loc = get_area(point, locations)
    
    c.execute('''CREATE TABLE IF NOT EXISTS location_table (user text, lat real, lon real, MIT_loc text, timing timestamp);''') 
    c.execute('''INSERT into location_table VALUES (?,?,?,?,?);''',(user, lat, lon, mit_loc, datetime.datetime.now())) 
    thirty_sec_ago = datetime.datetime.now()- datetime.timedelta(seconds = 30) 
    
    close_users = c.execute('''SELECT DISTINCT user, lat, lon FROM location_table WHERE timing > ? AND MIT_loc = ?''',(thirty_sec_ago, mit_loc)).fetchall()
    
    conn.commit()
    conn.close()
    
    closest_user = ""
    closest_distance = 100000000000000000000000
    
    if close_users:
        for user1 in close_users:
            user_name = user1[0]
            user_lat = user1[1]                 
            user_lon= user1[2]
            
            user_point = (user_lat, user_lon)
            
            test_distance = distance(point, user_point)
            
            if test_distance < closest_distance and user_name != user:
                closest_user = user_name
                closest_distance = test_distance
        
    return closest_user