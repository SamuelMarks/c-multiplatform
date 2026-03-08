param([string]$xmlPath, [string]$filename)
[xml]$xml = Get-Content $xmlPath
$class = $xml.coverage.packages.package.classes.class | Where-Object { $_.filename -match $filename }
if ($class) {
    $class.lines.line | Where-Object { [int]$_.number -ge 40 -and [int]$_.number -le 60 } | Select-Object number, hits, 'condition-coverage'
}
