from flask import Flask, request, jsonify
from datetime import datetime
import pytz

app = Flask(__name__)

tz = pytz.timezone('US/Eastern')

ClockIP = "http://192.168.2.252"
PlexIP = "http://192.168.2.10:32400/web"

@app.route('/time', methods=['GET'])
def time():
    time=datetime.now(tz)
    return jsonify(hour = time.hour, minute = time.minute, second = time.second)

@app.route('/')
def home():
    html = "<h1>Please go to clock IP to manipulate the clock!</h1><form action = " + ClockIP + "><button type = \"submit\">Clock Server</button></form><h2>For Plex server click below</h2><form action = " + PlexIP + "><button type = \"submit\">Plex Server</button></form>"
    return html
