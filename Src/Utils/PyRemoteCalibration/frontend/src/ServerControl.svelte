<script>
    export let statusMessage;
    export let startServer;
    export let stopServer;
</script>

<style>
    .section {
        border: 2px solid black;
        border-radius: 15px;
        padding: 20px;
        width: 400px;
        margin: 0 auto 20px auto; /* Center the section horizontally */
        display: flex;
        flex-direction: column;
        align-items: center; /* Center child elements horizontally */
    }

    .section h2 {
        font-size: 20px;
        margin-bottom: 10px;
        text-align: center;
    }

    .button-container {
        margin-top: 10px;
        display: flex;
        justify-content: center; /* Center buttons horizontally */
        gap: 10px; /* Space between buttons */
    }

    button {
        background-color: #007bff;
        color: white;
        border: none;
        padding: 10px 20px;
        cursor: pointer;
        border-radius: 5px;
        font-size: 16px;
        transition: background-color 0.3s;
    }

    button:hover {
        background-color: #0056b3;
    }

    .status {
        margin-top: 10px;
        font-size: 16px;
        text-align: center;
    }
</style>

<div class="section">
    <h2>Server Control</h2>
    <div class="button-container">
        <button on:click={startServer}>Start Server</button>
        <button on:click={stopServer}>Stop Server</button>
    </div>
    <div class="status">Status: {statusMessage}</div>
</div>

