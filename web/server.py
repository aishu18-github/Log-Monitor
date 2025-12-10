# server.py
from flask import Flask, jsonify, send_from_directory, Response
import os
import time

app = Flask(__name__, static_folder='.')

# absolute path to project-root/logs/monitor.log
# Correct absolute path (web/ → project-root → logs/)
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LOG_FILE = os.path.join(PROJECT_ROOT, "logs", "monitor.log")


@app.route("/")
def index():
    return send_from_directory('.', "index.html")

@app.route("/logs")
def get_logs():
    if not os.path.exists(LOG_FILE):
        return "No logs yet..."
    with open(LOG_FILE, "r") as f:
        logs = f.read()
    return Response(logs, mimetype="text/plain")

@app.route("/stats")
def get_stats():
    # CPU Usage: sample /proc/stat twice for delta
    def read_cpu():
        with open("/proc/stat", "r") as f:
            fields = f.readline().strip().split()[1:]
            fields = list(map(int, fields))
        idle = fields[3]
        total = sum(fields)
        return idle, total

    idle1, total1 = read_cpu()
    time.sleep(0.1)
    idle2, total2 = read_cpu()
    cpu_usage = (1 - (idle2 - idle1) / (total2 - total1)) * 100

    # Memory
    mem_total = mem_available = 1
    with open("/proc/meminfo", "r") as f:
        for line in f:
            if line.startswith("MemTotal"):
                mem_total = int(line.split()[1])
            elif line.startswith("MemAvailable"):
                mem_available = int(line.split()[1])
    mem_usage = ((mem_total - mem_available) / mem_total) * 100

    return jsonify({
        "cpu": round(cpu_usage, 1),
        "mem": int(mem_usage)
    })


if __name__ == "__main__":
    print("✅ Flask Server Running at http://localhost:5000. Expected Log Path:", LOG_FILE)
    app.run(debug=True, host="0.0.0.0", port=5000, use_reloader=False)
