# ============================================================
# setup_secrets.ps1
# Creates secret/config header files outside the repository
# so they are never accidentally committed to version control.
#
# Generated files in an external base directory:
#   <base>\_secrets\WifiSecret.h
#   <base>\_secrets\MqttSecret.h
#   <base>\_secrets\OtaSecret.h
#   <base>\_config\MqttConfig.h
#
# Base directory rule:
# - always parent directory of repo root
# ============================================================

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$secretsBase = Resolve-Path "$repoRoot\.."

$secretsDir = Join-Path $secretsBase "_secrets"
$configsDir = Join-Path $secretsBase "_config"

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
    }
    else {
        Write-Host "[EXISTS]  Directory: $dir" -ForegroundColor DarkGray
    }
}

# ---- Helper function ----------------------------------------------------------
function Write-FileWithBackup([string]$path, [string[]]$lines, [ref]$created, [ref]$updated, [ref]$backups) {
    if (Test-Path $path) {
        $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
        $backupPath = "$path.$timestamp.bak"
        Copy-Item -Path $path -Destination $backupPath -Force
        $backups.Value++
        Write-Host "[BACKUP]  $backupPath" -ForegroundColor DarkYellow

        $lines | Set-Content -Path $path -Encoding UTF8
        $updated.Value++
        Write-Host "[UPDATED] $path" -ForegroundColor Yellow
        return
    }

    $lines | Set-Content -Path $path -Encoding UTF8
    $created.Value++
    Write-Host "[CREATED] $path" -ForegroundColor Green
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

# ---- Write files (backup + overwrite if already present) ----------------------
$created = 0
$updated = 0
$backups = 0

Write-FileWithBackup "$secretsDir\WifiSecret.h"  $wifiLines       ([ref]$created) ([ref]$updated) ([ref]$backups)
Write-FileWithBackup "$secretsDir\MqttSecret.h"  $mqttSecretLines ([ref]$created) ([ref]$updated) ([ref]$backups)
Write-FileWithBackup "$secretsDir\OtaSecret.h"   $otaLines        ([ref]$created) ([ref]$updated) ([ref]$backups)
Write-FileWithBackup "$configsDir\MqttConfig.h"  $mqttConfigLines ([ref]$created) ([ref]$updated) ([ref]$backups)

Write-Host ""
Write-Host "Summary: created=$created, updated=$updated, backups=$backups" -ForegroundColor Cyan

# ---- Open Explorer so the user can edit the files ----------------------------
Write-Host ""
Write-Host "Opening Windows Explorer..." -ForegroundColor Cyan
Start-Process explorer.exe -ArgumentList "`"$secretsDir`""
Start-Process explorer.exe -ArgumentList "`"$configsDir`""

Write-Host ""
Write-Host "Done. Fill in all TODO values before building the project." -ForegroundColor Green
Write-Host ""
