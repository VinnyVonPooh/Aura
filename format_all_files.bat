@echo off

for /r %%f in (*.cpp) do (
    clang-format -i "%%f"
    echo %%f
)

for /r %%f in (*.h) do (
    clang-format -i "%%f"
    echo %%f
)

pause
