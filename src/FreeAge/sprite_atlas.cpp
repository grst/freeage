#include "FreeAge/sprite_atlas.h"

#include "FreeAge/logging.h"
#include "FreeAge/sprite.h"
#include "FreeAge/timing.h"
#include "RectangleBinPack/MaxRectsBinPack.h"

using namespace rbp;

SpriteAtlas::SpriteAtlas(Mode mode)
    : mode(mode) {}

SpriteAtlas::~SpriteAtlas() {}

void SpriteAtlas::AddSprite(Sprite* sprite) {
  sprites.push_back(sprite);
}

bool SpriteAtlas::BuildAtlas(int width, int height, int borderPixels) {
  Timer packTimer("SpriteAtlas::BuildAtlas packing");
  
  atlasWidth = width;
  atlasHeight = height;
  atlasBorderPixels = borderPixels;
  
  // TODO: Should we allow flipping? It is currently not implemented for texture coordinate setting in rendering.
  MaxRectsBinPack packer(width, height, /*allowFlip*/ false);
  
  // TODO: Right now, we only pack the sprites' main graphics.
  //       We should pack the shadows too (into another texture, since they are only 8 bit per pixel).
  
  int numRects = 0;
  for (Sprite* sprite : sprites) {
    numRects += sprite->NumFrames();
  }
  
  std::vector<RectSize> rects(numRects);
  int index = 0;
  for (Sprite* sprite : sprites) {
    for (int frameIdx = 0; frameIdx < sprite->NumFrames(); ++ frameIdx) {
      const QImage& image =
          (mode == Mode::Graphic) ?
          sprite->frame(frameIdx).graphic.image :
          sprite->frame(frameIdx).shadow.image;
      rects[index] = RectSize{image.width() + 2 * borderPixels, image.height() + 2 * borderPixels};
      ++ index;
    }
  }
  
  packedRects.clear();
  packedRectIndices.clear();
  // TODO: Choose the packing rule that is best.
  //       RectBottomLeftRule is much faster than RectBestShortSideFit, so it is more convenient for testing, unless we cache the results.
  packer.Insert(rects, packedRects, packedRectIndices, MaxRectsBinPack::RectBottomLeftRule);  // RectBestShortSideFit
  packTimer.Stop();
  return rects.empty();
}

bool SpriteAtlas::Save(const char* path) {
  FILE* file = fopen(path, "wb");
  if (!file) {
    return false;
  }
  
  fwrite(&atlasWidth, sizeof(int), 1, file);
  fwrite(&atlasHeight, sizeof(int), 1, file);
  fwrite(&atlasBorderPixels, sizeof(int), 1, file);
  int numRects = packedRects.size();
  fwrite(&numRects, sizeof(int), 1, file);
  
  for (int i = 0; i < numRects; ++ i) {
    const rbp::Rect& rect = packedRects[i];
    fwrite(&rect.x, sizeof(int), 1, file);
    fwrite(&rect.y, sizeof(int), 1, file);
    fwrite(&rect.width, sizeof(int), 1, file);
    fwrite(&rect.height, sizeof(int), 1, file);
    fwrite(&packedRectIndices[i], sizeof(int), 1, file);
  }
  
  fclose(file);
  return true;
}

bool SpriteAtlas::Load(const char* path, int expectedNumRects) {
  FILE* file = fopen(path, "rb");
  if (!file) {
    return false;
  }
  
  if (fread(&atlasWidth, sizeof(int), 1, file) != 1) { fclose(file); return false; }
  if (fread(&atlasHeight, sizeof(int), 1, file) != 1) { fclose(file); return false; }
  if (fread(&atlasBorderPixels, sizeof(int), 1, file) != 1) { fclose(file); return false; }
  int numRects;
  if (fread(&numRects, sizeof(int), 1, file) != 1) { fclose(file); return false; }
  if (numRects != expectedNumRects) {
    return false;
  }
  packedRects.resize(numRects);
  packedRectIndices.resize(numRects);
  
  for (int i = 0; i < numRects; ++ i) {
    rbp::Rect& rect = packedRects[i];
    if (fread(&rect.x, sizeof(int), 1, file) != 1) { fclose(file); return false; }
    if (fread(&rect.y, sizeof(int), 1, file) != 1) { fclose(file); return false; }
    if (fread(&rect.width, sizeof(int), 1, file) != 1) { fclose(file); return false; }
    if (fread(&rect.height, sizeof(int), 1, file) != 1) { fclose(file); return false; }
    if (fread(&packedRectIndices[i], sizeof(int), 1, file) != 1) { fclose(file); return false; }
  }
  
  fclose(file);
  return true;
}

QImage SpriteAtlas::RenderAtlas() {
  Timer paintTimer("SpriteAtlas::BuildAtlas rendering");
  
  // Invert packedRectIndices
  std::vector<int> originalToPackedIndex(packedRects.size());
  for (std::size_t i = 0; i < packedRectIndices.size(); ++ i) {
    originalToPackedIndex[packedRectIndices[i]] = i;
  }
  
  // Draw all images into their assigned rects.
  QImage atlas(atlasWidth, atlasHeight, (mode == Mode::Graphic) ? QImage::Format_ARGB32 : QImage::Format_Grayscale8);
  // Clear the atlas to have clean borders around the sprites
  atlas.fill(qRgba(0, 0, 0, 0));  // this sets the values to 0 for QImage::Format_Grayscale8.
  
  int index = 0;
  for (Sprite* sprite : sprites) {
    for (int frameIdx = 0; frameIdx < sprite->NumFrames(); ++ frameIdx) {
      Sprite::Frame::Layer& layer =
          (mode == Mode::Graphic) ?
          sprite->frame(frameIdx).graphic :
          sprite->frame(frameIdx).shadow;
      QImage& image = layer.image;
      const Rect& packedRect = packedRects[originalToPackedIndex[index]];
      
      layer.atlasX = packedRect.x + atlasBorderPixels;
      layer.atlasY = packedRect.y + atlasBorderPixels;
      
      int packedWidthWithoutBorder = packedRect.width - 2 * atlasBorderPixels;
      int packedHeightWithoutBorder = packedRect.height - 2 * atlasBorderPixels;
      
      if (packedWidthWithoutBorder == image.width() && packedHeightWithoutBorder == image.height()) {
        layer.rotated = false;
        
        // Draw the image directly into the assigned rect.
        if (mode == Mode::Graphic) {
          for (int y = 0; y < image.height(); ++ y) {
            const QRgb* inputScanline = reinterpret_cast<const QRgb*>(image.scanLine(y));
            QRgb* outputScanline = reinterpret_cast<QRgb*>(atlas.scanLine(layer.atlasY + y));
            for (int x = 0; x < image.width(); ++ x) {
              outputScanline[layer.atlasX + x] = inputScanline[x];
            }
          }
        } else {
          for (int y = 0; y < image.height(); ++ y) {
            const u8* inputScanline = reinterpret_cast<const u8*>(image.scanLine(y));
            u8* outputScanline = reinterpret_cast<u8*>(atlas.scanLine(layer.atlasY + y));
            for (int x = 0; x < image.width(); ++ x) {
              outputScanline[layer.atlasX + x] = inputScanline[x];
            }
          }
        }
      } else if (packedWidthWithoutBorder == image.height() && packedHeightWithoutBorder == image.width()) {
        layer.rotated = true;
        
        // Draw the image into the assigned rect while rotating it by 90 degrees (to the right).
        for (int y = 0; y < image.height(); ++ y) {
          const QRgb* inputScanline = reinterpret_cast<const QRgb*>(image.scanLine(y));
          for (int x = 0; x < image.width(); ++ x) {
            // TODO: Speed this up with raw QRgb or u8 access
            atlas.setPixelColor(layer.atlasX + image.height() - y, layer.atlasY + x, inputScanline[x]);
          }
        }
      } else {
        // Something went wrong.
        LOG(ERROR) << "Internal error: The size of the rect assigned to a sprite frame is incorrect.";
        return QImage();
      }
      
      // Unload the sprite image since it should not be needed anymore.
      image = QImage();
      
      // Dilate the sprite colors by one (without touching the alpha channel).
      // This prevents us from getting an ugly influence of the black (color value: (0, 0, 0, 0)) background
      // when using bilinear filtering to access pixels on the border of the sprite.
      // TODO: Use integral images for better performance
      if (mode == Mode::Graphic) {
        for (int y = 0; y < packedRect.height; ++ y) {
          QRgb* outputScanline = reinterpret_cast<QRgb*>(atlas.scanLine(packedRect.y + y));
          
          for (int x = 0; x < packedRect.width; ++ x) {
            int alpha = qAlpha(outputScanline[packedRect.x + x]);
            if (alpha > 1) {
              continue;
            }
            
            int minX = std::max(0, x - 1);
            int minY = std::max(0, y - 1);
            int maxX = std::min(packedRect.width - 1, x + 1);
            int maxY = std::min(packedRect.height - 1, y + 1);
            
            int redSum = 0;
            int greenSum = 0;
            int blueSum = 0;
            int count = 0;
            for (int sy = minY; sy <= maxY; ++ sy) {
              for (int sx = minX; sx <= maxX; ++ sx) {
                QRgb rgba = *(reinterpret_cast<QRgb*>(atlas.scanLine(packedRect.y + sy)) + (packedRect.x + sx));
                if (qAlpha(rgba) <= 1) {
                  continue;
                }
                
                redSum += qRed(rgba);
                greenSum += qGreen(rgba);
                blueSum += qBlue(rgba);
                ++ count;
              }
            }
            
            if (count > 0) {
              float factor = 1.f / count;
              outputScanline[packedRect.x + x] =
                  qRgba(redSum * factor + 0.5f, greenSum * factor + 0.5f, blueSum * factor + 0.5f, alpha);
            }
          }
        }
      }
      
      ++ index;
    }
  }
  
  return atlas;
}
