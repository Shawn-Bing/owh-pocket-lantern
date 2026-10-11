param([string]$Image = '', [switch]$Deskew, [switch]$HighDetail, [switch]$Numeric)
# Runs in Windows PowerShell 5.1, hidden by the agent. All recognition is local.
# stdin/stdout carry one JSON object per line; no screen or input APIs are used.
$ErrorActionPreference = 'Stop'
[Console]::InputEncoding = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
try {
    Add-Type -AssemblyName System.Runtime.WindowsRuntime
    Add-Type -AssemblyName System.Drawing
    $null = [Windows.Storage.StorageFile, Windows.Storage, ContentType = WindowsRuntime]
    $null = [Windows.Storage.Streams.IRandomAccessStream, Windows.Storage.Streams, ContentType = WindowsRuntime]
    $null = [Windows.Graphics.Imaging.BitmapDecoder, Windows.Graphics.Imaging, ContentType = WindowsRuntime]
    $null = [Windows.Graphics.Imaging.SoftwareBitmap, Windows.Graphics.Imaging, ContentType = WindowsRuntime]
    $null = [Windows.Media.Ocr.OcrEngine, Windows.Media.Ocr, ContentType = WindowsRuntime]
    $null = [Windows.Media.Ocr.OcrResult, Windows.Media.Ocr, ContentType = WindowsRuntime]
    $null = [Windows.Globalization.Language, Windows.Globalization, ContentType = WindowsRuntime]
    $script:asTask = [System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
        $_.Name -eq 'AsTask' -and $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and
        $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
    } | Select-Object -First 1
    function Await-Operation($Operation, [Type]$ResultType) {
        $task = $script:asTask.MakeGenericMethod($ResultType).Invoke($null, @($Operation))
        if (-not $task.Wait(4000)) { throw 'OCR operation timed out' }
        $task.GetAwaiter().GetResult()
    }
    $language = New-Object Windows.Globalization.Language('zh-Hans')
    $script:engine = [Windows.Media.Ocr.OcrEngine]::TryCreateFromLanguage($language)
    if (-not $script:engine) {
        throw 'Chinese OCR is unavailable. Install the Windows Chinese (Simplified) OCR language feature.'
    }
    $script:numericEngine = $null
    try { $script:numericEngine = [Windows.Media.Ocr.OcrEngine]::TryCreateFromLanguage((New-Object Windows.Globalization.Language('en-US'))) } catch { }
    function Add-OcrLines($Result, $Bitmap, $Lines, [double]$OriginalWidth, [double]$OriginalHeight,
                          [double]$Shear = 0) {
        $imageWidth = $OriginalWidth + $Shear * $OriginalHeight
        foreach ($line in $Result.Lines) {
            $words = @()
            foreach ($word in $line.Words) {
                $r = $word.BoundingRect
                # Invert the italic correction to recover actual client coordinates.
                $x = [Math]::Max(0.0, ($r.X / $Bitmap.PixelWidth * $imageWidth -
                    $Shear * ($r.Y + $r.Height) / $Bitmap.PixelHeight * $OriginalHeight) / $OriginalWidth)
                $right = [Math]::Min(1.0, (($r.X + $r.Width) / $Bitmap.PixelWidth * $imageWidth -
                    $Shear * $r.Y / $Bitmap.PixelHeight * $OriginalHeight) / $OriginalWidth)
                if ($right -le $x) { continue }
                $words += @{ text = $word.Text; roi = @($x, ($r.Y / $Bitmap.PixelHeight),
                    ($right - $x), ($r.Height / $Bitmap.PixelHeight)) }
            }
            $Lines.Add(@{ text = $line.Text; words = $words })
        }
    }
    function Read-ImageText([string]$Path, [long]$Id, [bool]$FixItalic = $false, [bool]$Detail = $false, [bool]$Numbers = $false) {
        $selectedEngine = if ($Numbers) { $script:numericEngine } else { $script:engine }
        if (-not $selectedEngine) { throw 'Numeric OCR is unavailable' }
        $scaleLimit = if ($Detail) { 3.0 } else { 2.0 }
        # Decode an in-memory copy. WinRT's decoder may retain its stream after
        # recognition; binding it to the disk file blocked the next overwrite
        # in a persistent worker even after calling the stream's Dispose.
        $imageBytes = [System.IO.File]::ReadAllBytes($Path)
        $sourceMemory = [System.IO.MemoryStream]::new($imageBytes, $false)
        $stream = $null
        $bitmap = $null
        try {
            $stream = [System.IO.WindowsRuntimeStreamExtensions]::AsRandomAccessStream($sourceMemory)
            $decoder = Await-Operation ([Windows.Graphics.Imaging.BitmapDecoder]::CreateAsync($stream)) ([Windows.Graphics.Imaging.BitmapDecoder])
            # Upsample small game windows so Chinese menu glyphs stay readable.
            # Bounding boxes are fractions of this image, independent of scale.
            $scale = [Math]::Min($scaleLimit, [Math]::Min(2560.0, [Windows.Media.Ocr.OcrEngine]::MaxImageDimension) /
                [Math]::Max($decoder.PixelWidth, $decoder.PixelHeight))
            $transform = New-Object Windows.Graphics.Imaging.BitmapTransform
            $transform.ScaledWidth = [uint32][Math]::Round($decoder.PixelWidth * $scale)
            $transform.ScaledHeight = [uint32][Math]::Round($decoder.PixelHeight * $scale)
            $bitmap = Await-Operation ($decoder.GetSoftwareBitmapAsync(
                [Windows.Graphics.Imaging.BitmapPixelFormat]::Bgra8,
                [Windows.Graphics.Imaging.BitmapAlphaMode]::Ignore, $transform,
                [Windows.Graphics.Imaging.ExifOrientationMode]::IgnoreExifOrientation,
                [Windows.Graphics.Imaging.ColorManagementMode]::DoNotColorManage)) ([Windows.Graphics.Imaging.SoftwareBitmap])
            $result = Await-Operation ($selectedEngine.RecognizeAsync($bitmap)) ([Windows.Media.Ocr.OcrResult])
            $lines = New-Object 'System.Collections.Generic.List[object]'
            Add-OcrLines $result $bitmap $lines $decoder.PixelWidth $decoder.PixelHeight
            if ($FixItalic) {
                # Overwatch's slanted display font is regularly misread at small
                # window sizes. A second, sheared pass straightens those glyphs.
                # Normal text from the first pass is retained as well.
                $original = $null; $corrected = $null; $graphics = $null
                $memory = $null; $correctedStream = $null; $correctedBitmap = $null
                try {
                    $original = [System.Drawing.Bitmap]::new($Path)
                    $corrected = [System.Drawing.Bitmap]::new([int][Math]::Ceiling($original.Width + .22 * $original.Height), $original.Height)
                    $graphics = [System.Drawing.Graphics]::FromImage($corrected)
                    $graphics.Clear([System.Drawing.Color]::Black)
                    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                    $points = [System.Drawing.PointF[]]@([System.Drawing.PointF]::new(0,0),
                        [System.Drawing.PointF]::new($original.Width,0),
                        [System.Drawing.PointF]::new([single](.22*$original.Height),$original.Height))
                    $graphics.DrawImage($original,$points)
                    $memory = [System.IO.MemoryStream]::new()
                    $corrected.Save($memory,[System.Drawing.Imaging.ImageFormat]::Png)
                    $memory.Position = 0
                    $correctedStream = [System.IO.WindowsRuntimeStreamExtensions]::AsRandomAccessStream($memory)
                    $correctedDecoder = Await-Operation ([Windows.Graphics.Imaging.BitmapDecoder]::CreateAsync($correctedStream)) ([Windows.Graphics.Imaging.BitmapDecoder])
                    $correctedScale = [Math]::Min($scaleLimit, [Math]::Min(2560.0,[Windows.Media.Ocr.OcrEngine]::MaxImageDimension) /
                        [Math]::Max($correctedDecoder.PixelWidth,$correctedDecoder.PixelHeight))
                    $transform.ScaledWidth = [uint32][Math]::Round($correctedDecoder.PixelWidth * $correctedScale)
                    $transform.ScaledHeight = [uint32][Math]::Round($correctedDecoder.PixelHeight * $correctedScale)
                    $correctedBitmap = Await-Operation ($correctedDecoder.GetSoftwareBitmapAsync(
                        [Windows.Graphics.Imaging.BitmapPixelFormat]::Bgra8,
                        [Windows.Graphics.Imaging.BitmapAlphaMode]::Ignore,$transform,
                        [Windows.Graphics.Imaging.ExifOrientationMode]::IgnoreExifOrientation,
                        [Windows.Graphics.Imaging.ColorManagementMode]::DoNotColorManage)) ([Windows.Graphics.Imaging.SoftwareBitmap])
                    $correctedResult = Await-Operation ($script:engine.RecognizeAsync($correctedBitmap)) ([Windows.Media.Ocr.OcrResult])
                    Add-OcrLines $correctedResult $correctedBitmap $lines $decoder.PixelWidth $decoder.PixelHeight .22
                } finally {
                    if ($correctedBitmap) { $correctedBitmap.Dispose() }
                    if ($correctedStream) { $correctedStream.Dispose() }
                    if ($memory) { $memory.Dispose() }
                    if ($graphics) { $graphics.Dispose() }
                    if ($corrected) { $corrected.Dispose() }
                    if ($original) { $original.Dispose() }
                }
            }
            @{ id = $Id; width = $decoder.PixelWidth; height = $decoder.PixelHeight; lines = @($lines.ToArray()); error = '' }
        } finally {
            if ($bitmap) { $bitmap.Dispose() }
            if ($stream) { $stream.Dispose() }
            $sourceMemory.Dispose()
        }
    }
    if ($Image) {
        Read-ImageText ([System.IO.Path]::GetFullPath($Image)) 1 $Deskew.IsPresent $HighDetail.IsPresent $Numeric.IsPresent | ConvertTo-Json -Depth 8 -Compress
        exit 0
    }
    [Console]::WriteLine('{"ready":true,"language":"zh-Hans"}')
    while ($null -ne ($requestLine = [Console]::ReadLine())) {
        $request = $null
        try {
            $request = $requestLine | ConvertFrom-Json
            if (-not $request.image -or $request.id -lt 1) { throw 'Invalid OCR request' }
            $reply = Read-ImageText ([System.IO.Path]::GetFullPath([string]$request.image)) ([long]$request.id) ([bool]$request.deskew) ([bool]$request.high_detail) ([bool]$request.numeric)
        } catch {
            $reply = @{ id = $request.id; error = $_.Exception.Message }
        }
        [Console]::WriteLine(($reply | ConvertTo-Json -Depth 8 -Compress))
    }
} catch {
    [Console]::WriteLine((@{ ready = $false; error = $_.Exception.Message } | ConvertTo-Json -Compress))
    exit 1
}
