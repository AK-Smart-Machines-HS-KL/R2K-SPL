<script>
    let modeMessage = "Select Mode";
    let selectedMode = "";

    function selectMode(mode) {
        selectedMode = mode;
    }

    function sendModeCommand() {
        if (selectedMode) {
            modeMessage = `Mode set to: ${selectedMode}`;
            // Add logic to send the selected mode to the robot
            console.log(modeMessage);
        } else {
            modeMessage = "Please select a mode first";
        }
    }
</script>

<style>
    .mode-controls {
        border: 2px solid black;
        border-radius: 15px;
        padding: 20px;
        width: 400px;
        margin: 0 auto 20px auto; /* Center the section horizontally */
        display: flex;
        flex-direction: column;
        align-items: center; /* Center child elements horizontally */
    }

    .button-group {
        display: flex;
        flex-direction: column;
        gap: 10px;
        margin-top: 10px;
        width: 100%;
    }

    .mode-selection {
        display: flex;
        justify-content: center;
        gap: 20px;
        margin-top: 10px;
    }

    button {
        padding: 10px;
        font-size: 14px;
        border: none;
        border-radius: 5px;
        cursor: pointer;
        background-color: gray;
        color: white;
        width: 100%;
        text-align: center;
    }

    button.active {
        background-color: green;
    }

    .send-button {
        background-color: blue;
        margin-top: 20px;
    }

    .status {
        margin-top: 20px;
        font-size: 18px;
        font-weight: bold;
    }
</style>

<div class="mode-controls">
    <h2>Robot Mode Control</h2>
    <div class="mode-selection">
        <button
            class:active={selectedMode === "Auto"}
            on:click={() => selectMode("Auto")}>
            Auto Mode
        </button>
        <button
            class:active={selectedMode === "Operator"}
            on:click={() => selectMode("Operator")}>
            Operator Mode
        </button>
    </div>
    <button class="send-button" on:click={sendModeCommand}>Send Mode Command</button>
    <div class="status">{modeMessage}</div>
</div>
