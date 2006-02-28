//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "OGLTile.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
OGLTile::OGLTile(IntRect Extent, IntPoint TexSize, PixelFormat pf, 
        SDLDisplayEngine * pEngine) 
    : m_Extent(Extent),
      m_TexSize(TexSize),
      m_pf(pf),
      m_pEngine(pEngine)
{
    if (m_pf == YCbCr420p) {
        createTexture(0, m_TexSize, I8);
        createTexture(1, m_TexSize/2, I8);
        createTexture(2, m_TexSize/2, I8);
    } else {
        createTexture(0, m_TexSize, m_pf);
    }
}

OGLTile::~OGLTile()
{
    glDeleteTextures(1, &m_TexID[0]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::~OGLTile: glDeleteTextures()");
}

const IntRect& OGLTile::getExtent() const
{
    return m_Extent;
}

const IntPoint& OGLTile::getTexSize() const
{
    return m_TexSize;
}

int OGLTile::getTexID(int i) const
{
    return m_TexID[i];
}

/*
void OGLTile::downloadTextures(BitmapPtr pBmp, int width, 
        OGLMemoryMode MemoryMode) const
{
    if (m_pf == YCbCr420p) {
        downloadTexture(0, pBmp, width, I8, MemoryMode);
    } else {
        downloadTexture(0, pBmp, width, m_pf, MemoryMode);
    }
}
*/

void OGLTile::blt(const DPoint& TLPoint, const DPoint& TRPoint,
        const DPoint& BLPoint, const DPoint& BRPoint) const
{
    double TexWidth;
    double TexHeight;
    int TextureMode = m_pEngine->getTextureMode();

    if (TextureMode == GL_TEXTURE_2D) {
        TexWidth = double(m_Extent.Width())/m_TexSize.x;
        TexHeight = double(m_Extent.Height())/m_TexSize.y;
    } else {
        TexWidth = m_TexSize.x;
        TexHeight = m_TexSize.y;
    }
    
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::bltTile: glBindTexture()");
    if (m_pf == YCbCr420p) {
        GLhandleARB hProgram = m_pEngine->getYCbCr420pShader()->getProgram();
        glUseProgramObjectARB(hProgram);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::bltTile: glUseProgramObjectARB()");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(TextureMode, m_TexID[0]);
        glUniform1iARB(glGetUniformLocationARB(hProgram, "YTexture"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(TextureMode, m_TexID[1]);
        glUniform1iARB(glGetUniformLocationARB(hProgram, "CbTexture"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(TextureMode, m_TexID[2]);
        glUniform1iARB(glGetUniformLocationARB(hProgram, "CrTexture"), 2);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::bltTile: glUniform1iARB()");
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(TextureMode, m_TexID[0]);
        glUseProgramObjectARB(0);
    }
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3d (TLPoint.x, TLPoint.y, 0.0);
    glTexCoord2d(TexWidth, 0.0);
    glVertex3d (TRPoint.x, TRPoint.y, 0.0);
    glTexCoord2d(TexWidth, TexHeight);
    glVertex3d (BRPoint.x, BRPoint.y, 0.0);
    glTexCoord2d(0.0, TexHeight);
    glVertex3d (BLPoint.x, BLPoint.y, 0.0);
    glEnd();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::bltTile: glEnd()");
}

void OGLTile::createTexture(int i, IntPoint Size, PixelFormat pf)
{
    glGenTextures(1, &m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::createTexture: glGenTextures()");
    glBindTexture(m_pEngine->getTextureMode(), m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLTile::createTexture: glBindTexture()");

    glTexImage2D(m_pEngine->getTextureMode(), 0,
            m_pEngine->getOGLDestMode(pf), Size.x, Size.y, 0,
            m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::createTexture: glTexImage2D()");
    
}

static ProfilingZone TexSubImageProfilingZone("    OGLTile::texture download");

void OGLTile::downloadTexture(int i, BitmapPtr pBmp, int width, 
                OGLMemoryMode MemoryMode) const
{
    PixelFormat pf;
    if (m_pf == YCbCr420p) {
        pf = I8;
    } else {
        pf = m_pf;
    }
    IntRect Extent = m_Extent;
    if (i != 0) {
        width /= 2;
        Extent = IntRect(m_Extent.tl/2.0, m_Extent.br/2.0);
    }
    int TextureMode = m_pEngine->getTextureMode();
    glBindTexture(TextureMode, m_TexID[i]);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::downloadTexture: glBindTexture()");
    int bpp = Bitmap::getBytesPerPixel(pf);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGOGLSurface::rebind: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    unsigned char * pStartPos = (unsigned char *)
            (Extent.tl.y*width*bpp + Extent.tl.x*bpp);
    if (MemoryMode == OGL) {
        pStartPos += (unsigned int)(pBmp->getPixels());
    }
    {
        ScopeTimer Timer(TexSubImageProfilingZone);
        glTexSubImage2D(TextureMode, 0, 0, 0, Extent.Width(), Extent.Height(),
                m_pEngine->getOGLSrcMode(pf), m_pEngine->getOGLPixelType(pf), 
                pStartPos);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLTile::downloadTexture: glTexSubImage2D()");
    
}

}
