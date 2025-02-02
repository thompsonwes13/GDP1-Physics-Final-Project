//#define GLAD_GL_IMPLEMENTATION
//#include <glad/glad.h>
//
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "GLCommon.h"

//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>     // "input output" stream
#include <fstream>      // "file" stream
#include <sstream>      // "string" stream ("string builder" in Java c#, etc.)
#include <string>
#include <vector>

//void ReadPlyModelFromFile(std::string plyFileName);
#include "PlyFileLoaders.h"
#include "Basic_Shader_Manager/cShaderManager.h"
#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "sharedThings.h"       // Fly camera
#include "cPhysics.h"
#include "cLightManager.h"
#include <windows.h>    // Includes ALL of windows... MessageBox
#include "cLightHelper/cLightHelper.h"
//
#include "cBasicTextureManager/cBasicTextureManager.h"

#include "cLowPassFilter.h"

//
//const unsigned int MAX_NUMBER_OF_MESHES = 1000;
//unsigned int g_NumberOfMeshesToDraw;
//sMesh* g_myMeshes[MAX_NUMBER_OF_MESHES] = { 0 };    // All zeros

std::vector<sMesh*> g_vecMeshesToDraw;

cPhysics* g_pPhysicEngine = NULL;
// This loads the 3D models for drawing, etc.
cVAOManager* g_pMeshManager = NULL;

cBasicTextureManager* g_pTextures = NULL;

//cLightManager* g_pLightManager = NULL;

void AddModelsToScene(cVAOManager* pMeshManager, GLuint shaderProgram);

void DrawMesh(sMesh* pCurMesh, GLuint program);

void DrawExplosion(glm::vec3 location);
void DrawDamageBox(glm::vec3 location, std::string side);

//glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool isControlDown(GLFWwindow* window);
//{
//    if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
//        (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS))
//    {
//        return true;
//    }
//    return false;
//}

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

glm::vec3 getRandom_vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3(
        getRandomFloat(min.x, max.x),
        getRandomFloat(min.y, max.y),
        getRandomFloat(min.z, max.z));
}

std::string getStringVec3(glm::vec3 theVec3)
{
    std::stringstream ssVec;
    ssVec << "(" << theVec3.x << ", " << theVec3.y << ", " << theVec3.z << ")";
    return ssVec.str();
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS)
    {
        // check if you are out of bounds
        if (::g_selectedLightIndex > 0)
        {
            ::g_selectedLightIndex--;
        }
        else
        {
            ::g_selectedLightIndex = cLightManager::NUMBEROFLIGHTS - 1;
        }
    }
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS)
    {
        ::g_selectedLightIndex++;
        if (::g_selectedLightIndex >= cLightManager::NUMBEROFLIGHTS)
        {
            ::g_selectedLightIndex = 0;
        }
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (::g_pLightManager->theLights[g_selectedLightIndex].param2.x == 1.0)
        {
            ::g_pLightManager->theLights[g_selectedLightIndex].param2.x = 0.0;
        }
        else if (::g_pLightManager->theLights[g_selectedLightIndex].param2.x == 0.0)
        {
            ::g_pLightManager->theLights[g_selectedLightIndex].param2.x = 1.0;
        }
    }

    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
    {
        //back in meshes
        if (::g_selectedMeshIndex > 0)
        {
            ::g_selectedMeshIndex--;
        }
        else
        {
            g_selectedMeshIndex = g_vecMeshesToDraw.size() - 1;
        }
    }
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    {
        //forward in meshes
        ::g_selectedMeshIndex++;
        if (::g_selectedMeshIndex >= g_vecMeshesToDraw.size())
        {
            ::g_selectedMeshIndex = 0;
        }
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        if (g_bShowDebugSpheres)
        {
            g_bShowDebugSpheres = false;
        }
        else if (!g_bShowDebugSpheres)
        {
            g_bShowDebugSpheres = true;
        }
    }
}

// Returns NULL if NOT found
sMesh* pFindMeshByFriendlyName(std::string theNameToFind)
{
    for (unsigned int index = 0; index != ::g_vecMeshesToDraw.size(); index++)
    {
        if (::g_vecMeshesToDraw[index]->uniqueFriendlyName == theNameToFind)
        {
            return ::g_vecMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}

void AABBOctTree(void);

int main(void)
{
    
    AABBOctTree();


    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Callback for keyboard, but for "typing"
    // Like it captures the press and release and repeat
    glfwSetKeyCallback(window, key_callback);

    // 
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowFocusCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);



    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    cShaderManager* pShaderManager = new cShaderManager();

    cShaderManager::cShader vertexShader;
    vertexShader.fileName = "assets/shaders/vertex01.glsl";

    cShaderManager::cShader fragmentShader;
    fragmentShader.fileName = "assets/shaders/fragment01.glsl";

    if ( ! pShaderManager->createProgramFromFile("shader01",
                                                 vertexShader, fragmentShader))
    {
        std::cout << "Error: " << pShaderManager->getLastError() << std::endl;
    }
    else
    {
        std::cout << "Shader built OK" << std::endl;
    }

    const GLuint program = pShaderManager->getIDFromFriendlyName("shader01");

    glUseProgram(program);

//    cVAOManager* pMeshManager = new cVAOManager();
    ::g_pMeshManager = new cVAOManager();

    ::g_pPhysicEngine = new cPhysics();
    // For triangle meshes, let the physics object "know" about the VAO manager
    ::g_pPhysicEngine->setVAOManager(::g_pMeshManager);

    // This also adds physics objects to the phsyics system
    AddModelsToScene(::g_pMeshManager, program);
    
    ::g_pFlyCamera = new cBasicFlyCamera();
    //::g_pFlyCamera->setEyeLocation(glm::vec3(0.0f, 5.0f, -50.0f));
    // To see the Galactica:
    ::g_pFlyCamera->setEyeLocation(glm::vec3(10'000.0f, 100'000.0f, 100'000.0f));
    // Rotate the camera 180 degrees
    ::g_pFlyCamera->rotateLeftRight_Yaw_NoScaling(glm::radians(180.0f));


    glUseProgram(program);

    // Enable depth buffering (z buffering)
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
    glEnable(GL_DEPTH_TEST);

    cLowPassFilter frameTimeFilter;
//    frameTimeFilter.setNumSamples(30000);

    double currentFrameTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();

    // Set up the lights
    ::g_pLightManager = new cLightManager();
    // Called only once
    ::g_pLightManager->loadUniformLocations(program);

    // Set up one of the lights in the scene
    ::g_pLightManager->theLights[0].position = glm::vec4(2'000.0f, 110'000.0f, 14'000.0f, 1.0f);
    ::g_pLightManager->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[0].atten.y = 0.000001f;
    ::g_pLightManager->theLights[0].atten.z = 0.000000001f;

    ::g_pLightManager->theLights[0].param1.x = 0.0f;    // Point light (see shader)
    ::g_pLightManager->theLights[0].param2.x = 1.0f;    // Turn on (see shader)

    // Left spot light
    ::g_pLightManager->theLights[1].position = glm::vec4(2'000.0f, 110'000.0f, 12'000.0f, 1.0f);
    ::g_pLightManager->theLights[1].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[1].atten.y = 0.00001f;
    ::g_pLightManager->theLights[1].atten.z = 0.0000001f;

    ::g_pLightManager->theLights[1].direction = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

    ::g_pLightManager->theLights[1].param1.x = 1.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[1].param2.x = 1.0f;    // Turn on (see shader)

    // Right spot light
    ::g_pLightManager->theLights[2].position = glm::vec4(2'000.0f, 110'000.0f, 12'000.0f, 1.0f);
    ::g_pLightManager->theLights[2].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[2].atten.y = 0.00001f;
    ::g_pLightManager->theLights[2].atten.z = 0.0000001f;

    ::g_pLightManager->theLights[2].direction = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

    ::g_pLightManager->theLights[2].param1.x = 1.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[2].param2.x = 1.0f;    // Turn on (see shader)

    // Banshee point light
    ::g_pLightManager->theLights[3].position = glm::vec4(2'000.0f, 110'000.0f, 12'000.0f, 1.0f);
    ::g_pLightManager->theLights[3].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[3].atten.y = 0.001f;
    ::g_pLightManager->theLights[3].atten.z = 0.000001f;

    ::g_pLightManager->theLights[3].direction = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

    ::g_pLightManager->theLights[3].param1.x = 0.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[3].param2.x = 1.0f;    // Turn on (see shader)

    // Directional
    ::g_pLightManager->theLights[5].position = glm::vec4(0.0f, 20.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[5].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[5].atten.y = 0.01f;
    ::g_pLightManager->theLights[5].atten.z = 0.001f;

    ::g_pLightManager->theLights[5].param1.x = 2.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[5].direction = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[5].param1.y = 5.0f;   //  y = inner angle
    ::g_pLightManager->theLights[5].param1.z = 10.0f;  //  z = outer angle

    ::g_pLightManager->theLights[1].param2.x = 0.0f;    // Turn on (see shader)

    ::g_pTextures = new cBasicTextureManager();

    ::g_pTextures->SetBasePath("assets/textures");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dua-lipa-promo.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Puzzle_parts.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Non-uniform concrete wall 0512-3-1024x1024.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("UV_Test_750x750.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("shape-element-splattered-texture-stroke_1194-8223.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Grey_Brick_Wall_Texture.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dirty-metal-texture_1048-4784.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080_24bit_black_and_white.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("banshee_texture.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("smiley_texture.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("explosion_texture.bmp");
    //
    ::g_pTextures->Create2DTextureFromBMPFile("SurprisedChildFace.bmp");

    // Load the space skybox
    /*std::string errorString;
    ::g_pTextures->SetBasePath("assets/textures/CubeMaps");
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("Space",
        "SpaceBox_right1_posX.bmp", 
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp", 
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp", 
        "SpaceBox_back6_negZ.bmp", true, errorString))
    {
        std::cout << "Loaded space skybox" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space skybox because: " << errorString << std::endl;
    }*/

    std::string errorString;
    ::g_pTextures->SetBasePath("assets/textures/CubeMaps");
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("Space",
        "DarkStormyLeft2048.bmp",
        "DarkStormyRight2048.bmp",
        "DarkStormyUp2048.bmp",
        "DarkStormyDown2048.bmp",
        "DarkStormyFront2048.bmp",
        "DarkStormyBack2048.bmp", true, errorString))
    {
        std::cout << "Loaded space skybox" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space skybox because: " << errorString << std::endl;
    }

    // Load the sunny day cube map
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("SunnyDay",
        "TropicalSunnyDayLeft2048.bmp",
        "TropicalSunnyDayRight2048.bmp",
        "TropicalSunnyDayUp2048.bmp",
        "TropicalSunnyDayDown2048.bmp",
        "TropicalSunnyDayFront2048.bmp",
        "TropicalSunnyDayBack2048.bmp",
        true, errorString))
    {
        std::cout << "Loaded space SunnyDay" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space SunnyDay because: " << errorString << std::endl;
    }

    //glGet with argument GL_ACTIVE_TEXTURE, or GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
    // 
    // void glGetIntegerv(GLenum pname, GLint* data);
    // 
    //GLint iActiveTextureUnits = 0;
    //glGetIntegerv(GL_ACTIVE_TEXTURE, &iActiveTextureUnits);
    //std::cout << "GL_ACTIVE_TEXTURE = " << (iActiveTextureUnits - GL_TEXTURE0) << std::endl;

    GLint iMaxCombinedTextureInmageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxCombinedTextureInmageUnits);
    std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << iMaxCombinedTextureInmageUnits << std::endl;

    // data returns one value, the maximum number of components of the inputs read by the fragment shader, 
    // which must be at least 128.
    GLint iMaxFragmentInputComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &iMaxFragmentInputComponents);
    std::cout << "GL_MAX_FRAGMENT_INPUT_COMPONENTS = " << iMaxFragmentInputComponents << std::endl;
    

    // data returns one value, the maximum number of individual floating - point, integer, or boolean values 
    // that can be held in uniform variable storage for a fragment shader.The value must be at least 1024. 
    GLint iMaxFragmentUniformComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &iMaxFragmentUniformComponents);
    std::cout << "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = " << iMaxFragmentUniformComponents << std::endl;
        

    //  Turn on the blend operation
    glEnable(GL_BLEND);
    // Do alpha channel transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cLightHelper TheLightHelper;

    // Is the default (cull back facing polygons)
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // HACK:
    unsigned int numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = 0;

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //        glm::mat4 m, p, v, mvp;
        glm::mat4 matProjection = glm::mat4(1.0f);

        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            1.0f,           // Near plane (as far from the camera as possible)
            10'000.0f);       // Far plane (as near to the camera as possible)
// For a "far" view of the large Galactica
//            1'000.1f,           // Near plane (as far from the camera as possible)
//            1'000'000.0f);       // Far plane (as near to the camera as possible)

        // View or "camera"
        glm::mat4 matView = glm::mat4(1.0f);

        //        glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        matView = glm::lookAt(::g_pFlyCamera->getEyeLocation(),
            ::g_pFlyCamera->getTargetLocation(),
            upVector);
        //        matView = glm::lookAt( cameraEye,
        //                               cameraTarget,
        //                               upVector);


        const GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        const GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);


        // Calculate elapsed time
        // We'll enhance this
        currentFrameTime = glfwGetTime();
        double tempDeltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Set a limit on the maximum frame time
        const double MAX_FRAME_TIME = 1.0 / 60.0;   // 60Hz (16 ms)
        if (tempDeltaTime > MAX_FRAME_TIME)
        {
            tempDeltaTime = MAX_FRAME_TIME;
        }

        // Add this sample to the low pass filer ("averager")
        frameTimeFilter.addSample(tempDeltaTime);
        // 
        double deltaTime = frameTimeFilter.getAverage();


        // **************************************************************
// Sky box
// Move the sky sphere with the camera
        sMesh* pSkySphere = pFindMeshByFriendlyName("SkySphere");
        pSkySphere->positionXYZ = ::g_pFlyCamera->getEyeLocation();

        // Disable backface culling (so BOTH sides are drawn)
        glDisable(GL_CULL_FACE);
        // Don't perform depth buffer testing
        glDisable(GL_DEPTH_TEST);

        pSkySphere->bIsVisible = true;

        pSkySphere->uniformScale = 100.0f;

        // Tell the shader this is the skybox, so use the cube map
        // uniform samplerCube skyBoxTexture;
        // uniform bool bIsSkyBoxObject;
        GLuint bIsSkyBoxObject_UL = glGetUniformLocation(program, "bIsSkyBoxObject");
        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_TRUE);
        
        // Set the cube map texture, just like we do with the 2D
        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("Space");
//        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("SunnyDay");
        // Make sure this is an unused texture unit
        glActiveTexture(GL_TEXTURE0 + 40);
        // *****************************************
        // NOTE: This is a CUBE_MAP, not a 2D
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeSamplerID);
//        glBindTexture(GL_TEXTURE_2D, cubeSamplerID);
        // *****************************************
        GLint skyBoxTextureSampler_UL = glGetUniformLocation(program, "skyBoxTextureSampler");
        glUniform1i(skyBoxTextureSampler_UL, 40);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here


        DrawMesh(pSkySphere, program);

        pSkySphere->bIsVisible = false;

        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_FALSE);

        glEnable(GL_CULL_FACE);
        // Enable depth test and write to depth buffer (normal rendering)
        glEnable(GL_DEPTH_TEST);
        //        glDepthMask(GL_FALSE);
        //        glDepthFunc(GL_LESS);
                // **************************************************************




        ::g_pLightManager->updateShaderWithLightInfo();

        // *******************************************************************
        //    ____                       _                      
        //   |  _ \ _ __ __ ___      __ | |    ___   ___  _ __  
        //   | | | | '__/ _` \ \ /\ / / | |   / _ \ / _ \| '_ \ 
        //   | |_| | | | (_| |\ V  V /  | |__| (_) | (_) | |_) |
        //   |____/|_|  \__,_| \_/\_/   |_____\___/ \___/| .__/ 
        //                                               |_|            
        // // Will do two passes, one with "close" projection (clipping)
        // and one with "far away"

        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            1.0f,           // Near plane (as far from the camera as possible)
            1000.0f);       // Far plane (as near to the camera as possible)
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);



        // *******************************************************************



        //// OH NO! 
        //for (sMesh* pCurMesh : g_vecMeshesToDraw)
        //{
        //    pCurMesh->positionXYZ.z += 1000.0f;
        //}
        //glm::vec3 theEye = ::g_pFlyCamera->getEyeLocation();
        //theEye.z += 1000.0f;
        //g_pFlyCamera->setEyeLocation(theEye);


        // **********************************************************************************
        if (::g_bShowDebugSpheres)
        {

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            const float DEBUG_LIGHT_BRIGHTNESS = 0.3f;

            const float ACCURACY = 0.1f;       // How many units distance
            float distance_75_percent =
                TheLightHelper.calcApproxDistFromAtten(0.75f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, 0.0f, 0.0f, 1.0f),
                distance_75_percent,
                program);


            float distance_50_percent =
                TheLightHelper.calcApproxDistFromAtten(0.5f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_50_percent,
                program);

            float distance_25_percent =
                TheLightHelper.calcApproxDistFromAtten(0.25f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, 0.0f, DEBUG_LIGHT_BRIGHTNESS, 1.0f),
                distance_25_percent,
                program);

            float distance_05_percent =
                TheLightHelper.calcApproxDistFromAtten(0.05f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_05_percent,
                program);

        }
        // **********************************************************************************

        // Draw all the objects
        for (unsigned int meshIndex = 0; meshIndex != ::g_vecMeshesToDraw.size(); meshIndex++)
        {
            //            sMesh* pCurMesh = ::g_myMeshes[meshIndex];
            sMesh* pCurMesh = ::g_vecMeshesToDraw[meshIndex];
            //            pCurMesh->bDoNotLight = true;
            DrawMesh(pCurMesh, program);

            if (pCurMesh->uniqueFriendlyName == "Banshee")
            {
                ::g_pFlyCamera->setEyeLocation(glm::vec3(pCurMesh->positionXYZ.x, pCurMesh->positionXYZ.y + 2000, pCurMesh->positionXYZ.z + 12000));
                ::g_pLightManager->theLights[1].position = glm::vec4(pCurMesh->positionXYZ.x - 100, pCurMesh->positionXYZ.y + 200, pCurMesh->positionXYZ.z - 2000, 1.0);
                ::g_pLightManager->theLights[2].position = glm::vec4(pCurMesh->positionXYZ.x + 100, pCurMesh->positionXYZ.y + 200, pCurMesh->positionXYZ.z - 2000, 1.0);
                ::g_pLightManager->theLights[3].position = glm::vec4(pCurMesh->positionXYZ.x, pCurMesh->positionXYZ.y + 1000, pCurMesh->positionXYZ.z + 1000, 1.0);
            }

            if (pCurMesh->uniqueFriendlyName == "Yellow_Sphere")
            {
                pCurMesh->displayCount++;

                if (pCurMesh->displayCount >= 100)
                {
                    g_vecMeshesToDraw.erase(g_vecMeshesToDraw.begin() + meshIndex);
                    break;
                }
            }

            cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("Banshee");
            glm::vec3 Offset;

            if (pCurMesh->uniqueFriendlyName == "Explosion")
            {
                pCurMesh->displayCount++;

                if (pCurMesh->side == "Forward")
                {
                    Offset = glm::vec3(0.0, 200.0, -1000.0);
                }
                else if (pCurMesh->side == "Back")
                {
                    Offset = glm::vec3(0.0, 100.0, 2000.0);
                }
                else if (pCurMesh->side == "Up")
                {
                    Offset = glm::vec3(0.0, 600.0, 0.0);
                }
                else if (pCurMesh->side == "Down")
                {
                    Offset = glm::vec3(0.0, -600.0, 0.0);
                }
                else if (pCurMesh->side == "Left")
                {
                    Offset = glm::vec3(-1250.0, -200.0, 0.0);
                }
                else if (pCurMesh->side == "Right")
                {
                    Offset = glm::vec3(1250.0, -200.0, 0.0);
                }

                pCurMesh->positionXYZ = pViperPhys->position + Offset;

                if (pCurMesh->displayCount >= 100)
                {
                    g_vecMeshesToDraw.erase(g_vecMeshesToDraw.begin() + meshIndex);
                    break;
                }
            }

        }//for (unsigned int meshIndex..

        //// For a "far" view of the large Galactica
        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            1'000.0f,           // Near plane (as far from the camera as possible)
            1'000'000.0f);       // Far plane (as near to the camera as possible)
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        // Draw everything again, but this time far away things
        for (unsigned int meshIndex = 0; meshIndex != ::g_vecMeshesToDraw.size(); meshIndex++)
        {
            //            sMesh* pCurMesh = ::g_myMeshes[meshIndex];
            sMesh* pCurMesh = ::g_vecMeshesToDraw[meshIndex];
            //            pCurMesh->bDoNotLight = true;
            DrawMesh(pCurMesh, program);

        }//for (unsigned int meshIndex..

        std::vector <cPhysics::cBroad_Cube*> pTheAABB_Cubes;
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        pTheAABB_Cubes.push_back(NULL);
        //std::vector<cPhysics::sTriangle> All_AABB_Tris;
        // Which AABB bounding box of the broad phase is the viper now? 
        {
            cPhysics::sPhysInfo* pBansheePhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("Banshee");
            if (pBansheePhys)
            {
                // The size of the AABBs that we sliced up the Galactical model in the broad phase
                const float AABBSIZE = 10000.0f;

                // Using the same XYZ location in space we used for the triangle vertices,
                //  we are going to pass the location of the viper to get an ID
                //  of an AABB/Cube that WOULD BE at that location (if there was one...)

                unsigned long long hypotheticalAABB_IDs[7];

                hypotheticalAABB_IDs[0] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x,
                    pBansheePhys->position.y,
                    pBansheePhys->position.z, AABBSIZE);

                hypotheticalAABB_IDs[1] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x + 2000,
                    pBansheePhys->position.y,
                    pBansheePhys->position.z, AABBSIZE);

                hypotheticalAABB_IDs[2] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x - 2000,
                    pBansheePhys->position.y,
                    pBansheePhys->position.z, AABBSIZE);

                hypotheticalAABB_IDs[3] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x,
                    pBansheePhys->position.y + 2000,
                    pBansheePhys->position.z, AABBSIZE);

                hypotheticalAABB_IDs[4] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x,
                    pBansheePhys->position.y - 2000,
                    pBansheePhys->position.z, AABBSIZE);

                hypotheticalAABB_IDs[5] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x,
                    pBansheePhys->position.y,
                    pBansheePhys->position.z + 2000, AABBSIZE);

                hypotheticalAABB_IDs[6] =
                    g_pPhysicEngine->calcBP_GridIndex(
                    pBansheePhys->position.x,
                    pBansheePhys->position.y,
                    pBansheePhys->position.z - 2000, AABBSIZE);

                // Where would that hypothetical AABB be in space
                glm::vec3 minXYZofHypotheticalCube = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(hypotheticalAABB_IDs[0], AABBSIZE);

                // Draw a cube at that location
                sMesh* pDebugAABB = pFindMeshByFriendlyName("AABB_MinXYZ_At_Origin");
                pDebugAABB->positionXYZ = minXYZofHypotheticalCube;
                pDebugAABB->bIsVisible = true;
                pDebugAABB->uniformScale = 10'000.0f;

                for (int index = 0; index < 7; index++)
                {
                    // Is this an AABB that's already part of the broad phase? 
                    // i.e. is it already in the map?
                    std::map< unsigned long long, cPhysics::cBroad_Cube* >::iterator
                        it_pCube = ::g_pPhysicEngine->map_BP_CubeGrid.find(hypotheticalAABB_IDs[index]);
                    //
                    if (it_pCube == ::g_pPhysicEngine->map_BP_CubeGrid.end())
                    {
                        // NO, there is no cube there
                        pDebugAABB->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                        numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = 0;
                    }
                    // NOT equal to the end
                    if (it_pCube != ::g_pPhysicEngine->map_BP_CubeGrid.end())
                    {
                        // YES, there is an AABB (full of triangles) there!
                        pDebugAABB->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
                        // 
                        // 
                        pTheAABB_Cubes[index] = it_pCube->second;
                        /*for (int i = 0; i < pTheAABB_Cube->vec_pTriangles.size(); i++)
                        {
                            All_AABB_Tris.push_back(pTheAABB_Cube->vec_pTriangles[i]);
                        }*/
                        //std::cout << pTheAABB_Cubes[index]->vec_pTriangles.size() << std::endl;
                        // Pass THIS smaller list of triangles to the narrow phase
                        numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = pTheAABB_Cubes[index]->vec_pTriangles.size();
                    }
                }

                //std::cout << All_AABB_Tris.size() << std::endl;

                DrawMesh(pDebugAABB, program);
                pDebugAABB->bIsVisible = false;

            }
        }

        // For Debug, draw a cube where the smaller Cube/AABB/Regions on the broad phase 
        //  structrue is, in world space
        // 
        // std::map< unsigned long long /*index*/, cBroad_Cube* > map_BP_CubeGrid;
        //
        sMesh* pDebugAABB = pFindMeshByFriendlyName("AABB_MinXYZ_At_Origin");
        if (pDebugAABB)
        {
            pDebugAABB->bIsVisible = true;
            pDebugAABB->uniformScale = 10'000.0f;

            for (std::map< unsigned long long, cPhysics::cBroad_Cube* >::iterator
                it_pCube = ::g_pPhysicEngine->map_BP_CubeGrid.begin();
                it_pCube != ::g_pPhysicEngine->map_BP_CubeGrid.end();
                it_pCube++)
            {

                // Draw a cube at that location
                //pDebugAABB->positionXYZ = it_pCube->second->getMinXYZ();
                DrawMesh(pDebugAABB, program);

            }

            pDebugAABB->bIsVisible = false;
        }//if (pDebugAABB)
               
        // Draw the LASER beam
        cPhysics::sLine LASERbeam;
        glm::vec3 LASERbeam_Offset = glm::vec3(0.0f, -2.0f, 0.0f);
        std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;

        // ************************************* FRONT SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(0.0, 0.0, -1.0);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 12;
                LASERbeam.startXYZ.z -= 12500;
                LASERbeam.startXYZ.y -= 2000;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.0f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Forward", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box in front (-z)
            if (pTheAABB_Cubes[6])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Forward", vec_RayTriangle_Collisions, pTheAABB_Cubes[6]->vec_pTriangles, false);
            }
        }
        // ************************************* LEFT SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(-1.0, 0.0, -1.0);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 10;
                LASERbeam.startXYZ.x -= 1250.0;
                LASERbeam.startXYZ.z -= 12000;
                LASERbeam.startXYZ.y -= 2250;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.0f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Left", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box to the left (-x)
            if (pTheAABB_Cubes[2])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Left", vec_RayTriangle_Collisions, pTheAABB_Cubes[2]->vec_pTriangles, false);
            }
        }
        // ************************************* RIGHT SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(1.0, 0.0, -1.0);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 10;
                LASERbeam.startXYZ.x += 1250.0;
                LASERbeam.startXYZ.z -= 12000;
                LASERbeam.startXYZ.y -= 2250;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.0f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            //::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);
            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Right", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box to the right (+x)
            if (pTheAABB_Cubes[1])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Right", vec_RayTriangle_Collisions, pTheAABB_Cubes[1]->vec_pTriangles, false);
            }
        }
        // ************************************* UP SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(0.0, 1.0, 0.1);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 11;
                //LASERbeam.startXYZ.y -= 0.085;
                LASERbeam.startXYZ.z -= 12000;
                LASERbeam.startXYZ.y -= 1750;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.1f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Up", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box above (+y)
            if (pTheAABB_Cubes[3])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Up", vec_RayTriangle_Collisions, pTheAABB_Cubes[3]->vec_pTriangles, false);
            }
        }
        // ************************************* DOWN SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(0.0, -1.0, 0.1);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 12;
                //LASERbeam.startXYZ.y += 0.1;
                LASERbeam.startXYZ.z -= 12000;
                LASERbeam.startXYZ.y -= 2250;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.1f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Down", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box below (-y)
            if (pTheAABB_Cubes[4])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Down", vec_RayTriangle_Collisions, pTheAABB_Cubes[4]->vec_pTriangles, false);
            }
        }
        // ************************************* BACK SIDE ************************************* //
        {
            if (::g_bShowLASERBeam)
            {
                // The fly camera is always "looking at" something 1.0 unit away
                glm::vec3 cameraDirection = glm::vec3(0.0, 0.0, 1.0);

                LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
                //LASERbeam.startXYZ.z -= 10;
                LASERbeam.startXYZ.z -= 11000;
                LASERbeam.startXYZ.y -= 2000;

                // Move the LASER below the camera
                LASERbeam.startXYZ += LASERbeam_Offset;
                glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

                glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                    glm::vec3(0.0f, 1.0f, 0.0f)));

                // Is the LASER less than 100 units long?
                // (is the last LAZER ball we drew beyond 500 units form the camera?)
                while (glm::distance(LASERbeam.startXYZ, LASER_ball_location) < 500.0f)
                {
                    // Move the next ball 0.1 times the normalized camera direction
                    LASER_ball_location += (cameraDirection * 50.0f);
                    DrawDebugSphere(LASER_ball_location, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 50.0f, program);
                }

                // Set the end of the LASER to the last location of the beam
                LASERbeam.endXYZ = LASER_ball_location;

            }//if (::g_bShowLASERBeam)

            // Draw the end of this LASER beam
            DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            // Now draw a different coloured ball wherever we get a collision with a triangle
            if (pTheAABB_Cubes[0])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Back", vec_RayTriangle_Collisions, pTheAABB_Cubes[0]->vec_pTriangles, false);
            }
            //Check Box behind (+z)
            if (pTheAABB_Cubes[5])
            {
                ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, "Back", vec_RayTriangle_Collisions, pTheAABB_Cubes[5]->vec_pTriangles, false);
            }
        }

        glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        float triangleSize = 500.0f;

        for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
            itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
        {
            for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
                itTri != itTriList->vecTriangles.end(); itTri++)
            {
                // Draw a sphere at the centre of the triangle
                DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, program);
                
                cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("Banshee");
                
                if (itTriList->rayDirection == "Forward" && pViperPhys->velocity.z < 0)
                {
                    pViperPhys->velocity.z = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Forward");
                }
                if (itTriList->rayDirection == "Back" && pViperPhys->velocity.z > 0)
                {
                    pViperPhys->velocity.z = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Back");
                }
                if (itTriList->rayDirection == "Up" && pViperPhys->velocity.y > 0)
                {
                    pViperPhys->velocity.y = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Up");
                }
                if (itTriList->rayDirection == "Down" && pViperPhys->velocity.y < 0)
                {
                    pViperPhys->velocity.y = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Down");
                }
                if (itTriList->rayDirection == "Left" && pViperPhys->velocity.x < 0)
                {
                    pViperPhys->velocity.x = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Left");
                }
                if (itTriList->rayDirection == "Right" && pViperPhys->velocity.x > 0)
                {
                    pViperPhys->velocity.x = 0;
                    DrawExplosion(itTri->intersectionPoint);
                    DrawDamageBox(pViperPhys->position, "Right");
                }


            }//for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles

        }//for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions

        vec_RayTriangle_Collisions.clear();

        // Physic update and test 
        ::g_pPhysicEngine->StepTick(deltaTime);

        //g_pPhysicEngine->vecGeneralPhysicsObjects[1]->velocity = glm::vec3 (0.0f);

        // Handle async IO stuff
        handleKeyboardAsync(window);
        handleMouseAsync(window);

        glfwSwapBuffers(window);
        glfwPollEvents();


        std::stringstream ssTitle;
        ssTitle << "Camera: "
            << ::g_pFlyCamera->getEyeLocation().x << ", "
            << ::g_pFlyCamera->getEyeLocation().y << ", "
            << ::g_pFlyCamera->getEyeLocation().z
            << "   ";
        ssTitle << "Light[" << g_selectedLightIndex << "]: "
            << " "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.x << ", "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.y << ", "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.z
            << "   "
            << "linear: " << ::g_pLightManager->theLights[g_selectedLightIndex].atten.y
            << "   "
            << "quad: " << ::g_pLightManager->theLights[g_selectedLightIndex].atten.z
            << "   ";
        ssTitle << "mesh[" << g_selectedMeshIndex << "] "
            << ::g_vecMeshesToDraw[g_selectedMeshIndex]->uniqueFriendlyName << "  "
            << ::g_vecMeshesToDraw[g_selectedMeshIndex]->positionXYZ.x << ", "
            << ::g_vecMeshesToDraw[g_selectedMeshIndex]->positionXYZ.y << ", "
            << ::g_vecMeshesToDraw[g_selectedMeshIndex]->positionXYZ.z
            << "  ";

        // Add the viper info, too
        cPhysics::sPhysInfo* pBansheePhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("Banshee");
        if (pBansheePhys)
        {
            ssTitle
        //        << " Viper XYZ:" << getStringVec3(pViperPhys->position)
                << " vel:" << getStringVec3(pBansheePhys->velocity)
                << " acc:" << getStringVec3(pBansheePhys->acceleration);

        }//if (pViperPhys)

        // Show frame time
        ssTitle << " deltaTime = " << deltaTime
            << " FPS: " << 1.0 / deltaTime;

        ssTitle << " BP tris: " << numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing;

 //       std::cout << " deltaTime = " << deltaTime << " FPS: " << 1.0 / deltaTime << std::endl;


//        glfwSetWindowTitle(window, "Hey!");
        glfwSetWindowTitle(window, ssTitle.str().c_str());


    }// End of the draw loop


    // Delete everything
    delete ::g_pFlyCamera;
    delete ::g_pPhysicEngine;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


void AddModelsToScene(cVAOManager* pMeshManager, GLuint program)
{
    {
        sModelDrawInfo galacticaModel;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
            galacticaModel, program);
        std::cout /* << galacticaModel.meshName << ": " */<< galacticaModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo cubeMinXYZ_at_OriginInfo;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Cube_MinXYZ_at_Origin_xyz_n_uv.ply",
            cubeMinXYZ_at_OriginInfo, program);
        std::cout /* << cubeMinXYZ_at_OriginInfo.meshName << ": " */<< cubeMinXYZ_at_OriginInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo warehouseModel;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Warehouse_xyz_n_uv.ply",
            warehouseModel, program);
        std::cout << warehouseModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo tankModel;
        pMeshManager->LoadModelIntoVAO("assets/models/Low_Poly_Tank_Model_3D_model_xyz_n_uv.ply",
            tankModel, program);
        std::cout /* << tankModel.meshName << " : " */<< tankModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo terrainModel;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Simple_MeshLab_terrain_x5_xyz_N_uv.ply",
            terrainModel, program);
        std::cout << terrainModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo bunnyModel;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/bun_zipper_res2_10x_size_xyz_N_uv.ply",
            bunnyModel, program);
        std::cout << bunnyModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo platPlaneDrawInfo;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Flat_Plane_xyz_N_uv.ply",
            platPlaneDrawInfo, program);
        std::cout << platPlaneDrawInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo sphereMesh;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz_N_uv.ply",

            sphereMesh, program);
        std::cout << sphereMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo sphereShadowMesh;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_Flat_Shadow_xyz_N_uv.ply",
            sphereShadowMesh, program);
        std::cout << sphereShadowMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo newViperModelInfo;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Viper_MkVII_xyz_n_uv.ply",
            newViperModelInfo, program);
        std::cout << newViperModelInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo castleMesh;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/bowsers_castle_xyz_N_uv.ply",
            castleMesh, program);
        std::cout << castleMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo bansheeMesh;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/space_banshee_xyz_N_uv.ply",
            bansheeMesh, program);
        std::cout << bansheeMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sMesh* pNewViper = new sMesh();
        pNewViper->modelFileName = "assets/models/Viper_MkVII_xyz_n_uv.ply";
        pNewViper->positionXYZ = glm::vec3(2600.0f, 59850.0f, 20850.0f);
        pNewViper->rotationEulerXYZ.y = 180;
        pNewViper->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pNewViper->bOverrideObjectColour = true;
        pNewViper->uniqueFriendlyName = "New_Viper_Player";
        pNewViper->bIsVisible = false;
        pNewViper->uniformScale = 500.0f;
        pNewViper->textures[0] = "dirty-metal-texture_1048-4784.bmp";
        pNewViper->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pNewViper);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pViperPhysObject = new  cPhysics::sPhysInfo();
        pViperPhysObject->bDoesntMove = false;
        pViperPhysObject->position = pNewViper->positionXYZ;
        pViperPhysObject->velocity = glm::vec3(0.0f);
        pViperPhysObject->pAssociatedDrawingMeshInstance = pNewViper;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pViperPhysObject);
    }

    {
        sMesh* pBanshee = new sMesh();
        pBanshee->modelFileName = "assets/models/space_banshee_xyz_N_uv.ply";
        pBanshee->positionXYZ = glm::vec3(2600.0f, 59850.0f, 20850.0f);
        pBanshee->rotationEulerXYZ.y = 90;
        pBanshee->objectColourRGBA = glm::vec4(0.2f, 0.0f, 0.5f, 1.0f);
        pBanshee->bOverrideObjectColour = true;
        pBanshee->uniqueFriendlyName = "Banshee";
        pBanshee->bIsVisible = true;
        pBanshee->uniformScale = 10.0f;
        pBanshee->textures[0] = "banshee_texture.bmp";
        pBanshee->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pBanshee);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pBansheePhysObject = new  cPhysics::sPhysInfo();
        pBansheePhysObject->bDoesntMove = false;
        pBansheePhysObject->position = pBanshee->positionXYZ;
        pBansheePhysObject->velocity = glm::vec3(0.0f);
        pBansheePhysObject->velocity.z = -1000;
        pBansheePhysObject->pAssociatedDrawingMeshInstance = pBanshee;

        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pBansheePhysObject);
    }

    {
        sMesh* pGalactica = new sMesh();
        pGalactica->modelFileName = "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply";
        pGalactica->positionXYZ = glm::vec3(-10000.0f, 0.0f, -5000.0f);
        pGalactica->rotationEulerXYZ.y = 17.0f;
        pGalactica->rotationEulerXYZ.x = 23.0f;
        pGalactica->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        //pGalactica->bIsWireframe = true;
        pGalactica->bOverrideObjectColour = true;
        pGalactica->uniqueFriendlyName = "Galactica";
        //pGalactica->bDoNotLight = true;
//        pGalactica->bIsVisible = true;
        pGalactica->bIsVisible = false;
        pGalactica->uniformScale = 1.0f;
        //
        pGalactica->textures[0] = "Non-uniform concrete wall 0512-3-1024x1024.bmp";
        pGalactica->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pGalactica);

        // This is just for testing to see if the xyz locations correctly map to a gridID and the other way around
        unsigned long long gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(0.0f, 0.0f, 0.0f, 1000.0f); // 0, 0, 0
        glm::vec3 minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(500.0f, 500.0f, 500.0f, 1000.0f);              // 0, 0, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-500.0f, -500.0f, -500.0f, 1000.0f);           // 
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(10.0f, 2500.0f, 10.0f, 1000.0f);               // 0, 2, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(2500.0f, 10.0f, 10.0f, 1000.0f);               // 2, 0, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(10.0f, 10.0f, 2500.0f, 1000.0f);               // 0, 0, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(8745.0f, 3723.0f, 2500.0f, 1000.0f);           // 8, 3, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-8745.0f, -3723.0f, -2500.0f, 1000.0f);           // 8, 3, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-999.0f, -999.0f, -999.0f, 1000.0f);           // -1, -1, -1
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);



        // 1000x1000x1000 aabbs
        //::g_pPhysicEngine->initBroadPhaseGrid();

        //::g_pPhysicEngine->generateBroadPhaseGrid(
        //    "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
        //    1000.0f,                            // AABB Cube region size
        //    pGalactica->positionXYZ,
        //    pGalactica->rotationEulerXYZ,
        //    pGalactica->uniformScale);


        sMesh* pGalacticaWireframe = new sMesh();
        pGalacticaWireframe->modelFileName = "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply";
        pGalacticaWireframe->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
        pGalacticaWireframe->positionXYZ = pGalactica->positionXYZ;
        pGalacticaWireframe->rotationEulerXYZ = pGalactica->rotationEulerXYZ;
        pGalacticaWireframe->uniformScale = pGalactica->uniformScale;
        pGalacticaWireframe->bIsWireframe = false;
        pGalacticaWireframe->bOverrideObjectColour = true;
        pGalacticaWireframe->bDoNotLight = true;
        pGalacticaWireframe->bIsVisible = true;

        ::g_vecMeshesToDraw.push_back(pGalacticaWireframe);


        // Debug AABB shape
        sMesh* pAABBCube_MinAtOrigin = new sMesh();
        pAABBCube_MinAtOrigin->modelFileName = "assets/models/Cube_MinXYZ_at_Origin_xyz_n_uv.ply";
        pAABBCube_MinAtOrigin->bIsWireframe = true;
        pAABBCube_MinAtOrigin->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        pAABBCube_MinAtOrigin->bOverrideObjectColour = true;
        pAABBCube_MinAtOrigin->bDoNotLight = true;
        pAABBCube_MinAtOrigin->bIsVisible = false;
        pAABBCube_MinAtOrigin->uniqueFriendlyName = "AABB_MinXYZ_At_Origin";

        ::g_vecMeshesToDraw.push_back(pAABBCube_MinAtOrigin);
    }

    {
        sMesh* pCastle = new sMesh();
        pCastle->modelFileName = "assets/models/bowsers_castle_xyz_N_uv.ply";
        pCastle->positionXYZ = glm::vec3(3000.0f, 0.0f, 0.0f);
        pCastle->rotationEulerXYZ.y = 0.0f;
        pCastle->rotationEulerXYZ.x = 0.0f;
        pCastle->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        //pGalactica->bIsWireframe = true;
        pCastle->bOverrideObjectColour = true;
        pCastle->uniqueFriendlyName = "Castle";
        //pGalactica->bDoNotLight = true;
//        pGalactica->bIsVisible = true;
        pCastle->bIsVisible = true;
        pCastle->uniformScale = 1000.0f;
        //
        pCastle->textures[0] = "Non-uniform concrete wall 0512-3-1024x1024.bmp";
        pCastle->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pCastle);

        ::g_pPhysicEngine->addTriangleMesh(
            "assets/models/bowsers_castle_xyz_N_uv.ply",
            pCastle->positionXYZ,
            pCastle->rotationEulerXYZ,
            pCastle->uniformScale);

        ::g_pPhysicEngine->generateBroadPhaseGrid(
        "assets/models/bowsers_castle_xyz_N_uv.ply",
        10000.0f,                            // AABB Cube region size
        pCastle->positionXYZ,
        pCastle->rotationEulerXYZ,
        pCastle->uniformScale);

        sMesh* pCastleWireframe = new sMesh();
        pCastleWireframe->modelFileName = "assets/models/bowsers_castle_xyz_N_uv.ply";
        pCastleWireframe->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
        pCastleWireframe->positionXYZ = pCastle->positionXYZ;
        pCastleWireframe->rotationEulerXYZ = pCastle->rotationEulerXYZ;
        pCastleWireframe->uniformScale = pCastle->uniformScale;
        pCastleWireframe->bIsWireframe = true;
        pCastleWireframe->bOverrideObjectColour = true;
        pCastleWireframe->bDoNotLight = true;
        pCastleWireframe->bIsVisible = false;

        ::g_vecMeshesToDraw.push_back(pCastleWireframe);
    }

//    {
//        sMesh* pWarehouse = new sMesh();
//        pWarehouse->modelFileName = "assets/models/Warehouse_xyz_n_uv.ply";
//        pWarehouse->positionXYZ = glm::vec3(0.0f, 50000.0f, 0.0f);
//        pWarehouse->rotationEulerXYZ.y = 0.0f;
//        pWarehouse->rotationEulerXYZ.x = 0.0f;
//        pWarehouse->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
//        //pGalactica->bIsWireframe = true;
//        pWarehouse->bOverrideObjectColour = true;
//        pWarehouse->uniqueFriendlyName = "Warehouse";
//        //pGalactica->bDoNotLight = true;
////        pGalactica->bIsVisible = true;
//        pWarehouse->bIsVisible = true;
//        pWarehouse->uniformScale = 100.0f;
//        //
//        pWarehouse->textures[0] = "Non-uniform concrete wall 0512-3-1024x1024.bmp";
//        pWarehouse->blendRatio[0] = 1.0f;
//
//        ::g_vecMeshesToDraw.push_back(pWarehouse);
//
//        ::g_pPhysicEngine->addTriangleMesh(
//            "assets/models/Warehouse_xyz_n_uv.ply",
//            pWarehouse->positionXYZ,
//            pWarehouse->rotationEulerXYZ,
//            pWarehouse->uniformScale);
//    }

    {
        sMesh* pSkySphere = new sMesh();
        pSkySphere->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
        pSkySphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pSkySphere->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        //       pSkySphere->bIsWireframe = true;
        pSkySphere->bOverrideObjectColour = true;
        pSkySphere->uniformScale = 25.0f;
        pSkySphere->uniqueFriendlyName = "SkySphere";
        pSkySphere->textures[0] = "bad_bunny_1920x1080.bmp";
        pSkySphere->blendRatio[0] = 1.0f;
        pSkySphere->bIsVisible = false;
        ::g_vecMeshesToDraw.push_back(pSkySphere);
    }
}
// x = 5, y = 15    --> 0, 1
// x = 40.0, y = 80.0   --> [4][8]
// (40.0, 80.0) --> box size = 100
//   [0][0]   
void calcBoxXYFromCoord(float x, float y, int &xIndex, int &yIndex, float boxSize)
{
    xIndex = (int)(x / boxSize);
    yIndex = (int)(y / boxSize);
    return;
}


void AABBOctTree(void)
{
    struct sSquare
    {
        //       vector< cTriangles* > vecTriangleInThisSquare
        glm::vec2 minXY;
        glm::vec2 maxXY;
        float width;
        unsigned int indexColRow;
    };

    sSquare grid[10][10];
    float sqaureWidth = 10;

    for (unsigned int x = 0; x < 10; x++)
    {
        for (unsigned int y = 0; y < 10; y++)
        {
            grid[x][y].width = sqaureWidth;
            grid[x][y].minXY.x = sqaureWidth * x;
            grid[x][y].minXY.y = sqaureWidth * y;

            grid[x][y].maxXY.x = sqaureWidth * x + sqaureWidth;
            grid[x][y].maxXY.y = sqaureWidth * y + sqaureWidth;
        }
    }

    int xIndex, yIndex;
    calcBoxXYFromCoord(5.0f, 15.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;

    calcBoxXYFromCoord(40.0f, 80.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;

    return;
}

void DrawExplosion(glm::vec3 location)
{
    sMesh* pYellowSphere = new sMesh();
    pYellowSphere->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
    pYellowSphere->positionXYZ = location;
    pYellowSphere->uniformScale = 200;
    pYellowSphere->bIsWireframe = false;
    pYellowSphere->rotationEulerXYZ.y = 0.0f;
    pYellowSphere->objectColourRGBA = glm::vec4((255 / 255.0f), (255 / 255.0f), (100 / 255.0f), 1.0f);
    pYellowSphere->uniqueFriendlyName = "Yellow_Sphere";
    pYellowSphere->textures[0] = "smiley_texture.bmp";

    ::g_vecMeshesToDraw.push_back(pYellowSphere);
}

void DrawDamageBox(glm::vec3 location, std::string side)
{
    glm::vec3 Offset;
    std::string meshSide;

    if (side == "Forward")
    {
        Offset = glm::vec3(0.0, 200.0, -1000.0);
        meshSide = "Forward";
    }
    else if (side == "Back")
    {
        Offset = glm::vec3(0.0, 100.0, 2000.0);
        meshSide = "Back";
    }
    else if (side == "Up")
    {
        Offset = glm::vec3(0.0, 600.0, 0.0);
        meshSide = "Up";
    }
    else if (side == "Down")
    {
        Offset = glm::vec3(0.0, -600.0, 0.0);
        meshSide = "Down";
    }
    else if (side == "Left")
    {
        Offset = glm::vec3(1250.0, -200.0, 0.0);
        meshSide = "Left";
    }
    else if (side == "Right")
    {
        Offset = glm::vec3(1250.0, -200.0, 0.0);
        meshSide = "Right";
    }

    sMesh* pDamageBox = new sMesh();
    pDamageBox->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
    pDamageBox->positionXYZ = location + Offset;
    pDamageBox->uniformScale = 250;
    pDamageBox->bIsWireframe = false;
    pDamageBox->rotationEulerXYZ.y = 0.0f;
    pDamageBox->objectColourRGBA = glm::vec4((255 / 255.0f), (255 / 255.0f), (100 / 255.0f), 1.0f);
    pDamageBox->uniqueFriendlyName = "Explosion";
    pDamageBox->textures[0] = "explosion_texture.bmp";
    pDamageBox->side = meshSide;

    ::g_vecMeshesToDraw.push_back(pDamageBox);
}