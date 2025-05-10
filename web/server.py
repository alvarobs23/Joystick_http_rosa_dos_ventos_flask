from flask import Flask, render_template, request
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

@app.route('/')
def index():
    return render_template('index.html')

# Rota para receber direção
@app.route('/compass', methods=['GET'])
def compass():
    direction = request.args.get('dir', default="Norte")
    print(f"Rosa dos ventos: {direction}")
    socketio.emit('compass', {'dir': direction})
    return 'OK', 200

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
