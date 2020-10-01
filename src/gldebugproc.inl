
#if defined( MSWIN )
static void CALLBACK
#else
static void
#endif
gl_debug_proc( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *data )
{
	(void) id;
	(void) data;

	const char *source_str=0, *type_str=0, *severity_str=0;

	switch(source) {
	case GL_DEBUG_SOURCE_API:
		source_str = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source_str = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source_str = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source_str = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source_str = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:
		source_str = "Other"; break;
	default:
		source_str = "Unknown";
	}

	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		type_str = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type_str = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type_str = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type_str = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type_str = "Performance"; break;
	case GL_DEBUG_TYPE_OTHER:
		type_str = "Other"; break;
	default: 
		type_str = "Unknown";
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		severity_str = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severity_str = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:
		severity_str = "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		severity_str = "Notification"; break;
	default:
		severity_str = "Unknown";
	}

	const bool warn_only = 
		type == GL_DEBUG_TYPE_PERFORMANCE ||
		type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ||
		severity == GL_DEBUG_SEVERITY_NOTIFICATION ||
		severity == GL_DEBUG_SEVERITY_LOW;

	if ( !strstr( message, "usage hint is" ) )
	{
		if ( warn_only )
		{
			LOGW
			(
				"[%s][%s]{%s}: %.*s",
				source_str, type_str, severity_str,
				length, message
			);
		}
		else
		{
			LOGE
			(
				"[%s][%s]{%s}: %.*s",
				source_str, type_str, severity_str,
				length, message
			);
		}
	}
}

