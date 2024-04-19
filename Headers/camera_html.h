const char* camera_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Video Streaming Demonstration</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            height: 100vh;
            background: #999;
            margin: 0;
            overflow: hidden;
        }
        img {
            max-width: 100%; /* Ensures the image is never wider than the viewport */
            height: auto; /* Maintains the aspect ratio */
            transform: rotate(180deg); /* Rotates the image to fit design */
            margin-bottom: 20px; /* Adds space between the stream and the controls */
        }
    </style>
</head>
<body>
    <h1>Video Streaming Demonstration</h1>
    <img id="stream" src="">
    <script>
    document.addEventListener('DOMContentLoaded', function (event) {
        var baseHost = document.location.origin;
        // var streamUrl = baseHost + ':/stream'; // Ensure this port is correct and open
        var streamUrl = baseHost + '/stream'; // Correct
        const view = document.getElementById('stream');
        view.src = streamUrl;
    });
    </script>
</body>
</html>
)rawliteral";
