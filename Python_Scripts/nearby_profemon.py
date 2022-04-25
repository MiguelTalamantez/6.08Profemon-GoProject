import requests
import random

def request_handler(request):
    if request['method']=='GET' :
        lat = float(request['values']['lat'])
        lon = float(request['values']['lon'])
        

        endpoint = "http://608dev-2.net/sandbox/sc/team3/profemon_geolocation.py"
        data = "lat={}&lon={}".format(lat, lon)
        headers = {'Content-Type': 'application/x-www-form-urlencoded', 'Content-Length':str(len(data))}
        location = requests.post(endpoint, data=data, headers=headers).text[11:-2]

        active_profemon = requests.get('http://608dev-2.net/sandbox/sc/team3/updating_locations.py').text[1:-2]
        active_profemon = active_profemon.split(')')
        for i, prof in enumerate(active_profemon):
            start = prof.find('(')
            new_prof = prof
            if start != -1:
                new_prof = prof[start+1:]
            active_profemon[i] = new_prof.replace("'", "").split(', ')
        
        in_loc = []
        for prof in active_profemon:
            if len(prof) == 3 and prof[1] == location:
                in_loc.append(prof[0])

        if len(in_loc) > 0:
            which_prof = random.randrange(len(in_loc))
            return in_loc[which_prof] + ',' + in_loc[which_prof].upper()
        return ''
    return "Must be a GET Request"