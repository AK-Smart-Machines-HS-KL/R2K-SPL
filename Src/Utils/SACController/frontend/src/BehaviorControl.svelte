<script>
    let behaviorMessage = "No Behavior Selected";
    let activeBehavior = "";

    const behaviors = [
        "defenseChaseBallCard",      
        "offenseChaseBallCard",      
        "clearOwnHalfCard",  
        "clearOwnHalfCardGoalie",
        "defenseCard",        
        "defenseLongShotCard",
        "dive",
        "goalShotCard",
        "goalieLongShotCard", 
        "offenseFastGoalKick",
        "offenseForwardPassCard",
        "offenseReceivePassCard"
    ];

    function selectBehavior(behavior) {
        behaviorMessage = `Selected behavior: ${behavior}`;
        activeBehavior = behavior;
        // Add logic to send the selected behavior to the robot
        console.log(behaviorMessage);
    }
</script>

<style>
    .behavior-controls {
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

    .status {
        margin-top: 20px;
        font-size: 18px;
        font-weight: bold;
    }
</style>

<div class="behavior-controls">
    <h2>Robot Behavior Control</h2>
    <div class="button-group">
        {#each behaviors as behavior}
            <button
                class:active={activeBehavior === behavior}
                on:click={() => selectBehavior(behavior)}>
                {behavior}
            </button>
        {/each}
    </div>
    <div class="status">{behaviorMessage}</div>
</div>
