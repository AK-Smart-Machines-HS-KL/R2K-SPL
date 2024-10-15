<script>
        let modeMessage = "Select Mode";
    let selectedMode = "";

    const modeMapping = {
        "Auto": 0,
        "Operator": 1
    };

    async function sendModeCommand() {
        if (selectedMode !== "") {
            const modeValue = modeMapping[selectedMode];
            modeMessage = `Mode set to: ${selectedMode}`;
            // Add logic to send the selected mode to the robot
            try {
                const response = await fetch('http://localhost:5000/mode', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({ mode: modeValue })
                });
                const result = await response.json();
                console.log(result);
            } catch (error) {
                console.error('Error:', error);
            }
        } else {
            modeMessage = "Please select a mode first";
        }
    }

    function selectMode(mode) {
        selectedMode = mode;
        updateButtonStyles();
    }

    function updateButtonStyles() {
        document.querySelectorAll('.mode-selection button').forEach(button => {
            button.classList.remove('active');
        });
        if (selectedMode) {
            document.querySelector(`.mode-selection button[data-mode="${selectedMode}"]`).classList.add('active');
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
