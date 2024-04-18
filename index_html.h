const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Car Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            height: 100vh;
            background: #999;
        }
        .control-panel {
            display: grid;
            grid-template-columns: repeat(3, 100px);
            grid-gap: 10px;
            justify-content: center;
        }
        button {
            padding: 10px;
            border: none;
            border-radius: 5px;
            background-color: #4CAF50;
            color: white;
            font-size: 16px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }
        button:hover {
            background-color: #45a049;
        }
        .special-buttons {
            grid-column: span 3;
            justify-content: center;
            display: flex;
            gap: 10px;
        }
        #bboxDisplay {
            margin-top: 20px;
            padding: 15px;
            background-color: #fff;
            color: #333;
            border-radius: 5px;
            width: 90%;
            text-align: center;
        }
    </style>
</head>
<body>
    <h1>ESP32 Car Control</h1>
    <p>Use the buttons below or keyboard arrows to control the car.</p>
    <div class="control-panel">
        <button onclick="sendCommand('1')">IN Trace Path 1</button>
        <button onclick="sendCommand('2')">IN Trace Path 2</button>
        <button onclick="sendCommand('3')">IN Trace Path 3</button>
        <button onclick="sendCommand('4')">OUT Trace Path 1</button>
        <button onclick="sendCommand('5')">OUT Trace Path 2</button>
        <button onclick="sendCommand('6')">OUT Trace Path 3</button>
        <button onclick="sendCommand('0')">END OPERATION</button>
        <button onclick="sendCommand('forward')">FORWARD</button>
        <button onclick="sendCommand('backward')">BACKWARD</button>
        <button onclick="sendCommand('right')">RIGHT</button>
        <button onclick="sendCommand('left')">LEFT</button>
        <button onclick="sendCommand('stop')" class="special-buttons">Stop</button>
        <button onclick="sendCommand('reset')" class="special-buttons">RESET</button>
        <button onclick="sendCommand('docking')" class="special-buttons">DOCKING</button>
    </div>
    <div id="bboxDisplay">Bounding Box Data will be displayed here</div>
    <script>
        document.body.addEventListener('keydown', function(e) {
            switch(e.key) {
                case 'w': sendCommand('forward'); break;
                case 's': sendCommand('backward'); break;
                case 'a': sendCommand('left'); break;
                case 'd': sendCommand('right'); break;
                case ' ': sendCommand('stop'); break; // Spacebar to stop
                case '1': sendCommand('1'); break;
                case '2': sendCommand('2'); break;
                case '3': sendCommand('3'); break;
                case '4': sendCommand('4'); break;
                case '5': sendCommand('5'); break;
                case '6': sendCommand('6'); break;
                case '0': sendCommand('0'); break;
                case '#': sendCommand('reset'); break;
            }
        });
              function sendCommand(command) {
    const url = `/command?act=${encodeURIComponent(command)}`;
    console.log('Sending command to:', url);  // Check the URL in the browser console
    fetch(url)
        .then(response => {
            if (response.ok) {
                return response.text();
            } else {
                throw new Error('Failed to send command with status: ' + response.status);
            }
        })
        .then(data => {
            console.log('Response:', data);  // Display the server response
        })
        .catch(error => {
            console.error('Fetch error:', error);  // Capture any fetch errors
        });
}


        // Function to fetch the camera detection output
        function fetchCameraOutput() {
            fetch('/camera-output')
                .then(response => response.text())
                .then(text => {
                    document.getElementById('bboxDisplay').textContent = text;
                })
                .catch(error => console.error('Error fetching camera output:', error));
        }

        // Refresh the output every 2 seconds
        setInterval(fetchCameraOutput, 2000);
    </script>
</body>
</html>
)rawliteral";
