# ============================================================
# setup_secrets.ps1
# Creates secret/config header files outside the repository
# so they are never accidentally committed to version control.
#
# Generated files (two levels above the repo root):
#   ..\_secrets\WifiSecret.h
#   ..\_secrets\MqttSecret.h
#   ..\_secrets\OtaSecret.h
#   ..\_configs\MqttConfig.h
# ============================================================

$repoRoot    = Resolve-Path "$PSScriptRoot\.."
$secretsBase = Resolve-Path "$repoRoot\.."

$secretsDir  = Join-Path $secretsBase "_secrets"
$configsDir  = Join-Path $secretsBase "_configs"

Write-Host ""
Write-Host "=== GardenIrrigationControl - Secret Setup ===" -ForegroundColor Cyan
Write-Host "Repo root    : $repoRoot"
Write-Host "Secrets dir  : $secretsDir"
Write-Host "Configs dir  : $configsDir"
Write-Host ""

# ---- Create directories -------------------------------------------------------
foreach ($dir in @($secretsDir, $configsDir)) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
        Write-Host "[CREATED] Directory: $dir" -ForegroundColor Green
    } else {
        Write-Host "[EXISTS]  Directory: $dir" -ForegroundColor DarkGray
    }
}

# ---- Helper function ----------------------------------------------------------
function Write-FileIfMissing([string]$path, [string[]]$lines, [int]$count) {
    if (-not (Test-Path $path)) {
        $lines | Set-Content -Path $path -Encoding UTF8
        Write-Host "[CREATED] $path" -ForegroundColor Green
        return $count + 1
    } else {
        Write-Host "[SKIPPED] $path (already exists - not overwritten)" -ForegroundColor Yellow
        return $count
    }
}

# ---- File templates -----------------------------------------------------------
$wifiLines = @(
    '#ifndef WIFI_SECRET_H',
    '#define WIFI_SECRET_H',
    '',
    '// TODO: Replace with your WiFi credentials',
    '#define WIFI_SSID "Your_WiFi_SSID"',
    '#define WIFI_PWD  "Your_WiFi_Password"',
    '',
    '#endif // WIFI_SECRET_H'
)

$mqttSecretLines = @(
    '#ifndef MQTT_SECRET_H',
    '#define MQTT_SECRET_H',
    '',
    '// TODO: Replace with your MQTT broker credentials',
    '// Leave empty strings if the broker requires no authentication.',
    '#define MQTT_USER "your_mqtt_username"',
    '#define MQTT_PWD  "your_mqtt_password"',
    '',
    '#endif // MQTT_SECRET_H'
)

$otaLines = @(
    '#ifndef OTA_SECRET_H',
    '#define OTA_SECRET_H',
    '',
    '// TODO: Set a strong password for OTA updates, or leave empty to disable.',
    '#define OTA_PASSWORD ""',
    '',
    '#endif // OTA_SECRET_H'
)

$mqttConfigLines = @(
    '#ifndef MQTT_CONFIG_H',
    '#define MQTT_CONFIG_H',
    '',
    '// TODO: Replace with the IP address of your MQTT broker',
    '#define MQTT_SERVER_IP   "192.168.x.x"',
    '#define MQTT_SERVER_PORT 1883',
    '',
    '#endif // MQTT_CONFIG_H'
)

# ---- Write files (skip if already present) ------------------------------------
$created = 0
$created = Write-FileIfMissing "$secretsDir\WifiSecret.h"  $wifiLines       $created
$created = Write-FileIfMissing "$secretsDir\MqttSecret.h"  $mqttSecretLines $created
$created = Write-FileIfMissing "$secretsDir\OtaSecret.h"   $otaLines        $created
$created = Write-FileIfMissing "$configsDir\MqttConfig.h"  $mqttConfigLines $created

Write-Host ""
if ($created -gt 0) {
    Write-Host "Files created ($created). Please fill in your credentials." -ForegroundColor Cyan
} else {
    Write-Host "All files already exist. No changes made." -ForegroundColor DarkGray
}

# ---- Open Explorer so the user can edit the files ----------------------------
Write-Host ""
Write-Host "Opening Windows Explorer..." -ForegroundColor Cyan
Start-Process explorer.exe -ArgumentList "`"$secretsDir`""
Start-Process explorer.exe -ArgumentList "`"$configsDir`""

Write-Host ""
Write-Host "Done. Fill in all TODO values before building the project." -ForegroundColor Green
Write-Host ""
