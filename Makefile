CMAKE = cmake --build build
GLS = glslangValidator --target-env vulkan1.3

NAME = phosphene 

INCLUDE_PATH 		= /home/tom/projects/lib
SHADERS_PATH		= ./shaders
SHADERS_SPV_PATH	= ./spv
SRCS_PATH			= ./src
HDRS_PATH			= ./src

UNAME := $(shell uname)

SRCS_NAME = main.cpp \
			phosphene.cpp \
			camera.cpp \
			phospheneCallback.cpp \
			phosphenePipeline.cpp \
			backend/phosStartVk.cpp \
			backend/vkImpl.cpp \
			backend/postPipeline.cpp \
			backend/swapchain.cpp \
			helper/command.cpp \
			helper/allocator.cpp \
			helper/phosHelper.cpp \
			helper/extensions.cpp \
			raytracing/sceneBuilder.cpp \
			raytracing/pipelineBuilder.cpp \
			raytracing/pipeline.cpp \
			raytracing/rayPicker.cpp \
			sceneLoader/sceneLoader.cpp \
			sceneLoader/objLoader.cpp \
			sceneLoader/scene.cpp \
			phosGui.cpp \

HDRS_NAME = phosphene.hpp \
    		camera.hpp \
			backend/phosStartVk.hpp \
			backend/vkImpl.hpp \
			helper/phosHelper.hpp \
			helper/command.hpp \
			helper/allocator.hpp \
			helper/extensions.hpp \
			raytracing/sceneBuilder.hpp \
			raytracing/pipelineBuilder.hpp \
			raytracing/rayPicker.hpp \
			sceneLoader/sceneLoader.hpp \
			sceneLoader/objLoader.hpp \
			sceneLoader/scene.hpp \
			../shaders/hostDevice.h \
			../shaders/hostDevicePicker.h \

SHADERS_NAME =	post.vert \
                post.frag \
				raytrace.rgen \
				raytrace.rmiss \
				raytraceShadow.rmiss \
				raytraceMesh.rchit \
				raytraceMeshShadow.rchit \
				raytraceShapeNormal.rchit \
				raytraceShapeColor.rchit \
				raytraceShapeShadow.rchit \
				pathtrace.rgen \
				pathtrace.rmiss \
				pathtraceMesh.rchit \
				pathtraceShape.rchit \
				sphere.rint \
				marching/mandelbulb.rint \
				marching/menger.rint \
				marching/mandelbox.rint \
				marching/test.rint \
				marching/test2.rint \

SHADER_DEPS_NAME = marching/marching.glsl \
					marching/marchingFold.glsl \
					marching/marchingBasicShape.glsl \
					marching/marchingBasicSD.glsl \
					marching/marchingDefault.glsl \
					rand.glsl \

SHADERS_RESULT_NAME =	$(addsuffix .spv, $(notdir $(SHADERS_NAME)))

SHADERS = $(addprefix $(SHADERS_PATH)/, $(SHADERS_NAME))
SHADER_DEPS = $(addprefix $(SHADERS_PATH)/, $(SHADER_DEPS_NAME))
SHADERS_RESULT = $(addprefix $(SHADERS_SPV_PATH)/, $(SHADERS_RESULT_NAME))
SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))
HDRS = $(addprefix $(HDRS_PATH)/, $(HDRS_NAME))

all: $(NAME) $(SHADERS) $(SHADERS_RESULT) Makefile

$(NAME): $(SRCS) $(HDRS) Makefile
	$(CMAKE)

shaders: $(SHADERS_RESULT) $(SHADER_DEPS) Makefile

$(SHADERS_RESULT): $(SHADERS) $(SHADER_DEPS) Makefile
	@test -d $(SHADERS_SPV_PATH) || mkdir -pm 775 $(SHADERS_SPV_PATH)
	@for f in $(SHADERS_NAME); do \
		b=`echo -e "$$f" | sed -e 's/.*\///'`; \
		echo -e "\033[38;2;0;255;0m[$(GLS)]\033[0m: $$f -> $$b.spv"; \
		printf "\e[1A"; \
		printf "\e[0K"; \
		$(GLS) -V $(SHADERS_PATH)/$$f -o $(SHADERS_SPV_PATH)/$$b.spv; \
	done

clean:
	rm -f $(SHADERS_SPV_PATH)/*.spv

fclean: clean

re: fclean all
