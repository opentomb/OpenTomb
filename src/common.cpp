#include <cstdio>
#include <SDL2/SDL.h>

#ifndef __APPLE_CC__
#include <SDL2/SDL_image.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <CoreServices/CoreServices.h>
#endif

#include "vt/vt_level.h"

#include "system.h"

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
    free(info);
}
#endif

void Com_TakeScreenShot()
{
    GLint ViewPort[4];
    char fname[128];
    GLubyte *pixels;
#ifndef __APPLE_CC__
    SDL_Surface *surface;
#endif
    uint32_t str_size;

    glGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, 128, "screen_%05d.png", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = static_cast<GLubyte*>(malloc(str_size * ViewPort[3]));
    glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#ifdef __APPLE_CC__
    CGColorSpaceRef deviceRgb = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(pixels, pixels, str_size * ViewPort[3], ReleaseScreenshotData);
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
    GLubyte buf[str_size];
    for(int h = 0; h < ViewPort[3] / 2; h++)
    {
        memcpy(buf, pixels + h * str_size, str_size);
        memcpy(pixels + h * str_size, pixels + (ViewPort[3] - h - 1) * str_size, str_size);
        memcpy(pixels + (ViewPort[3] - h - 1) * str_size, buf, str_size);
    }
    surface = SDL_CreateRGBSurfaceFrom(nullptr, ViewPort[2], ViewPort[3], 32, str_size, 0x000000FF, 0x00000FF00, 0x00FF0000, 0xFF000000);
    surface->format->format = SDL_PIXELFORMAT_RGBA8888;
    surface->pixels = pixels;
    IMG_SavePNG(surface, fname);

    surface->pixels = nullptr;
    SDL_FreeSurface(surface);
    free(pixels);
#endif
    screenshot_cnt++;
}