#ifndef INDEX_H
#define INDEX_H

const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
    <title>Water Stress Monitor - WiFi Setup</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }
        
        .header p {
            font-size: 14px;
            opacity: 0.9;
        }
        
        .content {
            padding: 30px;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 600;
            font-size: 14px;
        }
        
        input {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 16px;
            transition: all 0.3s;
        }
        
        input:focus {
            outline: none;
            border-color: #667eea;
        }
        
        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s;
        }
        
        button:hover {
            transform: translateY(-2px);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .status {
            margin-top: 20px;
            padding: 15px;
            border-radius: 10px;
            background: #f5f5f5;
            font-size: 14px;
        }
        
        .status h3 {
            margin-bottom: 10px;
            color: #333;
            font-size: 16px;
        }
        
        .status-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #e0e0e0;
        }
        
        .status-item:last-child {
            border-bottom: none;
        }
        
        .status-label {
            font-weight: 600;
            color: #666;
        }
        
        .status-value {
            color: #333;
        }
        
        .connected {
            color: #10b981;
            font-weight: 600;
        }
        
        .disconnected {
            color: #ef4444;
            font-weight: 600;
        }
        
        .info {
            background: #e0f2fe;
            border-left: 4px solid #0284c7;
            padding: 12px;
            margin-top: 20px;
            border-radius: 8px;
            font-size: 12px;
            color: #0c4a6e;
        }
        
        @media (max-width: 600px) {
            .container {
                margin: 10px;
            }
            
            .header {
                padding: 20px;
            }
            
            .content {
                padding: 20px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>💧 Water Stress Monitor</h1>
            <p>WiFi Configuration</p>
        </div>
        
        <div class="content">
            <form id="wifiForm">
                <div class="form-group">
                    <label>WiFi SSID</label>
                    <input type="text" id="ssid" name="ssid" required placeholder="Enter WiFi name">
                </div>
                
                <div class="form-group">
                    <label>WiFi Password</label>
                    <input type="password" id="password" name="password" required placeholder="Enter WiFi password">
                </div>
                
                <button type="submit">Save & Connect</button>
            </form>
            
            <div class="status">
                <h3>📡 Device Status</h3>
                <div class="status-item">
                    <span class="status-label">WiFi Configured:</span>
                    <span class="status-value" id="wifiConfigured">Checking...</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Connection Status:</span>
                    <span class="status-value" id="wifiStatus">Checking...</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Connected to:</span>
                    <span class="status-value" id="ssidDisplay">-</span>
                </div>
                <div class="status-item">
                    <span class="status-label">IP Address:</span>
                    <span class="status-value" id="ipAddress">-</span>
                </div>
            </div>
            
            <div class="info">
                💡 <strong>Setup Instructions:</strong><br>
                1. Enter your WiFi credentials above<br>
                2. Click "Save & Connect"<br>
                3. Device will restart and connect to your WiFi<br>
                4. After successful connection, you can use the device normally
            </div>
        </div>
    </div>
    
    <script>
        // Load status on page load
        async function loadStatus() {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                
                document.getElementById('wifiConfigured').innerHTML = data.wifi_configured ? '✓ Configured' : '✗ Not configured';
                document.getElementById('wifiConfigured').className = data.wifi_configured ? 'connected' : 'disconnected';
                
                const statusText = data.wifi_connected ? 'Connected' : 'Disconnected';
                document.getElementById('wifiStatus').innerHTML = data.wifi_connected ? '✓ ' + statusText : '✗ ' + statusText;
                document.getElementById('wifiStatus').className = data.wifi_connected ? 'connected' : 'disconnected';
                
                document.getElementById('ssidDisplay').textContent = data.ssid || '-';
                document.getElementById('ipAddress').textContent = data.ip || '-';
            } catch (error) {
                console.error('Error loading status:', error);
            }
        }
        
        // Handle form submission
        document.getElementById('wifiForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            const formData = new URLSearchParams();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            const button = document.querySelector('button');
            const originalText = button.textContent;
            button.textContent = 'Saving...';
            button.disabled = true;
            
            try {
                const response = await fetch('/save', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: formData
                });
                
                const html = await response.text();
                document.body.innerHTML = html;
            } catch (error) {
                alert('Error saving credentials. Please try again.');
                button.textContent = originalText;
                button.disabled = false;
            }
        });
        
        // Load status every 3 seconds
        loadStatus();
        setInterval(loadStatus, 3000);
    </script>
</body>
</html>
)rawliteral";

#endif