@echo off
for %%f in (*.wav) do (
    ffmpeg -i "%%f" -codec:a libmp3lame -q:a 2 "%%~nf.mp3"
)
