from flask import Flask, request, jsonify
from datetime import datetime
import pytz

app = Flask(__name__)

tz = pytz.timezone('US/Eastern')

@app.route('/time', methods=['GET'])
def time():
    time=datetime.now(tz)
    return jsonify(hour = time.hour, minute = time.minute, second = time.second)

@app.route('/')
def home():
    html = "<h1>Please go to clock IP to manipulate the clock!"
    return html
