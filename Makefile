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
				raytraceShapeNormal.rchit \
				raytraceShapeColor.rchit \
				raytraceShapeShadow.rchit \
				sphere.rint \
				marching/mandelbulb.rint \
				marching/menger.rint \
				marching/mandelbox.rint \

SHADERS_RESULT_NAME =	$(addsuffix .spv, $(notdir $(SHADERS_NAME)))


SHADERS = $(addprefix $(SHADERS_PATH)/, $(SHADERS_NAME))
SHADERS_RESULT = $(addprefix $(SHADERS_SPV_PATH)/, $(SHADERS_RESULT_NAME))

all: $(NAME) $(SHADERS) $(SHADERS_RESULT) Makefile

$(NAME): Makefile
	$(CMAKE)

shaders: $(SHADERS_RESULT) Makefile

$(SHADERS_RESULT): $(SHADERS) Makefile
	@test -d $(SHADERS_SPV_PATH) || mkdir -pm 775 $(SHADERS_SPV_PATH)
	@for f in $(SHADERS_NAME); do \
		b=`echo -e "$$f" | sed -e 's/.*\///'`; \
		echo -e "\033[38;2;0;255;0m[$(GLS)]\033[0m: $$f -> $$b.spv"; \
		$(GLS) -V $(SHADERS_PATH)/$$f -o $(SHADERS_SPV_PATH)/$$b.spv; \
		printf "\e[1A"; \
		printf "\e[0K"; \
	done

clean:
	rm -f $(SHADERS_SPV_PATH)/*.spv

fclean: clean

re: fclean all
