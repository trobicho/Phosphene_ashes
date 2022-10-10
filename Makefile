CC = g++
GLS = glslangValidator --target-env vulkan1.3
CXXFLAGS	=	-std=c++20 -g

NAME = phosphene 

INCLUDE_PATH 		= /home/tom/projects/lib
SRCS_PATH			= ./src
HDRS_PATH			= ./src
OBJS_PATH			= ./obj
SHADERS_PATH		= ./shaders
SHADERS_SPV_PATH	= ./spv
EXTERNAL_LIB_PATH	= ./external_lib

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	INCS_FLAGS = -I$(VULKAN_SDK)/include \
		-I$(GLFW3_PATH)/include \
		-I$(INCLUDE_PATH) \
		-I$(EXTERNAL_LIB_PATH)
	LDFLAGS = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan -lm
else
	LDFLAGS = -L$(GLFW3_PATH)/lib -L$(VULKAN_SDK)/lib `pkg-config --static --libs glm` -lvulkan -lglfw -lm
endif

SHADERS_NAME =	post.vert \
                post.frag \
				raytrace.rgen \
				raytrace.rmiss \
				raytrace.rchit \

SHADERS_RESULT_NAME =	$(addsuffix .spv, $(SHADERS_NAME))

SRCS_NAME =	main.cpp \
			phosphene.cpp \
			camera.cpp \
			phospheneCallback.cpp \
			backend/phosStartVk.cpp \
			backend/vkImpl.cpp \
			backend/postPipeline.cpp \
			backend/swapchain.cpp \
			helper/command.cpp \
			helper/allocator.cpp \
			helper/phosHelper.cpp \
			raytracing/rtTest.cpp \
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
			raytracing/rtTest.hpp  \
			../shaders/hostDevice.h \
			sceneLoader/sceneLoader.hpp \
			sceneLoader/objLoader.hpp \
			sceneLoader/scene.hpp \

EXTERNAL_LIB_NAME = json/json.hpp \
					imgui/imgui.h \
					tinyobjloader/tiny_obj_loader.h

OBJS_NAME	=	$(SRCS_NAME:.cpp=.o) 

SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))
HDRS = $(addprefix $(HDRS_PATH)/, $(HDRS_NAME))
OBJS = $(addprefix $(OBJS_PATH)/, $(OBJS_NAME))
SHADERS = $(addprefix $(SHADERS_PATH)/, $(SHADERS_NAME))
SHADERS_RESULT = $(addprefix $(SHADERS_SPV_PATH)/, $(SHADERS_RESULT_NAME))
EXTERNAL_LIBS = $(addprefix $(EXTERNAL_LIB_PATH)/, $(EXTERNAL_LIB_NAME))

all: $(EXTERNAL_LIBS) $(NAME) $(SHADERS) $(SHADERS_RESULT) Makefile

$(NAME): $(SRCS) $(HDRS) $(OBJS) Makefile
	$(CC) $(CXXFLAGS) $(INCS_FLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)

$(OBJS_PATH)/%.o: $(SRCS_PATH)/%.cpp $(HDRS) Makefile
	@test -d $(OBJS_PATH) || mkdir -pm 775 $(OBJS_PATH)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	@echo -e "\033[38;2;0;255;0m[cc]\033[0m: $< -> $@"
	@printf "\e[1A"
	@$(CC) $(CXXFLAGS) -I $(HDRS_PATH) $(INCS_FLAGS) -c $< -o $@
	@printf "\e[0K"

shaders: $(SHADERS_RESULT) Makefile

$(SHADERS_RESULT): $(SHADERS) Makefile
	@test -d $(SHADERS_SPV_PATH) || mkdir -pm 775 $(SHADERS_SPV_PATH)
	@for f in $(SHADERS_NAME); do \
		echo -e "\033[38;2;0;255;0m[$(GLS)]\033[0m: $$f -> $$f.spv"; \
		printf "\e[1A"; \
		printf "\e[0K"; \
		$(GLS) -V $(SHADERS_PATH)/$$f -o $(SHADERS_SPV_PATH)/$$f.spv; \
	done

external_lib: $(EXTERNAL_LIBS) Makefile

$(EXTERNAL_LIB_PATH)/json/json.hpp:
	@test -d $(EXTERNAL_LIB_PATH) || mkdir -pm 775 $(EXTERNAL_LIB_PATH)
	@test -d $(EXTERNAL_LIB_PATH)/json || mkdir -pm 775 $(EXTERNAL_LIB_PATH)/json
	@echo -e "\033[38;2;0;255;0m[download nlohmann/json]\033[0m"
	@if ! [ -f "$(EXTERNAL_LIB_PATH)/json/json.hpp" ] ; then \
		wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp ;\
		mv ./json.hpp ./$(EXTERNAL_LIB_PATH)/json/ ; \
	fi
	@printf "\e[1A"
	@printf "\e[0K"

$(EXTERNAL_LIB_PATH)/imgui/imgui.h:
	@echo -e "\033[38;2;0;255;0m[download imgui]\033[0m"
	@test -d $(EXTERNAL_LIB_PATH)/imgui || mkdir -pm 775 $(EXTERNAL_LIB_PATH)/imgui
	@if ! [ -f "$(EXTERNAL_LIB_PATH)/imgui/imgui.h" ] ; then \
		wget https://github.com/ocornut/imgui/archive/refs/heads/master.zip ; \
		unzip ./master.zip -d $(EXTERNAL_LIB_PATH)/imgui/ \
			-x imgui-master/.git* imgui-master/.editor* \
			imgui-master/examples/* imgui-master/misc/* imgui-master/docs/* \
			imgui-master/backends/imgui_impl_a* \
			imgui-master/backends/imgui_impl_d* \
			imgui-master/backends/imgui_impl_m* \
			imgui-master/backends/imgui_impl_o* \
			imgui-master/backends/imgui_impl_s* \
			imgui-master/backends/imgui_impl_w* \
			imgui-master/backends/imgui_impl_glut* ; \
		rm ./master.zip ; \
		mv $(EXTERNAL_LIB_PATH)/imgui/imgui-master/* $(EXTERNAL_LIB_PATH)/imgui/ ; \
		mv $(EXTERNAL_LIB_PATH)/imgui/backends/vulkan $(EXTERNAL_LIB_PATH)/imgui/ ; \
		mv $(EXTERNAL_LIB_PATH)/imgui/backends/imgui_impl_vulkan.* $(EXTERNAL_LIB_PATH)/imgui/ ; \
		rm -r $(EXTERNAL_LIB_PATH)/imgui/imgui-master/ ; \
		rm -r $(EXTERNAL_LIB_PATH)/imgui/backends/ ; \
	fi
	@printf "\e[1A"
	@printf "\e[0K"

$(EXTERNAL_LIB_PATH)/tinyobjloader/tiny_obj_loader.h:
	@echo -e "\033[38;2;0;255;0m[download tinyobjloader]\033[0m"
	@test -d $(EXTERNAL_LIB_PATH)/tinyobjloader || mkdir -pm 775 $(EXTERNAL_LIB_PATH)/tinyobjloader
	@if ! [ -f "$(EXTERNAL_LIB_PATH)/tinyobjloader/tiny_obj_loader.h" ] ; then \
		wget https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/master/tiny_obj_loader.h ; \
		mv ./tiny_obj_loader.h ./$(EXTERNAL_LIB_PATH)/tinyobjloader/ ; \
	fi
	@printf "\e[1A"
	@printf "\e[0K"

clean:
	rm -f $(OBJS)
	rm -f $(SHADERS_SPV_PATH)/*.spv

fclean: clean
	rm -f $(NAME)
	rm -rf $(EXTERNAL_LIB_PATH)

re: fclean all
