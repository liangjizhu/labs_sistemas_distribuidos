# datetime_service.py
from flask import Flask, Response
from datetime import datetime

app = Flask(__name__)

@app.route('/datetime')
def current_datetime():
    # Formato 17/12/2014 13:20:35
    ts = datetime.now().strftime('%d/%m/%Y %H:%M:%S')
    return Response(ts, mimetype='text/plain')

if __name__ == '__main__':
    # Se ejecuta en localhost:5000
    app.run(host='127.0.0.1', port=5000)
