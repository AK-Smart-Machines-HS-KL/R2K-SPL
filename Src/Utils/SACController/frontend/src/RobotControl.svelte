<script>
let directionMessage = "Stopped";
let activeMoveDirection = "";
let activeTurnDirection = "";
let activeAction = "";

const directionMapping = {
    "Stop": 0,
    "Forward": 1,
    "Backward": 2,
    "Left": 3,
    "Right": 4,
    "Turn Left": 5,
    "Turn Right": 6,
    "Kick": 7,
    "Dribble": 8,
    "GetUp": 9
};

async function sendDirectionCommand(direction) {
    const directionValue = directionMapping[direction];
    if (directionValue !== undefined) {
        directionMessage = `Direction set to: ${direction}`;
        // Add logic to send the direction command to the robot
        try {
            const response = await fetch('http://localhost:5000/direction', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ direction: directionValue })
            });
            const result = await response.json();
            console.log(result);
        } catch (error) {
            console.error('Error:', error);
        }
    }
}

function move(direction) {
    directionMessage = `Moving ${direction}`;
    activeMoveDirection = direction;
    activeTurnDirection = "";
    activeAction = "";
    sendDirectionCommand(direction);
    updateButtonStyles();
    console.log(directionMessage);
}

function turn(direction) {
    directionMessage = `Turning ${direction}`;
    activeTurnDirection = direction;
    activeMoveDirection = "";
    activeAction = "";
    sendDirectionCommand(`Turn ${direction}`);
    updateButtonStyles();
    console.log(directionMessage);
}

function action(actionType) {
    directionMessage = `${actionType} action performed`;
    activeAction = actionType;
    activeMoveDirection = "";
    activeTurnDirection = "";
    sendDirectionCommand(actionType);
    updateButtonStyles();
    console.log(directionMessage);
}

function stop() {
    directionMessage = "Stopped";
    activeMoveDirection = "";
    activeTurnDirection = "";
    activeAction = "";
    sendDirectionCommand("Stop");
    updateButtonStyles();
    console.log(directionMessage);
}

function updateButtonStyles() {
    document.querySelectorAll('.button-group button').forEach(button => {
        button.classList.remove('active');
    });
    if (activeMoveDirection) {
        document.querySelector(`button[data-direction="${activeMoveDirection}"]`).classList.add('active');
    } else if (activeTurnDirection) {
        document.querySelector(`button[data-direction="Turn ${activeTurnDirection}"]`).classList.add('active');
    } else if (activeAction) {
        document.querySelector(`button[data-action="${activeAction}"]`).classList.add('active');
    }
}
</script>

<style>
.controls {
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
    gap: 10px;
    margin-top: 10px;
}

button {
    padding: 10px;
    font-size: 16px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    background-color: gray;
    color: white;
}

button.active {
    background-color: green;
}

.status {
    margin-top: 20px;
    font-size: 18px;
    font-weight: bold;
}
</style>

<div class="controls">
    <h2>Robot Control</h2>
    <div class="button-group">
        <button class:active={activeMoveDirection === "Forward"} on:click={() => move("Forward")}>Forward</button>
    </div>
    <div class="button-group">
        <button class:active={activeMoveDirection === "Left"} on:click={() => move("Left")}>Left</button>
        <button class:active={activeMoveDirection === ""} on:click={stop}>Stop</button>
        <button class:active={activeMoveDirection === "Right"} on:click={() => move("Right")}>Right</button>
    </div>
    <div class="button-group">
        <button class:active={activeMoveDirection === "Backward"} on:click={() => move("Backward")}>Backward</button>
    </div>
    <div class="button-group">
        <button class:active={activeTurnDirection === "Leftwards"} on:click={() => turn("Left")}>Turn Left</button>
        <button class:active={activeTurnDirection === "Rightwards"} on:click={() => turn("Right")}>Turn Right</button>
    </div>
    <div class="button-group">
        <button class:active={activeMoveDirection === "Kick"} on:click={() => action("Kick")}>Kick</button>
        <button class:active={activeMoveDirection === "Dribble"} on:click={() => action("Dribble")}>Dribble</button>
        <button class:active={activeMoveDirection === "GetUp"} on:click={() => action("GetUp")}>GetUp</button>

    </div>
    <div class="status">{directionMessage}</div>
</div>