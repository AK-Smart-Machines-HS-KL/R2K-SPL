<script>
    let directionMessage = "Stopped";
    let activeMoveDirection = "";
    let activeTurnDirection = "";

    function move(direction) {
        directionMessage = `Moving ${direction}`;
        activeMoveDirection = direction;
        activeTurnDirection = "";
        // Add logic to send the move command to the robot for the specified direction
        console.log(directionMessage);
    }

    function turn(direction) {
        directionMessage = `Turning ${direction}`;
        activeTurnDirection = direction;
        activeMoveDirection = "";
        // Add logic to send the turn command to the robot for the specified direction
        console.log(directionMessage);
    }

    function stop() {
        directionMessage = "Stopped";
        activeMoveDirection = "";
        activeTurnDirection = "";
        // Add logic to send stop command to the robot
        console.log(directionMessage);
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
    <h2>Direction Control</h2>
    <div class="button-group">
        <button
            class:active={activeMoveDirection === "Forward"}
            on:click={() => move("Forward")}>Forward</button>
    </div>
    <div class="button-group">
        <button
            class:active={activeMoveDirection === "Left"}
            on:click={() => move("Left")}>Left</button>
        <button
            class:active={activeMoveDirection === ""}
            on:click={stop}>Stop</button>
        <button
            class:active={activeMoveDirection === "Right"}
            on:click={() => move("Right")}>Right</button>
    </div>
    <div class="button-group">
        <button
            class:active={activeMoveDirection === "Backward"}
            on:click={() => move("Backward")}>Backward</button>
    </div>
    <div class="button-group">
        <button
            class:active={activeTurnDirection === "Left"}
            on:click={() => turn("Left")}>Turn Left</button>
        <button
            class:active={activeTurnDirection === "Right"}
            on:click={() => turn("Right")}>Turn Right</button>
    </div>
    <div class="status">{directionMessage}</div>
</div>