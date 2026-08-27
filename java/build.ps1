$ErrorActionPreference = "Stop"

$root = Get-Location
$src = "$root\src"
$libDir = "$root\lib"
$build = "$root\build"

$jrePath = "$root\jre-17"

if (-not (Test-Path -Path $jrePath)) {
    Write-Host "Creating JRE..."

    $jdkPath = Split-Path -Parent (Split-Path -Parent (Get-Command java).Source)

    & "$jdkPath\bin\jlink.exe" `
        --module-path "$jdkPath\jmods" `
        --add-modules java.base,java.xml,java.net.http,jdk.compiler `
        --strip-debug `
        --no-man-pages `
        --no-header-files `
        --compress=2 `
        --output $jrePath
}
else {
    Write-Host "JRE already exists. Skipping creation."
}

# MAIN CLASS
$mainClass = "com.sandy.fda.FinacleDevAssist"

# 2. Cleaning & Preparing Directories
Write-Host "Cleaning build directory..."
Remove-Item -Recurse -Force $build -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path "$build\classes" | Out-Null

# Get all Java files
$files = Get-ChildItem -Recurse -Filter *.java $src | Select-Object -ExpandProperty FullName

# Build classpath from all libs
$libJars = Get-ChildItem "$libDir\*.jar" | ForEach-Object FullName
$classpath = ($libJars -join ";")

# 3. Compilation
Write-Host "Compiling Java files..."
javac -cp "$classpath" -d "$build" $files

# 4. Extracting Libraries & Copying Resources
Write-Host "Extracting dependency JARs..."
foreach ($jar in $libJars) {
    Push-Location $build
    jar xf $jar
    Pop-Location
}

Write-Host "Cleaning extracted signature and metadata files..."
$unwantedPatterns = @(
    "$build\META-INF\*.SF",
    "$build\META-INF\*.DSA",
    "$build\META-INF\*.RSA",
    "$build\META-INF\MANIFEST.MF",
    "$build\META-INF\INDEX.LIST"
)
Remove-Item -Path $unwantedPatterns -Force -ErrorAction SilentlyContinue

# Copy resources folder directly into the root of $build so it maps to /resources/
Copy-Item -Recurse "$src\resources" "$build\resources"

# 5. Packaging JAR
Write-Host "Creating runnable JAR..."
$manifestPath = "$build\MANIFEST.MF"

@"
Manifest-Version: 1.0
Main-Class: $mainClass

"@ | Set-Content -Encoding ASCII $manifestPath

# Package directly from $build root
jar cfm finacle-dev-assist.jar $manifestPath -C $build .

Write-Host "`nSUCCESS: finacle-dev-assist.jar created"