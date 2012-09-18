#pragma once
#include <QtGui/QImage>

namespace Sprites {

QImage getSprite(int tileType, int scaleFactor = 1);
QSize getSpritesBounds();

};
