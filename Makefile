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

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	INCS_FLAGS = -I$(VULKAN_SDK)/include \
		-I$(GLFW3_PATH)/include \
		-I $(INCLUDE_PATH)
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
			phosStartVk.cpp \
			vkImpl.cpp \
			phosHelper.cpp \
			command.cpp \
			postPipeline.cpp \
			swapchain.cpp \
			camera.cpp \
			phosphene.cpp \
			allocator.cpp \
			rtTest.cpp

HDRS_NAME =	phosStartVk.hpp \
			vkImpl.hpp \
			phosHelper.hpp \
			command.hpp \
			camera.hpp \
			../shaders/hostDevice.h \
			phosphene.hpp \
			allocator.hpp \
			rtTest.hpp 


OBJS_NAME	=	$(SRCS_NAME:.cpp=.o) 

SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))
HDRS = $(addprefix $(HDRS_PATH)/, $(HDRS_NAME))
OBJS = $(addprefix $(OBJS_PATH)/, $(OBJS_NAME))
SHADERS = $(addprefix $(SHADERS_PATH)/, $(SHADERS_NAME))
SHADERS_RESULT = $(addprefix $(SHADERS_SPV_PATH)/, $(SHADERS_RESULT_NAME))

all: $(NAME) Makefile

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
		$(GLS) -V $(SHADERS_PATH)/$$f -o $(SHADERS_SPV_PATH)/$$f.spv; \
		printf "\e[1A"; \
	done

clean:
	rm -f $(OBJS)
	rm -f $(SHADERS_SPV_PATH)/*.spv

fclean: clean
	rm -f $(NAME)

re: fclean all
