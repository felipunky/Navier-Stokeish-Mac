#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include <glm.hpp>

#include "Shader.h"
//#include "wtypes.h"
#include <time.h>
#include <iostream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <sstream>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

boost::filesystem::path find_executable()
{
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);
    
#if defined(_WIN32)
    ::GetModuleFileName(NULL, &buffer[0], bufferSize);
    
#elif defined(__linux__)
    // Get the process ID.
    int pid = getpid();
    
    // Construct a path to the symbolic link pointing to the process executable.
    // This is at /proc/<pid>/exe on Linux systems (we hope).
    std::ostringstream oss;
    oss << "/proc/" << pid << "/exe";
    std::string link = oss.str();
    
    // Read the contents of the link.
    int count = readlink(link.c_str(), &buffer[0], bufferSize);
    if(count == -1) throw std::runtime_error("Could not read symbolic link");
    buffer[count] = '\0';
    
#elif defined(__APPLE__)
    if(_NSGetExecutablePath(&buffer[0], &bufferSize))
    {
        buffer.resize(bufferSize);
        _NSGetExecutablePath(&buffer[0], &bufferSize);
    }
    
#else
#error Cannot yet find the executable on this platform
#endif
    
    std::string s = &buffer[0];
    return s;
}

// FPS, iFrame counter.
int initialTime = time( NULL ), finalTime, frameCount, frames, FPS;
const char* title;
// Mouse.
static double xPre, yPre;
double xPos, yPos, xDif, yDif, xAce, yAce, vX, vY;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Our image size.
const unsigned int SRC_WIDTH = 600;
const unsigned int SRC_HEIGHT = 400;

int WIDTH, HEIGHT;

// We need this to be able to resize our window.
void framebuffer_size_callback( GLFWwindow* window, int width, int height );
// We need this to be able to close our window when pressing a key.
void processInput( GLFWwindow* window );
// We need this to be able to call our load image function from below main.
unsigned int loadTexture( const char *path );

// Our mouse button click flag.
float pressed = 0.0, right_pressed = 0.0;

// Our mouse.
static void cursorPositionCallback( GLFWwindow *window, double xPos, double yPos );
// Our mouse button press.
static void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods );

int main()
{
    
    std::cout << "Type yes if you have a retina display otherwise type no: ";
    
    std::string retina;
    
    std::cin >> retina;
    
    // We initialize glfw and specify which versions of OpenGL to target.
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    
    // Our window object.
    GLFWwindow* window = glfwCreateWindow( SRC_WIDTH, SRC_HEIGHT, "NavierStokeish", NULL, NULL );
    if ( window == NULL )
    {
        
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
        
    }
    
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    
    // Initialize the mouse.
    glfwSetCursorPosCallback( window, cursorPositionCallback );
    // Initialize the mouse button.
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
    
    // Load glad, we need this library to specify system specific functions.
    if( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
    {
        
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
        
    }
    
    boost::filesystem::path path = find_executable().parent_path();
    
    std::string vertexShaderString = path.string() + "/Resources/Shaders/vertex.glsl";
    char *vertexShader = &vertexShaderString[0u];
    
    std::string vertexShaderStringOne = path.string() + "/Resources/Shaders/BufferDVertex.glsl";
    char *vertexShaderOne = &vertexShaderStringOne[0];
    
    std::string imageString = path.string() + "/Resources/Shaders/Image.glsl";
    char *image = &imageString[0];
    
    std::string bufferAString = path.string() + "/Resources/Shaders/BufferA.glsl";
    char *bufferA = &bufferAString[0];
    
    std::string bufferBString = path.string() + "/Resources/Shaders/BufferB.glsl";
    char *bufferB = &bufferBString[0];
    
    std::string bufferCString = path.string() + "/Resources/Shaders/BufferC.glsl";
    char *bufferC = &bufferCString[0];
    
    std::string bufferDString = path.string() + "/Resources/Shaders/BufferD.glsl";
    char *bufferD = &bufferDString[0];
    
    std::cout << bufferAString << std::endl;
    
    // We build and compile our shader program.
    Shader Image( vertexShader, image );
    Shader BufferA( vertexShader, bufferA );
    Shader BufferB( vertexShader, bufferB );
    Shader BufferC( vertexShader, bufferC );
    Shader BufferD( vertexShaderOne, bufferD );
    
    // This is for our screen quad.
    // We define the points in space that we want to render.
    float vertices[] =
    {
        
        // Positions.            // TextureCoordinates.
        -1.0f,  -1.0f, 0.0f,     //-1.0f,  1.0f,
        -1.0f,   1.0f, 0.0f,     //1.0f,  1.0f,
        1.0f,   1.0f, 0.0f,     //1.0f, -1.0f,
        1.0f,  -1.0f, 0.0f,    //-1.0f, -1.0f
        
    };
    
    // We define our Element Buffer Object indices so that if we have vertices that overlap,
    // we dont have to render twice, 50% overhead.
    unsigned int indices[] =
    {
        
        0, 1, 3,
        1, 2, 3
        
    };
    
    // We create a buffer ID so that we can later assign it to our buffers.
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );
    glGenBuffers( 1, &EBO );
    
    // Bind Vertex Array Object.
    glBindVertexArray( VAO );
    
    // Copy our vertices array in a buffer for OpenGL to use.
    // We assign our buffer ID to our new buffer and we overload it with our triangles array.
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    
    // Copy our indices in an array for OpenGL to use.
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );
    
    // Set our vertex attributes pointers.
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* ) 0 );
    glEnableVertexAttribArray( 0 );
    
    // Unbind the VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    // Unbind the VAO.
    glBindVertexArray( 0 );
    
    const int siz = 350;
    
    std::vector<glm::vec3> points;
    std::vector<int> ind;
    int counter = 0;
    
    float dis = 0.009;
    
    for ( int i = -siz; i < siz; ++i )
    {
        
        for ( int j = -siz; j < siz; ++j )
        {
            
            float x = i * dis;
            float y = j * dis;
            float z = 0;
            points.push_back( glm::vec3( x, y, z ) );
            
            ind.push_back( counter );
            
            counter++;
            
        }
        
    }
    
    // We create a buffer ID so that we can later assign it to our buffers.
    unsigned int VBOO, VAOO, EBOO;
    glGenVertexArrays( 1, &VAOO );
    glGenBuffers( 1, &VBOO );
    glGenBuffers( 1, &EBOO );
    
    // Bind Vertex Array Object.
    glBindVertexArray( VAOO );
    
    // Copy our vertices array in a buffer for OpenGL to use.
    // We assign our buffer ID to our new buffer and we overload it with our triangles array.
    glBindBuffer( GL_ARRAY_BUFFER, VBOO );
    //glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( Points ), passPoints, GL_STATIC_DRAW );
    glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( glm::vec3 ), &points.front(), GL_STATIC_DRAW );
    
    // Copy our indices in an array for OpenGL to use.
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBOO );
    //glBufferData( GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof( GLint ), passIndex, GL_STATIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof( int ), &ind.front(), GL_STATIC_DRAW );
    
    // Set our vertex attributes pointers.
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* ) 0 );
    glEnableVertexAttribArray( 0 );
    /*
     // Set our texture coordinates attributes.
     glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )( 6 * sizeof( float ) ) );
     glEnableVertexAttribArray( 2 );
     */
    
    // Unbind the VBO.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    // Unbind the VAO.
    glBindVertexArray( 0 );
    
    // Load them textures.
    //unsigned int tex = loadTexture( "Cassini-Projection-2.jpg" );
    
    BufferA.use();
    BufferA.setInt( "iChannel0", 0 );
    BufferA.setInt( "iChannel1", 1 );
    
    BufferB.use();
    BufferB.setInt( "iChannel0", 1 );
    BufferB.setInt( "iChannel1", 0 );
    
    BufferC.use();
    BufferC.setInt( "iChannel0", 0 );
    BufferC.setInt( "iChannel1", 1 );
    
    BufferD.use();
    BufferD.setInt( "iChannel0", 0 );
    //BufferD.setInt( "iChannel1", 1 );
    
    Image.use();
    Image.setInt( "iChannel0", 0 );
    Image.setInt( "iChannel1", 1 );
    Image.setInt( "iChannel2", 2 );
    Image.setInt( "iChannel3", 3 );
    
    // BufferA Ping Pong FrameBuffers
    // Framebuffer configuration.
    unsigned int frameBuffer;
    glGenFramebuffers( 1, &frameBuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
    
    glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );
    
    // Create a colour attachment texture.
    unsigned int textureColourBuffer;
    glGenTextures( 1, &textureColourBuffer );
    glBindTexture( GL_TEXTURE_2D, textureColourBuffer );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // BufferA frameBuffer configuration.
    // Framebuffer configuration.
    unsigned int frameBufferOne;
    glGenFramebuffers( 1, &frameBufferOne );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferOne );
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferOne;
    glGenTextures( 1, &textureColourBufferOne );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferOne);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferOne, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // BufferB frameBuffer configurations.
    // Framebuffer configuration.
    unsigned int frameBufferTwo;
    glGenFramebuffers( 1, &frameBufferTwo );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferTwo );
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferTwo;
    glGenTextures( 1, &textureColourBufferTwo );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferTwo );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferTwo, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Framebuffer configuration.
    unsigned int frameBufferThree;
    glGenFramebuffers( 1, &frameBufferThree );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferThree );
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferThree;
    glGenTextures( 1, &textureColourBufferThree );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferThree );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferThree, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // BufferC frameBuffer configurations.
    // Framebuffer configuration.
    unsigned int frameBufferFour;
    glGenFramebuffers(1, &frameBufferFour);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferFour);
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferFour;
    glGenTextures( 1, &textureColourBufferFour );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferFour );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferFour, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // Framebuffer configuration.
    unsigned int frameBufferFive;
    glGenFramebuffers( 1, &frameBufferFive );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferFive );
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferFive;
    glGenTextures( 1, &textureColourBufferFive );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferFive );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferFive, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // BufferD frameBuffer configurations.
    // Framebuffer configuration.
    unsigned int frameBufferSix;
    glGenFramebuffers( 1, &frameBufferSix );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferSix );
    
    // Create a colour attachment texture.
    unsigned int textureColourBufferSix;
    glGenTextures( 1, &textureColourBufferSix );
    glBindTexture( GL_TEXTURE_2D, textureColourBufferSix );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferSix, 0 );
    
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // We want to know if the frame we are rendering is even or odd.
    bool even = true;
    
    // Render Loop.
    while( !glfwWindowShouldClose( window ) )
    {
        
        
        glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );
        
        //glfwGetCursorPos( window, &xPos, &yPos );
        
        // Input.
        processInput( window );
        
        // Render.
        
        // Bind to frameBuffer and draw scene as normally would to colour texture.
        glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBuffer : frameBufferOne );
        
        glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        BufferA.use();
        float currentFrame = glfwGetTime();
        //Set the iTimeDelta uniform.
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        BufferA.setFloat( "iTimeDelta", deltaTime );
        // Set the iTime uniform.
        float timeValue = currentFrame;
        BufferA.setFloat( "iTime", timeValue );
        // Set the iResolution uniform.
        BufferA.setVec2( "iResolution", WIDTH, HEIGHT );
        // Input iMouse.
        glfwGetCursorPos( window, &xPos, &yPos );
        
        if( retina == "yes" )
        {
        
            xPos *= 2.0;
            yPos *= 2.0;
            
        }
        
        yPos = HEIGHT - yPos;
        
        xDif = xPos - xPre;
        yDif = yPos - yPre;
        
        if( xDif != 0 && pressed > 0.5 )
        {
            
            if( deltaTime == 0 ) deltaTime = 1;
            
            vX = xDif / deltaTime;
            
        }
        
        if( yDif != 0 && pressed > 0.5 )
        {
            
            if( deltaTime == 0 ) deltaTime = 1;
            
            vY = yDif / deltaTime;
            
        }
        
        // BufferA
        BufferA.setVec3( "iMouse", xPos, yPos, pressed );
        
        glBindVertexArray( VAO );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );
        
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
        // BufferB
        glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBufferTwo : frameBufferThree );
        
        glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        BufferB.use();
        BufferB.setInt( "iFrame", frames );
        //Set the iTimeDelta uniform.
        BufferB.setFloat( "iTimeDelta", deltaTime );
        // Set the iTime uniform.
        BufferB.setFloat( "iTime", timeValue );
        // Set the iResolution uniform.
        BufferB.setVec2( "iResolution", WIDTH, HEIGHT );
        // Input iMouse.
        BufferB.setVec3( "iMouse", xPos, yPos, pressed );
        // Input mouse iVel.
        BufferB.setVec2( "iVel", vX, vY );
        
        glBindVertexArray( VAO );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray( 0 );
        
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        // BufferC
        glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBufferFour : frameBufferFive );
        
        glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        BufferC.use();
        //Set the iTimeDelta uniform.
        BufferC.setFloat( "iTimeDelta", deltaTime );
        // Set the iTime uniform.
        BufferC.setFloat( "iTime", timeValue );
        // Set the iResolution uniform.
        BufferC.setVec2( "iResolution", WIDTH, HEIGHT );
        // Input iMouse.
        BufferC.setVec3( "iMouse", xPos, yPos, pressed );
        
        glBindVertexArray( VAO );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferFive : textureColourBufferFour );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );
        
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        // BufferD
        glBindFramebuffer( GL_FRAMEBUFFER, textureColourBufferSix );
        glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        BufferD.use();
        //Set the iTimeDelta uniform.
        BufferD.setFloat( "iTimeDelta", deltaTime );
        // Set the iTime uniform.
        BufferD.setFloat( "iTime", timeValue );
        // Set the iResolution uniform.
        BufferD.setVec2( "iResolution", WIDTH, HEIGHT );
        // Input iMouse.
        BufferD.setVec3( "iMouse", xPos, yPos, pressed );
        
        glBindVertexArray( VAOO );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
        glDrawElements( GL_POINTS, ind.size(), GL_UNSIGNED_INT, 0 );
        glEnable( GL_PROGRAM_POINT_SIZE );
        glBindVertexArray( 0 );
        
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        
        // Our last stage for our colour Navier-Stokes and mixing it with the Wave Equation.
        Image.use();
        // Set the iResolution uniform.
        Image.setVec2( "iResolution", WIDTH, HEIGHT );
        // Set the iTime uniform.
        Image.setFloat( "iTime", timeValue );
        glBindVertexArray( VAO );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferFive : textureColourBufferFour );
        glActiveTexture( GL_TEXTURE3 );
        glBindTexture( GL_TEXTURE_2D, textureColourBufferSix );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
        
        glBindVertexArray( 0 );
        
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        
        glfwSwapBuffers( window );
        glfwPollEvents();
        
        xPre = xPos;
        yPre = yPos;
        
        even = !even;
        
        frameCount++;
        frames++;
        finalTime = time( NULL );
        if( finalTime - initialTime > 0 )
        {
            
            FPS = frameCount / ( finalTime - initialTime );
            std::stringstream title;
            title << "Navier-Stokes Simulation, FPS : " << FPS;
            frameCount = 0;
            initialTime = finalTime;
            
            glfwSetWindowTitle( window, title.str().c_str() );
            
        }
        
    }
    
    // De-allocate all resources once they've outlived their purpose.
    glDeleteVertexArrays( 1, &VAO );
    glDeleteVertexArrays( 1, &VAOO );
    glDeleteBuffers( 1, &VBO );
    glDeleteBuffers( 1, &VBOO );
    glDeleteBuffers( 1, &EBO );
    glDeleteBuffers( 1, &EBOO );
    
    glfwTerminate();
    return 0;
}

void processInput( GLFWwindow *window )
{
    
    if ( glfwGetKey( window, GLFW_KEY_ESCAPE) == GLFW_PRESS )
    glfwSetWindowShouldClose( window, true );
    
}

void framebuffer_size_callback( GLFWwindow* window, int width, int height )
{
    
    glViewport( 0, 0, width, height );
    
}

static void cursorPositionCallback( GLFWwindow *window, double xPos, double yPos )
{
    
    xPos = 2.0 * xPos / WIDTH - 1;
    yPos = -2.0 * yPos / HEIGHT + 1;
    
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    
    if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
    {
        
        if( pressed == 0.0 )
        {
            
            pressed = 1.0;
            
        }
        
    }
    
    if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
    {
        
        if ( pressed == 1.0 )
        {
            
            pressed = 0.0;
            
        }
        
    }
    
    //std::cout << std::boolalpha << pressed << std::endl;
    
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
/*
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
        format = GL_RED;
        else if (nrComponents == 3)
        format = GL_RGB;
        else if (nrComponents == 4)
        format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}
 */
