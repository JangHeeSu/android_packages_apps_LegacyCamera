#include "Renderer.h"

#include <GLES2/gl2ext.h>

#include <android/log.h>
#define  LOG_TAG    "Renderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

Renderer::Renderer()
      : mGlProgram(0),
        mInputTextureName(-1),
        mInputTextureWidth(0),
        mInputTextureHeight(0),
        mSurfaceWidth(0),
        mSurfaceHeight(0)
{
    InitializeGLContext();
}

Renderer::~Renderer() {
}

GLuint Renderer::loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint Renderer::createProgram(const char* pVertexSource, const char* pFragmentSource)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
    {
        return 0;
    }
    LOGI("VertexShader Loaded!");

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
    {
        return 0;
    }
    LOGI("FragmentShader Loaded!");

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");

        LOGI("Shaders Attached!");

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        LOGI("Program Linked!");

        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = (char*) malloc(bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

// Set this renderer to use the default frame-buffer (screen) and
// set the viewport size to be the given width and height (pixels).
bool Renderer::SetupGraphics(int width, int height)
{
    bool succeeded = false;
    do {
        if (mGlProgram == 0)
        {
            if (!InitializeGLProgram())
            {
              break;
            }
        }
        glUseProgram(mGlProgram);
        if (!checkGlError("glUseProgram")) break;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mFrameBuffer = NULL;
        mSurfaceWidth = width;
        mSurfaceHeight = height;

        glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
        if (!checkGlError("glViewport")) break;
        succeeded = true;
    } while (false);

    return succeeded;
}


// Set this renderer to use the specified FBO and
// set the viewport size to be the width and height of this FBO.
bool Renderer::SetupGraphics(FrameBuffer* buffer)
{
    bool succeeded = false;
    do {
        if (mGlProgram == 0)
        {
            if (!InitializeGLProgram())
            {
              break;
            }
        }
        glUseProgram(mGlProgram);
        if (!checkGlError("glUseProgram")) break;

        glBindFramebuffer(GL_FRAMEBUFFER, buffer->GetFrameBufferName());

        mFrameBuffer = buffer;
        mSurfaceWidth = mFrameBuffer->GetWidth();
        mSurfaceHeight = mFrameBuffer->GetHeight();

        glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
        if (!checkGlError("glViewport")) break;
        succeeded = true;
    } while (false);

    return succeeded;
}

bool Renderer::Clear(float r, float g, float b, float a)
{
    bool succeeded = false;
    do {
        bool rt = (mFrameBuffer == NULL)?
                SetupGraphics(mSurfaceWidth, mSurfaceHeight) :
                SetupGraphics(mFrameBuffer);

        if(!rt)
            break;

        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);

        succeeded = true;
    } while (false);
    return succeeded;

}

void Renderer::InitializeGLContext()
{
    if(mFrameBuffer != NULL)
    {
        delete mFrameBuffer;
        mFrameBuffer = NULL;
    }

    mInputTextureName = -1;
    mInputTextureType = GL_TEXTURE_2D;
    mGlProgram = 0;
}

int Renderer::GetTextureName()
{
    return mInputTextureName;
}

void Renderer::SetInputTextureName(GLuint textureName)
{
    mInputTextureName = textureName;
}

void Renderer::SetInputTextureType(GLenum textureType)
{
    mInputTextureType = textureType;
}

void Renderer::SetInputTextureDimensions(int width, int height)
{
    mInputTextureWidth = width;
    mInputTextureHeight = height;
}
