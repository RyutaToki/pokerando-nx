# Cargar con: source env.sh
export DEVKITPRO=/opt/devkitpro
export DEVKITA64=$DEVKITPRO/devkitA64
export PROJ="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
export JAVA_HOME="$PROJ/.tools/jdk"
export PATH="$PROJ/.tools/venv/bin:$JAVA_HOME/bin:$DEVKITPRO/tools/bin:$DEVKITA64/bin:$PATH"
