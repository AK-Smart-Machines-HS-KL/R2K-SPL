<script>
    export let contrastValue;
    export let updateContrast;
</script>

<style>
    .section {
        border: 2px solid black;
        border-radius: 15px;
        padding: 30px;
        width: 300px;
    }

    .section h2 {
        font-size: 20px;
        margin-bottom: 20px;
    }

    .slider-container {
        display: flex;
        align-items: center;
        justify-content: space-between;
    }

    .slider-container input {
        margin-left: 10px;
        width: 100%;
    }
</style>

<div class="section">
    <h2>Camera Settings</h2>
    <div class="slider-container">
        <input type="number" value={contrastValue} readonly style="width: 60px; text-align: center;" />
        <input type="range" min="0" max="100" value={contrastValue} on:input={updateContrast} />
    </div>
</div>
