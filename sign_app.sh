#!/bin/bash

APP="build/3D-game.app"

echo "Cleaning $APP ..."
find "$APP" -type f \( -name ".DS_Store" -o -name "._*" -o -name "Thumbs.db" \) -delete 2>/dev/null
find "$APP" -type d -name "__MACOSX" -exec rm -rf {} + 2>/dev/null
xattr -rc "$APP" 2>/dev/null
chmod -R -N "$APP" 2>/dev/null
chflags -R nouchg "$APP" 2>/dev/null

echo "Signing $APP ..."
codesign --force --deep --sign - "$APP"
codesign --verify --verbose "$APP"
