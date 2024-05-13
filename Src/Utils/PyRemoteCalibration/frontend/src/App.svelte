<script>
    import ServerControl from './ServerControl.svelte';
    import CameraSettings from './CameraSettings.svelte';

    let statusMessage = "Not Connected";
    let cameraSettings = {
        autoExposure: 1,                 // true (1 for checked)
        autoExposureBrightness: 0,
        exposure: 2000,
        gain: 160,
        autoWhiteBalance: 1,             // true (1 for checked)
        autoFocus: 0,                    // false (0 for unchecked)
        focus: 0,
        autoHue: 0,                      // false (0 for unchecked)
        hue: 0,
        saturation: 80,
        contrast: 32,
        sharpness: 4,
        redGain: 2048,
        greenGain: 2048,
        blueGain: 2048
    };

    async function startConnection() {
        try {
            const response = await fetch('http://localhost:5000/start', {
                method: 'POST',
            });
            const result = await response.json();
            if (result.clients.length > 0) {
                statusMessage = `Connected to ${result.clients.map(client => client[0]).join(', ')}`;
            } else {
                statusMessage = "Connected but no clients";
            }
        } catch (error) {
            console.error('Error:', error);
            alert('Failed to start connection');
        }
    }

    async function stopConnection() {
        try {
            const response = await fetch('http://localhost:5000/stop', { method: 'POST' });
            const result = await response.json();
            statusMessage = `Disconnected`;
        } catch (error) {
            console.error('Error:', error);
            alert('Failed to stop connection');
        }
    }

    async function updateSetting(setting, value) {
        cameraSettings[setting] = value;
        try {
            const response = await fetch('http://localhost:5000/update_camera_setting', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ setting, value: parseInt(value) }),
            });
            const result = await response.json();
            console.log(result.message);
        } catch (error) {
            console.error('Error:', error);
        }
    }
</script>


<style>
    body {
        text-align: center;
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
    }

    h1 {
        margin-top: 20px;
        font-size: 28px;
    }

    .header {
        display: flex;
        flex-direction: column;
        align-items: center;
    }

    .logo {
        width: 150px;
        height: auto;
        margin-top: 20px;
    }

    .main-container {
        display: flex;
        justify-content: center;
        align-items: flex-start;
        gap: 20px;
        margin-top: 20px;
    }

    .container {
        display: flex;
        flex-direction: column;
        align-items: center;
        padding: 20px;
        border-radius: 8px;
    }

    .setting {
        margin: 10px;
    }
</style>

<div class="header">
    <img src="r2k.png" alt="R-ZWEI Kickers Logo" class="logo">
    <h1>Robot Control Panel</h1>
</div>
<div class="main-container">
    <div class="container">
        <ServerControl
            {statusMessage}
            startServer={startConnection}
            stopServer={stopConnection} />
    </div>
    <div class="container">
        <CameraSettings
            {cameraSettings}
            updateSetting={updateSetting} />
    </div>
</div>
