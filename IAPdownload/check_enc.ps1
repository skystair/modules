 = [System.IO.File]::ReadAllText('f:\GIT\modules\IAPdownload\_code\device\lcd\lcd.c', [System.Text.Encoding]::Default)
Write-Host .Substring(800, 200)
