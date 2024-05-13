<script>
    export let cameraSettings;
    export let updateSetting;

    // Map settings to their respective ranges, types, and labels
    const settingConfigurations = {
        autoExposure: { type: 'checkbox', label: 'Auto Exposure' },
        autoExposureBrightness: { type: 'range', min: -255, max: 255, label: 'Auto Exposure Brightness' },
        exposure: { type: 'range', min: 0, max: 1048575, label: 'Exposure' },
        gain: { type: 'range', min: 0, max: 1023, label: 'Gain' },
        autoWhiteBalance: { type: 'checkbox', label: 'Auto White Balance' },
        autoFocus: { type: 'checkbox', label: 'Auto Focus' },
        focus: { type: 'range', min: 0, max: 250, step: 25, label: 'Focus' },
        autoHue: { type: 'checkbox', label: 'Auto Hue' },
        hue: { type: 'range', min: -180, max: 180, label: 'Hue' },
        saturation: { type: 'range', min: 0, max: 255, label: 'Saturation' },
        contrast: { type: 'range', min: 0, max: 255, label: 'Contrast' },
        sharpness: { type: 'range', min: 0, max: 9, label: 'Sharpness' },
        redGain: { type: 'range', min: 0, max: 4095, label: 'Red Gain' },
        greenGain: { type: 'range', min: 0, max: 4095, label: 'Green Gain' },
        blueGain: { type: 'range', min: 0, max: 4095, label: 'Blue Gain' }
    };
</script>

<style>
    .section {
        border: 2px solid black;
        border-radius: 15px;
        padding: 30px;
        width: 300px;
        margin-bottom: 20px;
    }

    .section h2 {
        font-size: 20px;
        margin-bottom: 20px;
    }

    .control-container {
        display: flex;
        flex-direction: column;
        margin-bottom: 10px;
    }

    .control-container label {
        font-weight: bold;
        margin-bottom: 5px;
    }

    .control-container input[type="range"] {
        width: 100%;
    }

    .control-container input[type="checkbox"] {
        margin-left: 10px;
    }

    .control-container input[type="number"] {
        width: 100px;
        text-align: center;
    }
</style>

<div class="section">
    <h2>Camera Settings</h2>

    <!-- Auto Exposure -->
    <div class="control-container">
        <label>{settingConfigurations.autoExposure.label}</label>
        <input 
            type="checkbox" 
            checked={cameraSettings.autoExposure} 
            on:change={(e) => updateSetting('autoExposure', e.target.checked ? 1 : 0)} 
        />
    </div>

    <!-- Exposure, shown only if autoExposure is disabled -->
    {#if !cameraSettings.autoExposure}
        <div class="control-container">
            <label>{settingConfigurations.exposure.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.exposure} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.exposure.min} 
                    max={settingConfigurations.exposure.max} 
                    bind:value={cameraSettings.exposure} 
                    on:input={(e) => updateSetting('exposure', e.target.value)} 
                />
            </div>
        </div>
    {/if}

    <!-- Auto Exposure Brightness -->
    <div class="control-container">
        <label>{settingConfigurations.autoExposureBrightness.label}</label>
        <div style="display: flex; align-items: center;">
            <input type="number" value={cameraSettings.autoExposureBrightness} readonly />
            <input 
                type="range" 
                min={settingConfigurations.autoExposureBrightness.min} 
                max={settingConfigurations.autoExposureBrightness.max} 
                bind:value={cameraSettings.autoExposureBrightness} 
                on:input={(e) => updateSetting('autoExposureBrightness', e.target.value)} 
            />
        </div>
    </div>

    <!-- Gain -->
    <div class="control-container">
        <label>{settingConfigurations.gain.label}</label>
        <div style="display: flex; align-items: center;">
            <input type="number" value={cameraSettings.gain} readonly />
            <input 
                type="range" 
                min={settingConfigurations.gain.min} 
                max={settingConfigurations.gain.max} 
                bind:value={cameraSettings.gain} 
                on:input={(e) => updateSetting('gain', e.target.value)} 
            />
        </div>
    </div>

    <!-- Auto White Balance -->
    <div class="control-container">
        <label>{settingConfigurations.autoWhiteBalance.label}</label>
        <input 
            type="checkbox" 
            checked={cameraSettings.autoWhiteBalance} 
            on:change={(e) => updateSetting('autoWhiteBalance', e.target.checked ? 1 : 0)} 
        />
    </div>

    <!-- Red Gain, Green Gain, Blue Gain, shown only if autoWhiteBalance is disabled -->
    {#if !cameraSettings.autoWhiteBalance}
        <div class="control-container">
            <label>{settingConfigurations.redGain.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.redGain} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.redGain.min} 
                    max={settingConfigurations.redGain.max} 
                    bind:value={cameraSettings.redGain} 
                    on:input={(e) => updateSetting('redGain', e.target.value)} 
                />
            </div>
        </div>

        <div class="control-container">
            <label>{settingConfigurations.greenGain.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.greenGain} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.greenGain.min} 
                    max={settingConfigurations.greenGain.max} 
                    bind:value={cameraSettings.greenGain} 
                    on:input={(e) => updateSetting('greenGain', e.target.value)} 
                />
            </div>
        </div>

        <div class="control-container">
            <label>{settingConfigurations.blueGain.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.blueGain} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.blueGain.min} 
                    max={settingConfigurations.blueGain.max} 
                    bind:value={cameraSettings.blueGain} 
                    on:input={(e) => updateSetting('blueGain', e.target.value)} 
                />
            </div>
        </div>
    {/if}

    <!-- Auto Focus -->
    <div class="control-container">
        <label>{settingConfigurations.autoFocus.label}</label>
        <input 
            type="checkbox" 
            checked={cameraSettings.autoFocus} 
            on:change={(e) => updateSetting('autoFocus', e.target.checked ? 1 : 0)} 
        />
    </div>

    <!-- Focus, shown only if autoFocus is disabled -->
    {#if !cameraSettings.autoFocus}
        <div class="control-container">
            <label>{settingConfigurations.focus.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.focus} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.focus.min} 
                    max={settingConfigurations.focus.max} 
                    step={settingConfigurations.focus.step}
                    bind:value={cameraSettings.focus} 
                    on:input={(e) => updateSetting('focus', e.target.value)} 
                />
            </div>
        </div>
    {/if}

    <!-- Auto Hue -->
    <div class="control-container">
        <label>{settingConfigurations.autoHue.label}</label>
        <input 
            type="checkbox" 
            checked={cameraSettings.autoHue} 
            on:change={(e) => updateSetting('autoHue', e.target.checked ? 1 : 0)} 
        />
    </div>

    <!-- Hue, shown only if autoHue is disabled -->
    {#if !cameraSettings.autoHue}
        <div class="control-container">
            <label>{settingConfigurations.hue.label}</label>
            <div style="display: flex; align-items: center;">
                <input type="number" value={cameraSettings.hue} readonly />
                <input 
                    type="range" 
                    min={settingConfigurations.hue.min} 
                    max={settingConfigurations.hue.max} 
                    bind:value={cameraSettings.hue} 
                    on:input={(e) => updateSetting('hue', e.target.value)} 
                />
            </div>
        </div>
    {/if}

    <!-- Saturation -->
    <div class="control-container">
        <label>{settingConfigurations.saturation.label}</label>
        <div style="display: flex; align-items: center;">
            <input type="number" value={cameraSettings.saturation} readonly />
            <input 
                type="range" 
                min={settingConfigurations.saturation.min} 
                max={settingConfigurations.saturation.max} 
                bind:value={cameraSettings.saturation} 
                on:input={(e) => updateSetting('saturation', e.target.value)} 
            />
        </div>
    </div>

    <!-- Contrast -->
    <div class="control-container">
        <label>{settingConfigurations.contrast.label}</label>
        <div style="display: flex; align-items: center;">
            <input type="number" value={cameraSettings.contrast} readonly />
            <input 
                type="range" 
                min={settingConfigurations.contrast.min} 
                max={settingConfigurations.contrast.max} 
                bind:value={cameraSettings.contrast} 
                on:input={(e) => updateSetting('contrast', e.target.value)} 
            />
        </div>
    </div>

    <!-- Sharpness -->
    <div class="control-container">
        <label>{settingConfigurations.sharpness.label}</label>
        <div style="display: flex; align-items: center;">
            <input type="number" value={cameraSettings.sharpness} readonly />
            <input 
                type="range" 
                min={settingConfigurations.sharpness.min} 
                max={settingConfigurations.sharpness.max} 
                bind:value={cameraSettings.sharpness} 
                on:input={(e) => updateSetting('sharpness', e.target.value)} 
            />
        </div>
    </div>
</div>
