#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
std::random_device rd;
std::mt19937 gen(rd());
float randomFloat(float max, float min)
{


    std::uniform_real_distribution<float> dist(min, max);


    return (std::round(dist(gen) * 1000.0f)) / 1000.0f;
}
struct Circle {
    unsigned int VAO, VBO;
    int vertexCount;
    float scaleX, scaleY;
    float ogX, ogY;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const float MoveRange = 0.35f;
const float ColorRange = 1.0f;
const float ScaleRange = 0.2f;
Circle createCircle(float centerX, float centerY, float radius, int numSegments)
{

    Circle c;
    std::vector<float> vertices;

    // CENTER of the fan
    vertices.push_back(centerX);
    vertices.push_back(centerY);
    vertices.push_back(0.0f);

    c.scaleX = randomFloat(ScaleRange +1.0f,1.0f);
    c.scaleY = randomFloat(ScaleRange + 1.0f, 1.0f);

    c.ogX = centerX;
    c.ogY = centerY;

    for (int i = 0; i <= numSegments; i++) // <= to close the circle
    {
        float angle = 2.0f * 3.1415926f * i / numSegments;
        vertices.push_back(centerX + radius * cos(angle));
        vertices.push_back(centerY + radius * sin(angle));
        vertices.push_back(0.0f);
    }

    c.vertexCount = numSegments + 2; // center + ring + closing vertex


    glGenVertexArrays(1, &c.VAO);
    glGenBuffers(1, &c.VBO);

    glBindVertexArray(c.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, c.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return c;
}

float absoluteCircle = 0.10f;
float changingCircle = 0.5f;
void drawCircle(const Circle& c, Shader& shader, float moux, float mouy)
{
    shader.use();
    float time = glfwGetTime();

    float dx = moux - c.ogX;
    float dy = mouy - c.ogY;
    float distance = std::sqrt(dx * dx + dy * dy);


    float t = (distance - absoluteCircle) / changingCircle;
    t = std::clamp(t, 0.0f, 1.0f);

    // smooth
    t = t * t * (3.0f - 2.0f * t);


    glm::mat4 transform = glm::mat4(1.0f);
    // scale
    float pulse = 0.01f * sin(time * 3.0f + c.ogX * 20.0f);
    float sx = 1.0f + (c.scaleX - 1.0f) * t + pulse * t;
    float sy = 1.0f + (c.scaleY - 1.0f) * t + pulse * t;

    transform = glm::scale(
        transform,
        glm::vec3(sx, sy, 1.0f)
    );

    float wobble = 0.13f;
    float angle = time + (c.ogX + c.ogY * 10.0f);

    float ox = cos(angle) * wobble * t;
    float oy = sin(angle) * wobble * t;


    // move
    transform = glm::translate(
        transform,
        glm::vec3(ox,oy, 0.0f)
    );
    float worldx = c.ogX + ox;
    float worldY = c.ogY + oy;

    float y = (worldY * 9.0f + worldx * 1.0f) + time;


    float r = ((-1.0f * cos(y)) * 1.5f) - 0.7f;
    float g = ((-1.0f * cos(y))) / 6.0f + 0.75f;
    float b = ((-1.0f * cos(y))) /6.0f + 0.85f;


    float ogr = 0.968f;
    float ogg = 0.850f;
    float ogb = 0.658f;
    r = ogr + (r - ogr) * t;
    g = ogg + (g - ogg) * t;
    b = ogb + (b - ogb) * t;



    glUniform4f(
        glGetUniformLocation(shader.ID, "ourColor"),
        r, g, b, 1.0f
    );

    // upload transform
    glUniformMatrix4fv(
        glGetUniformLocation(shader.ID, "transform"),
        1,
        GL_FALSE,
        glm::value_ptr(transform)
    );

    // --- DRAW ---
    glBindVertexArray(c.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, c.vertexCount);
    glBindVertexArray(0);
}
void destroyCircle(Circle& c)
{
    glDeleteVertexArrays(1, &c.VAO);
    glDeleteBuffers(1, &c.VBO);
}

int main()
{
    double mouseX, mouseY;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    std::vector<Circle> circles;
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    int rowamount = 30;
    int colamount = 30;
    float topleftX = -0.9f;
    float toplefltY = 0.9f;
    float bottomrightX = 0.9f;
    float bottomrightY = -0.9f;
    float lenght = std::abs(topleftX - 0) + std::abs(bottomrightX - 0);
    float radiant = lenght / (2.0f * rowamount) ;

    int segmen = 8;
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------



    for (int col = 0; col < colamount; col++) {
        for (int row = 0; row < rowamount; row++) {

            float cX = topleftX + row * (2.0f * radiant) + radiant;
            float cY = toplefltY - (col * (2.0f * radiant) + radiant);

            circles.push_back(
                createCircle(cX, cY, radiant, segmen)
            );
        }
    }


    // on shutdown

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        //
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float moux = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
        float mouy = 1.0f - (2.0f * mouseY) / SCR_HEIGHT;
        glClearColor(0.137f,  0.192f,  0.40f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& c : circles)
            drawCircle(c, ourShader,moux,mouy);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    for (auto& c : circles)
        destroyCircle(c);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}