#include "ShaderInfo.h"

void ShaderInfo::get() {
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[0]) );
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,1 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[1]) );
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,2 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[2]) );
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &( MAX_COMPUTE_WORK_GROUP_SIZE[0] ));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &( MAX_COMPUTE_WORK_GROUP_SIZE[1] ));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &( MAX_COMPUTE_WORK_GROUP_SIZE[2] ));
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &MAX_COMPUTE_SHARED_MEMORY_SIZE);

}

void ShaderInfo::printGlobalInfo() {
	ShaderInfo::get();
	DEBUGLOG->log("--- COMPUTE SHADER GLOBAL INFO : ---------------------");
	DEBUGLOG->log("max compute work group invocations : ", MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
	DEBUGLOG->log("max compute work group size x      : ", MAX_COMPUTE_WORK_GROUP_SIZE[0]);
	DEBUGLOG->log("max compute work group size y      : ", MAX_COMPUTE_WORK_GROUP_SIZE[1]);
	DEBUGLOG->log("max compute work group size z      : ", MAX_COMPUTE_WORK_GROUP_SIZE[2]);
	DEBUGLOG->log("max compute work group count x     : ", MAX_COMPUTE_WORK_GROUP_COUNT[0]);
	DEBUGLOG->log("max compute work group count y     : ", MAX_COMPUTE_WORK_GROUP_COUNT[1]);
	DEBUGLOG->log("max compute work group count z     : ", MAX_COMPUTE_WORK_GROUP_COUNT[2]);
	DEBUGLOG->log("max compute shared memory size     : ", MAX_COMPUTE_SHARED_MEMORY_SIZE);
	DEBUGLOG->log("------------------------------------------------------");
}

void ShaderInfo::print(Shader* shader) {
	DEBUGLOG->log("--- SHADER INFO : ------------------------------------");
	DEBUGLOG->log("Shader ProgramHandle : ", shader->getProgramHandle());
	DEBUGLOG->log("Uniforms in this Shader: ", shader->getUniformNames().size() );
	DEBUGLOG->indent();
	for ( std::map<std::string, GLuint>::const_iterator it = shader->getUniformHandles().begin(); it != shader->getUniformHandles().end(); ++it )
	{
		DEBUGLOG->log((*it).first + " : ", (*it).second );
	}
	DEBUGLOG->outdent();
	DEBUGLOG->log("------------------------------------------------------");
}

void ShaderInfo::print(ComputeShader* shader)
{
	DEBUGLOG->log("--- COMPUTE SHADER INFO : ----------------------------");
	DEBUGLOG->log("Shader ProgramHandle : ", shader->getProgramHandle());
	DEBUGLOG->log("Uniforms in this Shader: ", shader->getUniformNames().size() );
	DEBUGLOG->indent();
	for ( std::map<std::string, GLuint>::const_iterator it = shader->getUniformHandles().begin(); it != shader->getUniformHandles().end(); ++it )
	{
		DEBUGLOG->log((*it).first + " : ", (*it).second );
	}
	DEBUGLOG->outdent();

	DEBUGLOG->log("local_group_size_x : ", shader->getLocalGroupSizeX() );
	DEBUGLOG->log("local_group_size_y : ", shader->getLocalGroupSizeY() );
	DEBUGLOG->log("local_group_size_z : ", shader->getLocalGroupSizeZ() );
	DEBUGLOG->log("------------------------------------------------------");
}
