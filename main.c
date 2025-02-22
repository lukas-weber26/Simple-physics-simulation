#include <assert.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define ball_radious 0.04

const char * vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec2 transform;\n" 
    "void main()\n"
    "{\n"
    "gl_Position = vec4(transform, 0.0, 0.0) + vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char * fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "FragColor = vec4(1.0,0.5,0.2,1.0);\n"
    "}\0";

void checkLink(unsigned int shaderProgram);
void checkSuccess(unsigned int vertexShader);
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);

float random_float () {
    return (float) rand() / (float)(RAND_MAX/0.5);
}

    typedef struct ball {
	float x;
	float y; 
	float v_x;
	float v_y; 
	float a_x;
	float a_y; 
	float m;
    } ball;

    void update_ball_position(ball * b) {
	b->v_x += b->a_x*0.1;
	b->x += b->v_x*0.1;
	b->v_y += b->a_y*0.1;
	b->y += b->v_y*0.1;
    }

    void check_ball_bounce(ball * b) {
	if (b->x <= -(1-ball_radious)) {
	    b ->v_x = - b-> v_x;
	    b -> x = -(1-ball_radious);
	}
	if (b->x >=(1-ball_radious)) {
	    b ->v_x = - b-> v_x;
	    b -> x = (1-ball_radious);
	}
	if (b->y <= -(1-ball_radious)) {
	    b ->v_y = - b-> v_y;
	    b ->y = -(1-ball_radious);
	}
	if (b->y >= (1-ball_radious)) {
	    b ->v_y = - b-> v_y;
	    b ->y = (1-ball_radious);
	}

    }

void check_ball_collisions(ball balls [], int num_balls) {
    for (int main_ball = 0; main_ball < num_balls; main_ball++) {
	for (int secondary_ball= main_ball+1; secondary_ball< num_balls; secondary_ball++) {

	    double x_distance = balls[main_ball].x - balls[secondary_ball].x;  
	    double y_distance = balls[main_ball].y - balls[secondary_ball].y;  
	    double dist = sqrt(x_distance * x_distance + y_distance * y_distance);
	    if (dist <= 2*ball_radious) {
    

		ball first = balls[main_ball];
		ball second = balls[secondary_ball];

		float v_first_final_x = (first.m - second.m)/(first.m + second.m)*first.v_x + 2* (second.m/(first.m+second.m))*second.v_x;
		float v_second_final_x = 2*(first.m/(first.m+second.m))*first.v_x - ((first.m-second.m)/(first.m + second.m))*second.v_x;
		
		float v_first_final_y = (first.m - second.m)/(first.m + second.m)*first.v_y + 2* (second.m/(first.m+second.m))*second.v_y;
		float v_second_final_y = 2*(first.m/(first.m+second.m))*first.v_y - ((first.m-second.m)/(first.m + second.m))*second.v_y;
    
		balls[main_ball].v_x = v_first_final_x;
		balls[secondary_ball].v_x = v_second_final_x;
		
		balls[main_ball].v_y = v_first_final_y;
		balls[secondary_ball].v_y = v_second_final_y;


	    } 
	}
    }
}

int main()
{

    //these seem like pretty normal c library things! 
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //fortunately glew does not require too much crap

    GLFWwindow * window = glfwCreateWindow(800, 800, "First Window", NULL, NULL);
    if (window == NULL) {
	printf("GLFW Window fail.");
	glfwTerminate();
	exit(1);
    }

    //binds gl functions to the current window
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwPollEvents(); //this seems to be an issue with wayland 
    
    glViewport(0, 0, 800, 800);
    
    //YEAH TURNS OUT THE POSITION OF THIS MATTERS A LOT. HAS TO BE AFTER THE WINDOW HAS BEEN CREATED!
    glewExperimental = GL_TRUE; 
    glewInit();

    #define number_of_vertices 36 

    float vertices[3*(number_of_vertices+2)] = {
	0.0, 0.0, 0.0,
    };

    for (int i = 1; i < number_of_vertices + 2; i++) {
	vertices[3*i] = ball_radious*cos(2*M_PI* ((float)(i-1)/(float)(number_of_vertices)));
	vertices[3*i + 1] = ball_radious*sin(2*M_PI* ((float)(i-1)/(float)(number_of_vertices)));
	vertices[3*i + 2] = 0.0;
    }

    //tell gl to generate vertex arrays and bind them
    unsigned int VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
     
    unsigned int VBO;
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO); //you have to bind buffers to make the following gl operations affect said buffer
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW); //note that GL_STATIC_DRAW means that this is not expected to change much
   
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    glShaderSource(vertexShader,1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkSuccess(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkSuccess(fragmentShader);

    unsigned int shaderProgram; 
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    //final step here: Make the gpu interpret the shader program in a meaningful way!
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glEnable(GL_MULTISAMPLE);
    
    int number_balls = 30;
    int transform_loc = glGetUniformLocation(shaderProgram,"transform");
    ball balls[number_balls];
    
    for (int i = 0; i < number_balls; i++) {
	ball b = {random_float(),random_float(),random_float(),random_float(),0.0,-0.05,random_float()};
	balls[i] = b;
    }

    while(!glfwWindowShouldClose(window)) {
	processInput(window);
	
	glClearColor(0.14f, 0.14f, 0.14f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	
	for (int i = 0; i < number_balls; i++) {
	    glUniform2f(transform_loc,balls[i].x, balls[i].y);
	    glDrawArrays(GL_TRIANGLE_FAN, 0, number_of_vertices + 2);
	    update_ball_position(&balls[i]);
	    check_ball_bounce(&balls[i]);
	}
	
	check_ball_collisions(balls, number_balls);
	
	glfwSwapBuffers(window);
	glfwPollEvents(); //this seems to be an issue with wayland 

    }
    
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { 
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow * window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);	
    }
}

void checkSuccess(unsigned int vertexShader) {
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char infoLog[512];
	glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
	printf("%s\n",infoLog);
	exit(0);
    }
}

void checkLink(unsigned int shaderProgram) {
    int success;
    glGetProgramiv(shaderProgram,GL_LINK_STATUS, &success);
    if (!success) {
	char out[512];
	glGetProgramInfoLog(shaderProgram,512,NULL,out);
	printf("%s",out);
	exit(0);
    }
}
