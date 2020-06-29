#include "Context.h"
#include "Engine.h"
#include "Log.h"

namespace Atlas {

	Context::Context(SDL_Window* window) : window(window) {

		context = SDL_GL_CreateContext(window);

		LocalAPISetup();

		isCreated = true;

	}

	Context::~Context() {

		SDL_GL_DeleteContext(context);

	}

	void Context::AttachTo(Window* window) {

		this->window = window->GetSDLWindow();

		if (isCreated) {
			SDL_GL_MakeCurrent(this->window, context);
		}
		else {
			context = SDL_GL_CreateContext(this->window);
			LocalAPISetup();

			isCreated = true;
		}

	}

	void Context::Bind() {

		SDL_GL_MakeCurrent(window, context);

	}

	void Context::Unbind() {

		SDL_GL_MakeCurrent(nullptr, nullptr);

	}

	void* Context::Get() {

		return context;

	}

	void Context::LocalAPISetup() {

		// If the textures aren't working as expected this line should be changed
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// If texture data isn't returned as expected this line should be changed
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

#ifdef AE_API_GL
		// Standard in OpenGL ES
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glDisable(GL_DITHER);

		// For debugging
		glLineWidth(1.0f);

#ifdef AE_SHOW_API_DEBUG_LOG
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Context::DebugCallback, this);
#endif

	}

	void Context::DebugCallback(GLenum source, GLenum type, GLuint ID, GLenum severity,
		GLsizei length, const GLchar* message, const void* userParam) {

		int32_t logType = Log::Type::TYPE_MESSAGE, logSeverity = Log::Severity::SEVERITY_LOW;

		// Filter notifications
		if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
			return;
		
		auto context = static_cast<const Context*>(userParam);

		std::string output = "OpenGL debug log:\nContext name: "
			 + context->name + "\nGenerated by: ";

		switch (source) {
		case GL_DEBUG_SOURCE_API: output.append("OpenGL API"); break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: output.append("Window-system API"); break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: output.append("Shader compiler");  break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: output.append("Third party"); break;
		case GL_DEBUG_SOURCE_APPLICATION: output.append("Atlas Engine"); break;
		case GL_DEBUG_SOURCE_OTHER: output.append("Unknown source"); break;
		default: break;
		}

		output.append("\nType: ");

		switch (type) {
		case GL_DEBUG_TYPE_ERROR: output.append("Error");  
			logType = Log::Type::TYPE_ERROR;  break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: output.append("Deprecated behavior"); 
			logType = Log::Type::TYPE_WARNING; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: output.append("Undefined behavior");
			logType = Log::Type::TYPE_WARNING; break;
		case GL_DEBUG_TYPE_PORTABILITY: output.append("Bad portability"); 
			logType = Log::Type::TYPE_WARNING; break;
		case GL_DEBUG_TYPE_PERFORMANCE: output.append("Performance issue"); 
			logType = Log::Type::TYPE_WARNING; break;
		case GL_DEBUG_TYPE_OTHER: output.append("Unknown"); 
			logType = Log::Type::TYPE_MESSAGE; break;
		default: break;
		}

		output.append("\nSeverity: ");

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH: output.append("High"); 
			logSeverity = Log::Severity::SEVERITY_HIGH; break;
		case GL_DEBUG_SEVERITY_MEDIUM: output.append("Medium");  
			logSeverity = Log::Severity::SEVERITY_MEDIUM; break;
		case GL_DEBUG_SEVERITY_LOW: output.append("Low");  
			logSeverity = Log::Severity::SEVERITY_LOW; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: output.append("Notification");  
			logSeverity = Log::Severity::SEVERITY_LOW; break;
		default: break;
		}

		output.append("\nObject ID: " + std::to_string(ID));

		output.append("\nMessage: " + std::string(message));

		switch (logType) {
		case Log::Type::TYPE_MESSAGE: Log::Message(message, logSeverity); break;
		case Log::Type::TYPE_WARNING: Log::Warning(message, logSeverity); break;
		case Log::Type::TYPE_ERROR: Log::Error(message, logSeverity); break;
		}

	}

}