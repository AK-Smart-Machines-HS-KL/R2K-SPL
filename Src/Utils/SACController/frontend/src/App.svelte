<script>
    import ServerControl from './ServerControl.svelte';
    import RobotControl from './RobotControl.svelte';
    import BehaviorControl from './BehaviorControl.svelte';
    import ModeControl from './ModeControl.svelte';

    let statusMessage = "Not Connected";
    
      async function startConnection() {
        try {
            const response = await fetch('http://localhost:5000/start', {
                method: 'POST',
            });
            const result = await response.json();
            if (result.clients.length > 0) {
                statusMessage = `Connected to ${result.clients.map(client => client[0]).join(', ')}`;
            } else {
                statusMessage = "Listener Server started but no clients";
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
    <img src="robocup-logo.png" alt="RoboCup Logo" class="logo">
    <h1>Shared Autonomy Challenge</h1>
</div>
<div class="main-container">
    <div class="container">
        <ServerControl
            {statusMessage}
            startServer={startConnection}
            stopServer={stopConnection} />
    </div>
        <div class="container">
        <ModeControl />
    </div>
        <div class="container">
        <RobotControl />
    </div>
        <div class="container">
        <BehaviorControl />
    </div>
</div>
