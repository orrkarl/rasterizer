#pragma once


#include "../general/predefs.h"
#include "../utils/converters.h"

namespace nr
{

class Shader
{
public:

    Shader(
        const std::string& code,
        const NRbool compile,
        const Role shaderType,
        Error& err)
        : m_type(shaderType),
          m_shader(glCreateShader(utils::fromNRShaderType(shaderType))),
          m_isCompiled(compile)
    {
        auto str = code.c_str();
        GLint len = code.size();
        glShaderSource(m_shader, 1, &str, &len);

        if (compile)
        {
            glCompileShader(m_shader);
            if ((err = utils::getLastGLError()) != Error::NO_ERROR)
            {
                GLint success;
                glGetShaderiv(m_shader, GL_COMPILE_STATUS, &success);
                
                if (success == GL_FALSE)
                {
                    err = Error::INVALID_VALUE;
                    release();
                }
            }
        }
    }

    ~Shader() { release(); }

    Error compile() 
    {
        if (m_isCompiled) return Error::NO_ERROR;

        Error err = Error::NO_ERROR;
        glCompileShader(m_shader);
        if ((err = utils::getLastGLError()) == Error::NO_ERROR)
        {
            GLint success;
            glGetShaderiv(m_shader, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE)
            {
                err = Error::INVALID_VALUE;
            }
        }

        m_isCompiled = error::isSuccess(err) ? true : false;
        return err;
    }

    void release()
    {
        glDeleteShader(m_shader);
    }

    NRbool isCompiled() const { return m_isCompiled; }

    GLuint getContent() const { return m_shader; }

    Role getShaderType() const { return m_type; }

private:
    const GLuint m_shader;
    const Role m_type;
    NRbool m_isCompiled;

};

}