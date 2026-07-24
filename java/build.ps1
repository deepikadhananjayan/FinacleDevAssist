$ErrorActionPreference = "Stop"

$root = Get-Location
$src = "$root\src"
$libDir = "$root\lib"
$build = "$root\build"

# MAIN CLASS
$mainClass = "com.sandy.fda.FinacleDevAssist"

# Clean
Remove-Item -Recurse -Force $build -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path "$build\classes" | Out-Null

# Get all Java files
$files = Get-ChildItem -Recurse -Filter *.java $src | Select-Object -ExpandProperty FullName

# Build classpath from all libs
$libJars = Get-ChildItem "$libDir\*.jar" | ForEach-Object FullName
$classpath = ($libJars -join ";")

# Compile
javac -cp "$classpath" -d "$build\classes" $files

# Copy compiled classes
Copy-Item -Recurse "$build\classes\*" $build

# Extract all jars into build
foreach ($jar in $libJars) {
    Push-Location $build
    jar xf $jar
    Pop-Location
}

# Create MANIFEST
$manifestPath = "$build\MANIFEST.MF"

@"
Manifest-Version: 1.0
Main-Class: $mainClass

"@ | Set-Content -Encoding ASCII $manifestPath

jar cfm finacle-dev-assist.jar $manifestPath -C $build .

Write-Host "`nSUCCESS: finacle-dev-assist.jar created"