<script>
    import ServerControl from './ServerControl.svelte';
    import CameraSettings from './CameraSettings.svelte';

    let statusMessage = "Not Connected";
    let contrastValue = 25;

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

    async function updateContrast(event) {
        contrastValue = parseInt(event.target.value, 10);
        try {
            const response = await fetch('http://localhost:5000/update_contrast', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ contrast: contrastValue }),
            });
            const result = await response.json();
            console.log(result.message);
        } catch (error) {
            console.error('Error:', error);
        }
    }
</script>

<style>

    h1 {
        margin-top: 20px;
        font-size: 28px;
    }

    .container {
        display: flex;
        flex-direction: column;
        align-items: center;
        margin-top: 20px;
    }
</style>

<div class="container">
    <h1>Robot Control Panel</h1>
    <ServerControl
        {statusMessage}
        startServer={startConnection}
        stopServer={stopConnection} />
    <CameraSettings
        {contrastValue}
        updateContrast={updateContrast} />
</div>
