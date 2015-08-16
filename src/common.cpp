#include "common.h"

#include <cstdio>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#ifndef __APPLE_CC__
#include <SDL2/SDL_image.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <CoreServices/CoreServices.h>
#endif

#include "loader/level.h"

static int screenshot_cnt = 0;

void Com_Init()
{
}

void Com_Destroy()
{
}

#ifdef __APPLE_CC__
static void ReleaseScreenshotData(void *info, const void *data,
                                  size_t size)
{
}
#endif

void Com_TakeScreenShot()
{
    GLint ViewPort[4];
    glGetIntegerv(GL_VIEWPORT, ViewPort);

    char fname[128];
    snprintf(fname, 128, "screen_%05d.png", screenshot_cnt);

    const auto str_size = ViewPort[2] * 4;

    std::vector<GLubyte> pixels(str_size * ViewPort[3]);
    glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
#ifdef __APPLE_CC__
    CGColorSpaceRef deviceRgb = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(pixels.data(), pixels.data(), str_size * ViewPort[3], ReleaseScreenshotData);
    CGImageRef image = CGImageCreate(ViewPort[2], ViewPort[3], 32, 8, str_size * ViewPort[3], deviceRgb, kCGImageAlphaLast, dataProvider, nullptr, false, kCGRenderingIntentDefault);
    CGDataProviderRelease(dataProvider);
    CGColorSpaceRelease(deviceRgb);

    CFStringRef pathString = CFStringCreateWithBytes(kCFAllocatorDefault, (const Uint8*)fname, strlen(fname), kCFStringEncodingASCII, FALSE);
    CFURLRef destinationUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, pathString, kCFURLPOSIXPathStyle, FALSE);
    CFRelease(pathString);
    CGDataConsumerRef consumer = CGDataConsumerCreateWithURL(destinationUrl);
    CFRelease(destinationUrl);
    CGImageDestinationRef imageDestination = CGImageDestinationCreateWithDataConsumer(consumer, kUTTypePNG, 1, nullptr);
    CGDataConsumerRelease(consumer);
    CGImageDestinationAddImage(imageDestination, image, nullptr);
    CGImageRelease(image);
    CGImageDestinationFinalize(imageDestination);
    CFRelease(imageDestination);

#else
    std::vector<GLubyte> buf(str_size);
    for(int h = 0; h < ViewPort[3] / 2; h++)
    {
        memcpy(buf.data(), &pixels[h * str_size], str_size);
        memcpy(&pixels[h * str_size], &pixels[(ViewPort[3] - h - 1) * str_size], str_size);
        memcpy(&pixels[(ViewPort[3] - h - 1) * str_size], buf.data(), str_size);
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(nullptr, ViewPort[2], ViewPort[3], 32, str_size, 0x000000FF, 0x00000FF00, 0x00FF0000, 0xFF000000);
    surface->format->format = SDL_PIXELFORMAT_RGBA8888;
    surface->pixels = pixels.data();
    IMG_SavePNG(surface, fname);

    surface->pixels = nullptr;
    SDL_FreeSurface(surface);
#endif
    screenshot_cnt++;
}
