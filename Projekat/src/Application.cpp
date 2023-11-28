#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <vector>

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);

float dayCloudColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float nightCloudColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
float paintedHeight = 65.0f;
float cloudOffsetX = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isNight = false;
bool addPressed = false;
bool subtractPressed = false;

int numApples = 5;
int numCirclesToShow = 5;
const int numSmallCircles = 10;

float smallCircleCenters[numSmallCircles][2] = {
    {0.4f, -0.3f}, {0.44f, -0.3f}, {0.7f, -0.1f},
    {0.65f, -0.1f}, {0.43f, -0.1f}, {0.50f, -0.15f},
    {0.6f, -0.0f}, {0.54f, -0.0f}, {0.65f, -0.4f},{0.5f, -0.4f}
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
 
void processInput(GLFWwindow* window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 

    float heightChangeSpeed = 80.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        paintedHeight += heightChangeSpeed * deltaTime;
        if (paintedHeight > 300.0f) {
            paintedHeight = 300.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        paintedHeight -= heightChangeSpeed * deltaTime;
        if (paintedHeight < 65.0f) {
            paintedHeight = 65.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if (!addPressed && numCirclesToShow < numSmallCircles) {
            numCirclesToShow++;
            addPressed = true;
        }
    }
    else {
        addPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if (!subtractPressed && numCirclesToShow > 0) {
            numCirclesToShow--;
            subtractPressed = true;
        }
    }
    else {
        subtractPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        isNight = true; 
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        isNight = false; 
    }

}

int initGLFW() {
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* createWindow(const char* title, int width, int height) {
    GLFWwindow* window = glfwCreateWindow(width, height, title, glfwGetPrimaryMonitor(), nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    return window;
}

void initGLEW() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }
}

void setupCallbacks(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

GLuint generateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void setupVAOandVBO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t size) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setupVAOandVBOTexture(GLuint& VAO, GLuint& VBO, const float* vertices, size_t size, unsigned int stride) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint loadAndSetupTexture(const char* imagePath, GLuint shaderProgram, const char* uniformName) {
  
    GLuint textureID = loadImageToTexture(imagePath);
    if (textureID == 0) {
        std::cerr << "Failed to load texture: " << imagePath << std::endl;
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Postavljanje uniforme za teksturu u shader programu
    unsigned uTexLoc = glGetUniformLocation(shaderProgram, uniformName);
    glUniform1i(uTexLoc, 0);

    return textureID;
}

void drawTree() {
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLES, 4, 3);
    glDrawArrays(GL_TRIANGLES, 7, 3);
    glDrawArrays(GL_TRIANGLES, 10, 3);
    glDrawArrays(GL_TRIANGLES, 13, 3);
    glDrawArrays(GL_TRIANGLES, 16, 3);
    glDrawArrays(GL_TRIANGLES, 19, 3);
    glDrawArrays(GL_TRIANGLES, 22, 3);
    glDrawArrays(GL_TRIANGLES, 25, 3);
    glDrawArrays(GL_TRIANGLES, 28, 3);
}

void drawGeometry(GLuint shaderProgram, GLuint VAO, GLenum primitiveType, GLint start, GLsizei count) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(primitiveType, start, count);
    glBindVertexArray(0);
}

int main(void)
{
    initGLFW();
    GLFWwindow* window = createWindow("OpenGL Landscape", 1920, 1080);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glfwMakeContextCurrent(window);
    initGLEW();
    setupCallbacks(window);

    GLuint shaderProgram = createShader("src/vert/valley.vert", "src/frag/valley.frag");
    GLuint shaderProgramGreen = createShader("src/vert/valley.vert", "src/frag/forest.frag");
    GLuint shaderProgramTree = createShader("src/vert/valley.vert", "src/frag/tree.frag");
    GLuint shaderProgramCrown = createShader("src/vert/valley.vert", "src/frag/crown.frag");
    GLuint shaderProgramMoon = createShader("src/vert/valley.vert", "src/frag/moon.frag");
    GLuint shaderProgramSignature = createShader("src/vert/valley.vert", "src/frag/signature.frag");
    GLuint shaderProgramCircle = createShader("src/vert/sun.vert", "src/frag/sun.frag");
    GLuint shaderProgramA = createShader("src/vert/apple.vert", "src/frag/apple.frag");
    GLuint shaderProgramCloud = createShader("src/vert/apple.vert", "src/frag/cloud.frag");
    
    unsigned int VAO[12];
    glGenVertexArrays(12, VAO);
    unsigned int VBO[12];
    glGenBuffers(12, VBO);

    //0 DOLINA---------------------------------------------------------------------
    
    const int numPoints = 100;
    float vertices[numPoints * 2 + 6];
    vertices[0] = 0;
    vertices[1] = -1.0;
    vertices[2] = -1.0;
    vertices[3] = -1.0;
    for (int i = 0; i < numPoints; ++i) {
        float x = static_cast<float>(i) / (numPoints - 1) * 2.0 - 1.0;
        float y = -sin(x * 3.14) * 0.2 - 0.4;
        vertices[i * 2 + 4] = x;
        vertices[i * 2 + 5] = y;
    }
    vertices[numPoints * 2 + 4] = 1.0;
    vertices[numPoints * 2 + 5] = -1.0;

    setupVAOandVBO(VAO[0], VBO[0], vertices, sizeof(vertices));

    //1 SUMA----------------------------------------------------------------------
    
    float triangleVertices[] = {
        -0.7f, 0.1f,
        -0.8f, -0.4f,
        -0.6f, -0.4f,

        -0.6f, 0.0f,
        -0.7f, -0.45f,
        -0.5f, -0.45f,

        -0.5f, -0.1f,
        -0.55f, -0.5f,
        -0.45f, -0.5f,

        -0.8f, -0.1f,
        -0.85f, -0.5f,
        -0.75f, -0.5f,

        -0.85f, -0.2f,
        -0.9f, -0.45f,
        -0.8f, -0.45f,

        -0.60f, -0.15f,
        -0.67f, -0.55f,
        -0.53f, -0.55f,

        -0.45f, -0.3f,
        -0.5f, -0.53f,
        -0.4f, -0.53f

    };
    setupVAOandVBO(VAO[1], VBO[1], triangleVertices, sizeof(triangleVertices));

    //2 STABLO----------------------------------------------------------------------
    
    float treeBaseVertices[] = {
        0.5f, -0.88f,  
        0.57f, -0.88f, 
        0.57f, -0.5f,  
        0.5f, -0.5f, 

            // Trougao 1
        0.5f, -0.5f,
        0.53f, -0.5f,
        0.4f, -0.3f, // vrh trougla
    
        0.47f, -0.43f,
        0.48f, -0.43f,
        0.44f, -0.3f,
    
        0.55f, -0.5f,
        0.571, -0.5f,
        0.7f, -0.1f,

        0.60f, -0.36f,
        0.62f, -0.34f,
        0.65f, -0.1f, 

        0.52f, -0.5f,
        0.54, -0.5f,
        0.43f, -0.1f, 

        0.49f, -0.35f,
        0.502, -0.39f,
        0.50f, -0.15f, 

        0.54f, -0.5f,
        0.56, -0.5f,
        0.6f, -0.0f,

        0.575f, -0.21f,
        0.582, -0.19f,
        0.54f, -0.0f, 

        0.57f, -0.45f,
        0.556, -0.49f,
        0.65f, -0.4f, 

    };
    setupVAOandVBO(VAO[2], VBO[2], treeBaseVertices, sizeof(treeBaseVertices));
    glBindVertexArray(VAO[2]);
    glDrawArrays(GL_QUADS, 0, 4);

    //3 KRUG------------------------------------------------------------------------

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspectRatio = width / (float)height;

    const float PI = 3.14159265358979323846f;
    const int circlePoints = 100; 
    float radius = 0.35f;         
    float centerX = 0.535f;       
    float centerY = -0.17f;       
    float circleVertices[circlePoints * 2];
    for (int i = 0; i < circlePoints; ++i) {
        float angle = 2.0f * PI * float(i) / float(circlePoints);
        circleVertices[i * 2] = centerX + (radius * cos(angle) / aspectRatio);
        circleVertices[i * 2 + 1] = centerY + radius * sin(angle);
    }
    setupVAOandVBO(VAO[3], VBO[3], circleVertices, sizeof(circleVertices));

  
    //4 SUNCE--------------------------------------------------------------------------------
    float centerX2 = -0.8f;
    float centerY2 = 0.7f;
    float radius2 = 0.1f; 
    const int circlePoints2 = 100;
    float sunVertices[circlePoints2 * 2];
    for (int i = 0; i < circlePoints2; ++i) {
        float angle = 2.0f * PI * float(i) / float(circlePoints2);
        sunVertices[i * 2] = centerX2 + (radius2 * cos(angle) / aspectRatio);
        sunVertices[i * 2 + 1] = centerY2 + radius2 * sin(angle);
    };
    setupVAOandVBO(VAO[4], VBO[4], sunVertices, sizeof(sunVertices));

    
    //5 ZVIJEZDICE--------------------------------------------------------------------------------
    const int numberOfStars = 100;
    float starVertices[numberOfStars * 2];
    srand(time(NULL));
    for (int i = 0; i < numberOfStars; ++i) {
        starVertices[i * 2] = (float)rand() / RAND_MAX * 2.0f - 1.0f; // x koordinata između -1.0 i 1.0
        starVertices[i * 2 + 1] = (float)rand() / RAND_MAX; // y koordinata između 0.0 i 1.0 (samo gornja polovina)
    }
    setupVAOandVBO(VAO[5], VBO[5], starVertices, sizeof(starVertices));

    //6 POTPIS--------------------------------------------------------------------------------------

    GLuint unifiedShader = createShader("src/vert/basic.vert", "src/frag/basic.frag");
    float signatureVertices[] =
    { 1.0f, -1.0f,  1.0f, 0.0f,
     1.0f, -0.85f,  1.0f, 1.0f,
     0.7f, -1.0f,  0.0f, 0.0f,
     0.7f, -0.85f,  0.0f, 1.0f
    };
    
    unsigned int stride = (2 + 2) * sizeof(float);
    setupVAOandVBOTexture(VAO[6], VBO[6], signatureVertices, sizeof(signatureVertices), stride);

    GLuint checkerTexture = loadAndSetupTexture("src/res/potpis.png", unifiedShader, "uTex");

    //7 JABUKE-------------------------------------------------------------------------
    
    const int numberOfPointsInCircle = 100;
    float circleVerticesApple[numberOfPointsInCircle * 2];
    float radius3 = 0.07f;
    float textureCoords[numberOfPointsInCircle * 2];

    for (int i = 0; i < numberOfPointsInCircle; ++i) {
        float angle = 2.0f * PI * i / numberOfPointsInCircle;
        circleVerticesApple[i * 2] = radius3 * cos(angle)/ aspectRatio; 
        circleVerticesApple[i * 2 + 1] = radius3 * sin(angle); 
        textureCoords[i * 2] = (cos(angle) + 1.0f) / 2.0f;
        textureCoords[i * 2 + 1] = (sin(angle) + 1.0f) / 2.0f;
    }

    glBindVertexArray(VAO[7]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVerticesApple), circleVerticesApple, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    GLuint appleTexture = loadAndSetupTexture("src/res/jabuka.png", shaderProgramA, "appleTexture");
    
    //8 OBLACI------------------------------------------------------------------------------

    const int numberOfPointsInCircleCloud = 100;
    float radiusCloud = 0.1f;
    const int numCloudCirclesCloud = 15;
    float cloudCircleCenters[numCloudCirclesCloud][2] = {
        {-0.12f, 0.85f}, {-0.2f, 0.8f},{-0.03f, 0.8f},
        {-0.15f, 0.74f}, {-0.08f, 0.74f},
               
    {-0.12f - 0.6f, 0.85f - 0.3f}, {-0.2f - 0.6f, 0.8f - 0.3f}, {-0.03f - 0.6f, 0.8f - 0.3f},
    {-0.15f - 0.6f, 0.74f - 0.3f}, {-0.08f - 0.6f, 0.74f - 0.3f},
        
    {-0.12f + 0.6f, 0.85f - 0.3f}, {-0.2f + 0.6f, 0.8f - 0.3f}, {-0.03f + 0.6f, 0.8f - 0.3f},
    {-0.15f + 0.6f, 0.74f - 0.3f}, {-0.08f + 0.6f, 0.74f - 0.3f}
                
    };
    float circleVerticesCloud[numberOfPointsInCircleCloud * 2];
    for (int i = 0; i < numberOfPointsInCircleCloud; ++i) {
        float angle = 2.0f * PI * i / numberOfPointsInCircleCloud;
        circleVerticesCloud[i * 2] = radiusCloud * cos(angle)/ aspectRatio; 
        circleVerticesCloud[i * 2 + 1] = radiusCloud * sin(angle);
    }
    setupVAOandVBO(VAO[8], VBO[9], circleVerticesCloud, sizeof(circleVerticesCloud));

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        if (isNight) {
            glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        }
        else {
            glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
        }  
        glClear(GL_COLOR_BUFFER_BIT);

        // DOLINA        
        glLineWidth(10.0);
        drawGeometry(shaderProgram, VAO[0], GL_TRIANGLE_FAN, 0, numPoints + 3);

        // SUMA
        drawGeometry(shaderProgramGreen, VAO[1], GL_TRIANGLES, 0, 21);

        // STABLO + GRANE
        glUseProgram(shaderProgramTree);
        unsigned int paintedHeightLoc = glGetUniformLocation(shaderProgramTree, "paintedHeight");
        glUniform1f(paintedHeightLoc, paintedHeight);
        glBindVertexArray(VAO[2]);
        drawTree();
        glBindVertexArray(0);

        // KRUG
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(shaderProgramCrown);
        glUniform3f(glGetUniformLocation(shaderProgramCrown, "circleColor"), 0.0f, 1.0f, 0.0f);
        drawGeometry(shaderProgramCrown, VAO[3], GL_TRIANGLE_FAN, 0, circlePoints);
      
        // JABUKE
        glUseProgram(shaderProgramA); 
        glBindVertexArray(VAO[7]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, appleTexture);
        GLint circleCenterLoc = glGetUniformLocation(shaderProgramA, "circleCenter");
        for (int i = 0; i < numCirclesToShow; i++) {
            glUniform2f(circleCenterLoc, smallCircleCenters[i][0], smallCircleCenters[i][1]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfPointsInCircle);
        }
        glBindVertexArray(0);
                
        // NEBO I ZVIJEZDE
        if (isNight) {

            glUseProgram(shaderProgramMoon);
            glUniform2f(glGetUniformLocation(shaderProgramMoon, "center"), centerX2, centerY2);

            glBindVertexArray(VAO[4]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, circlePoints2);
            glBindVertexArray(0);

            glUseProgram(shaderProgramMoon);
            glBindVertexArray(VAO[5]);
            glPointSize(2.0f);
            glDrawArrays(GL_POINTS, 0, numberOfStars);
            glBindVertexArray(0);
        }
        else {
            glUseProgram(shaderProgramCircle);

            float timeValue = glfwGetTime();
            float sizeFactor = 1.0f + 0.1f * sin(timeValue * 5.0f); // Treperi između 0.9 i 1.1

            glUniform1f(glGetUniformLocation(shaderProgramCircle, "sizeFactor"), sizeFactor);

            glUniform2f(glGetUniformLocation(shaderProgramCircle, "center"), centerX2, centerY2);

            glUniform1f(glGetUniformLocation(shaderProgramCircle, "time"), timeValue);

            glBindVertexArray(VAO[4]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, circlePoints2);
            glBindVertexArray(0);
        }

        // POTPIS
        glUseProgram(unifiedShader);
        glBindVertexArray(VAO[6]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, checkerTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // OBLACI
        cloudOffsetX += 0.01f;
        if (cloudOffsetX > 1.5f) {
            cloudOffsetX = -1.5f; 
        }

        glUseProgram(shaderProgramCloud);
        if (isNight) {
            glUniform4fv(glGetUniformLocation(shaderProgramCloud, "cloudColor"), 1, nightCloudColor);
        }
        else {
            glUniform4fv(glGetUniformLocation(shaderProgramCloud, "cloudColor"), 1, dayCloudColor);
        }

        glBindVertexArray(VAO[8]);
        for (int i = 0; i < numCloudCirclesCloud; i++) {
            glUniform2f(glGetUniformLocation(shaderProgramCloud, "circleCenter"),
                cloudCircleCenters[i][0] + cloudOffsetX, cloudCircleCenters[i][1]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfPointsInCircle);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window); 
        glfwPollEvents();        
    }

  
    glDeleteBuffers(12, VBO);
    glDeleteVertexArrays(12, VAO);

    glfwTerminate();
    return 0;
}

GLuint compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        char bom[3] = { 0 };
        file.read(bom, 3);

        // Proverite da li se radi o BOM-u (EF BB BF za UTF-8)
        if (!(bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF)) {
            // Ako nije BOM, vratite se na početak
            file.seekg(0);
        }
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        
    }
    return shader;
}
GLuint createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;
    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);


    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}