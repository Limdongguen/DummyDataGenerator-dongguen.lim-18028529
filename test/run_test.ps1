# DummyDataGenerator POC Test Script
param([string]$ExePath = "build\dummy-data-generator.exe")

$PASS = 0; $FAIL = 0
function Check($desc, $cond) {
    if ($cond) { Write-Host "[PASS] $desc" -ForegroundColor Green; $script:PASS++ }
    else        { Write-Host "[FAIL] $desc" -ForegroundColor Red;   $script:FAIL++ }
}

Write-Host ""; Write-Host "== DummyDataGenerator Test ==" -ForegroundColor Cyan; Write-Host ""

$root = Split-Path $PSScriptRoot -Parent
$exe  = Join-Path $root $ExePath
if (-not (Test-Path $exe)) {
    $exe = Join-Path $root "DummyDataGenerator_POC\x64\Debug\DummyDataGenerator_POC.exe"
}
Check "Executable exists" (Test-Path $exe)
if ($FAIL -gt 0) { Write-Host "Build first." -ForegroundColor Red; exit 1 }

$base = Join-Path $PSScriptRoot "output"

# Test 1: Standard (5 samples, 10 orders, seed 42)
Write-Host "[Test 1] Standard -- 5 samples, 10 orders, seed 42" -ForegroundColor Yellow
$o1 = Join-Path $base "test1"
& $exe $o1 5 10 42 | Out-Null
Check "test1 dir created"      (Test-Path $o1)
Check "samples.json created"   (Test-Path "$o1\samples.json")
Check "orders.json created"    (Test-Path "$o1\orders.json")
Check "inventory.json created" (Test-Path "$o1\inventory.json")
Check "production.json created"(Test-Path "$o1\production.json")

$sJ = [System.IO.File]::ReadAllText("$o1\samples.json",    [System.Text.Encoding]::UTF8)
$oJ = [System.IO.File]::ReadAllText("$o1\orders.json",     [System.Text.Encoding]::UTF8)
$iJ = [System.IO.File]::ReadAllText("$o1\inventory.json",  [System.Text.Encoding]::UTF8)
$pJ = [System.IO.File]::ReadAllText("$o1\production.json", [System.Text.Encoding]::UTF8)

Check "samples: S-001 exists"               ($sJ -match '"id": "S-001"')
Check "samples: S-005 exists"               ($sJ -match '"id": "S-005"')
Check "samples: semiconductor name"         ($sJ -match "S-00")
Check "samples: yield field"                ($sJ -match '"yield"')
Check "samples: avgProductionTime field"    ($sJ -match '"avgProductionTime"')
Check "orders: 10 records"                  (([regex]::Matches($oJ,'"orderId"')).Count -eq 10)
Check "orders: valid status"               ($oJ -match '"status": "(RESERVED|CONFIRMED|PRODUCING|RELEASED)"')
Check "orders: customerName field"          ($oJ -match '"customerName"')
Check "inventory: S-001 stock"              ($iJ -match '"sampleId": "S-001"')
Check "inventory: positive quantity"        ($iJ -match '"quantity": [1-9]')
Check "production: jobs array"              ($pJ -match '"jobs"')

# Test 2: Large (10 samples, 50 orders, seed 99)
Write-Host ""; Write-Host "[Test 2] Large -- 10 samples, 50 orders, seed 99" -ForegroundColor Yellow
$o2 = Join-Path $base "test2"
& $exe $o2 10 50 99 | Out-Null
Check "test2 dir created"   (Test-Path $o2)
$s2 = [System.IO.File]::ReadAllText("$o2\samples.json",[System.Text.Encoding]::UTF8)
$or2= [System.IO.File]::ReadAllText("$o2\orders.json", [System.Text.Encoding]::UTF8)
Check "10 samples: S-010 exists" ($s2 -match '"id": "S-010"')
Check "50 orders generated"      (([regex]::Matches($or2,'"orderId"')).Count -eq 50)

# Test 3: Reproducibility (same seed -> same output)
Write-Host ""; Write-Host "[Test 3] Reproducibility -- seed 7 x2" -ForegroundColor Yellow
$o3a = Join-Path $base "test3a"; $o3b = Join-Path $base "test3b"
& $exe $o3a 3 5 7 | Out-Null; & $exe $o3b 3 5 7 | Out-Null
$sa=[System.IO.File]::ReadAllText("$o3a\samples.json",[System.Text.Encoding]::UTF8)
$sb=[System.IO.File]::ReadAllText("$o3b\samples.json",[System.Text.Encoding]::UTF8)
$oa=[System.IO.File]::ReadAllText("$o3a\orders.json", [System.Text.Encoding]::UTF8)
$ob=[System.IO.File]::ReadAllText("$o3b\orders.json", [System.Text.Encoding]::UTF8)
Check "Same seed -> same samples.json" ($sa -eq $sb)
Check "Same seed -> same orders.json"  ($oa -eq $ob)

# Test 4: JSON validity
Write-Host ""; Write-Host "[Test 4] JSON validity (ConvertFrom-Json)" -ForegroundColor Yellow
try { $j=$sJ|ConvertFrom-Json; Check "samples.json parse OK (5)" ($j.samples.Count -eq 5)
      Check "nextSeq field (6)" ($j.nextSeq -eq 6) } catch { Check "samples.json parse OK" $false }
try { $j2=$oJ|ConvertFrom-Json; Check "orders.json parse OK (10)" ($j2.orders.Count -eq 10)
} catch { Check "orders.json parse OK" $false }
try { $j3=$iJ|ConvertFrom-Json; Check "inventory.json parse OK"  ($j3.inventory.Count -gt 0)
} catch { Check "inventory.json parse OK" $false }
try { $j4=$pJ|ConvertFrom-Json; Check "production.json parse OK" ($null -ne $j4.jobs)
} catch { Check "production.json parse OK" $false }

Write-Host ""; Write-Host "==============================" -ForegroundColor Cyan
$total=$PASS+$FAIL; $col=if($FAIL -eq 0){"Green"}else{"Red"}
Write-Host "Result: $PASS/$total passed" -ForegroundColor $col
Write-Host "==============================" -ForegroundColor Cyan; Write-Host ""
if ($FAIL -gt 0) { exit 1 }