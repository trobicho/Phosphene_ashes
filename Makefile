CMAKE = cmake --build build
GLS = glslangValidator --target-env vulkan1.3

NAME = phosphene 

INCLUDE_PATH 		= /home/tom/projects/lib
SHADERS_PATH		= ./shaders
SHADERS_SPV_PATH	= ./spv

UNAME := $(shell uname)


SHADERS_NAME =	post.vert \
                post.frag \
				raytrace.rgen \
				raytrace.rmiss \
				raytraceShadow.rmiss \
				raytraceMesh.rchit \
				raytraceMeshShadow.rchit \
				raytraceShape.rchit \
				raytraceShapeShadow.rchit \
				sphere.rint \
				mandelbulb.rint \
				menger.rint \

SHADERS_RESULT_NAME =	$(addsuffix .spv, $(SHADERS_NAME))

SRCS_NAME =	main.cpp \
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
			sceneLoader/sceneLoader.cpp \
			sceneLoader/objLoader.cpp \
			sceneLoader/scene.cpp \

HDRS_NAME =	phosphene.hpp \
			camera.hpp \
			backend/phosStartVk.hpp \
			backend/vkImpl.hpp \
			helper/phosHelper.hpp \
			helper/command.hpp \
			helper/allocator.hpp \
			helper/extensions.hpp \
			raytracing/sceneBuilder.hpp \
			raytracing/pipelineBuilder.hpp \
			sceneLoader/sceneLoader.hpp \
			sceneLoader/objLoader.hpp \
			sceneLoader/scene.hpp \
			../shaders/hostDevice.h \

SHADERS = $(addprefix $(SHADERS_PATH)/, $(SHADERS_NAME))
SHADERS_RESULT = $(addprefix $(SHADERS_SPV_PATH)/, $(SHADERS_RESULT_NAME))

all: $(NAME) $(SHADERS) $(SHADERS_RESULT) Makefile

$(NAME): Makefile
	$(CMAKE)

shaders: $(SHADERS_RESULT) Makefile

$(SHADERS_RESULT): $(SHADERS) Makefile
	@test -d $(SHADERS_SPV_PATH) || mkdir -pm 775 $(SHADERS_SPV_PATH)
	@for f in $(SHADERS_NAME); do \
		echo -e "\033[38;2;0;255;0m[$(GLS)]\033[0m: $$f -> $$f.spv"; \
		printf "\e[1A"; \
		printf "\e[0K"; \
		$(GLS) -V $(SHADERS_PATH)/$$f -o $(SHADERS_SPV_PATH)/$$f.spv; \
	done

clean:
	rm -f $(SHADERS_SPV_PATH)/*.spv

fclean: clean

re: fclean all
