# From https://blog.didenko.com/2013/11/version-inventory.html

Param (
 [String]$Project,
 [String]$GitRoot,
 [String]$HeaderFile="version.h",
 [String]$VerPrefix="https://github.com/defl/videoprocessor/commit/"
)

Push-Location -LiteralPath $GitRoot

$VerFileHead     = "`#pragma once`n`n`#include <atlstr.h>`n`n"
$VerFileTail     = "`n"
$VerDescribePre  = "static const TCHAR* VERSION_DESCRIBE=TEXT(`""
$VerDescribePost = "`");`n"

$VerBy       = (git log -n 1 --format=format:"static const TCHAR* VERSION_AUTHOR=TEXT(`\`"%an `<%ae`>`\`");%n") | Out-String
$VerUrl      = (git log -n 1 --format=format:"static const TCHAR* VERSION_URL=TEXT(`\`"$VerPrefix%H`\`");%n") | Out-String
$VerDate     = (git log -n 1 --format=format:"static const TCHAR* VERSION_DATE=TEXT(`\`"%ai`\`");%n") | Out-String
$VerDescribe = (git describe --tags --long).Trim() | Out-String
$VerDescribe = $VerDescribe.TrimEnd()

$VerChgs = ((git ls-files --exclude-standard -d -m -o -k) | Measure-Object -Line).Lines

if ($VerChgs -gt 0) {
  $VerDirty = "const bool VERSION_DIRTY=true;`n"
} else {
  $VerDirty = "const bool VERSION_DIRTY=false;`n"
}

"Written $Project\" + (
  New-Item -Force -Path "$Project" -Name "$HeaderFile" -ItemType "file" -Value "$VerFileHead$VerUrl$VerDate$VerDescribePre$VerDescribe$VerDescribePost$VerBy$VerDirty$VerFileTail"
).Name + " as:"
""
Get-Content "$Project\$HeaderFile"
""

Pop-Location
