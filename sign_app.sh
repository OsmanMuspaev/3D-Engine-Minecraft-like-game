#!/bin/bash

APP="build/3D-game.app"

echo "🔥 Агрессивная очистка $APP"

# 1. Удаляем всё лишнее
find "$APP" -type f \( -name ".DS_Store" -o -name "._*" -o -name "Thumbs.db" -o -name "desktop.ini" -o -name "Icon?" -o -name ".localized" \) -delete 2>/dev/null
find "$APP" -type d -name "__MACOSX" -exec rm -rf {} + 2>/dev/null

# 2. Очищаем все extended attributes
xattr -rc "$APP" 2>/dev/null

# 3. Удаляем ACL
chmod -R -N "$APP" 2>/dev/null

# 4. Сбрасываем immutable flag
chflags -R nouchg "$APP" 2>/dev/null

# 5. Удаляем конкретные problematic атрибуты
find "$APP" -exec xattr -d com.apple.FinderInfo {} \; 2>/dev/null
find "$APP" -exec xattr -d com.apple.ResourceFork {} \; 2>/dev/null

# 6. Создаем копию без resource fork (если проблема в них)
cp -R "$APP" "$APP.tmp" 2>/dev/null
rm -rf "$APP"
mv "$APP.tmp" "$APP"

# 7. Финальная очистка
xattr -rc "$APP" 2>/dev/null

# 8. Пробуем подписать
echo "🔐 Пробуем подписать..."
if codesign --force -s - "$APP" 2>&1; then
    echo "✅ Успешно подписано!"
    codesign --verify --verbose "$APP"
else
    echo "❌ Ошибка подписи. Попробуйте пересобрать приложение с флагами:"
    echo "   - Для Unity: Включите 'Strip Engine Code' в настройках сборки"
    echo "   - Для Xcode: Включите 'Remove Debug Symbols' в схеме Archive"
    echo "   - Для CMake: Добавьте -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"
fi