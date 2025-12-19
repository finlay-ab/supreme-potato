// Toggle performance measurement
#define ENABLE_112_MEASURING_PERFORMANCE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>
#include <numbers>
#include <typeinfo>
#include <stdexcept>
#include <cmath>

#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

// matrix/ vector math
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"

// base shapes
#include "cone.hpp"
#include "cylinder.hpp"
#include "cube.hpp"

// load objects
#include "loadobj.hpp"
#include "simple_mesh.hpp"
#include "rocket.hpp"

// texture utils
#include "texture.hpp"

// particles
#include "particle_system.hpp"


namespace
{
    constexpr char const *kWindowTitle = "COMP3811 - CW2";
    constexpr float kMovementPerSecond_ = 5.f;  // units per second
    constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

    struct State_
    {
        ShaderProgram *prog;
        ShaderProgram *terrainProg;
		ShaderProgram* particleProg;

        enum class CameraType
        {
            Free,
            FollowRocket,
            GroundRocket
        };

        CameraType camType = CameraType::Free;

        CameraType camType2 = CameraType::GroundRocket;
        bool splitScreen = false;

        struct CamCtrl_
        {
            Vec3f position = {0.f, 0.f, 0.f};
            float phi = 0.f;
            float theta = 0.f;

            // input flags
            bool moveForward, moveBackward;
            bool moveLeft, moveRight;
            bool moveUp, moveDown;
            bool fast, slow;

            bool mouseCaptured;
            float lastX, lastY;
        } camControl;

        struct Animation_
        {
            bool active = false;
            bool paused = false;
            float currentTime = 0.0f;

            Vec3f startPosition = {0.f, 0.f, 0.f};
        } animation;

        struct Lighting_
        {
			bool light1Enabled = false;
			bool light2Enabled = false;
			bool light3Enabled = false;
            bool globalDirectionalEnabled = true;
        } lighting;
    };

    void glfw_callback_error_(int, char const *);

    void glfw_callback_key_(GLFWwindow *, int, int, int, int);
    void glfw_callback_motion_(GLFWwindow *, double, double);
    void glfw_callback_mouse_button_(GLFWwindow *, int, int, int);

    struct GLFWCleanupHelper
    {
        ~GLFWCleanupHelper();
    };
    struct GLFWWindowDeleter
    {
        ~GLFWWindowDeleter();
        GLFWwindow *window;
    };

}

int main()
try
{
    // Initialize GLFW
    if (GLFW_TRUE != glfwInit())
    {
        char const *msg = nullptr;
        int ecode = glfwGetError(&msg);
        throw Error("glfwInit() failed with '{}' ({})", msg, ecode);
    }

    // Ensure that we call glfwTerminate() at the end of the program.
    GLFWCleanupHelper cleanupHelper;

    // Configure GLFW and create window
    glfwSetErrorCallback(&glfw_callback_error_);

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DEPTH_BITS, 24);

#if !defined(NDEBUG)
    // When building in debug mode, request an OpenGL debug context. This
    // enables additional debugging features. However, this can carry extra
    // overheads. We therefore do not do this for release builds.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // ~ !NDEBUG

    GLFWwindow *window = glfwCreateWindow(
        1280,
        720,
        kWindowTitle,
        nullptr, nullptr);

    if (!window)
    {
        char const *msg = nullptr;
        int ecode = glfwGetError(&msg);
        throw Error("glfwCreateWindow() failed with '{}' ({})", msg, ecode);
    }

    GLFWWindowDeleter windowDeleter{window};

    // Set up event handling
    // TODO: Additional event handling setup
    State_ state{};

    glfwSetWindowUserPointer(window, &state);

    glfwSetKeyCallback(window, &glfw_callback_key_);
    glfwSetCursorPosCallback(window, &glfw_callback_motion_);
    glfwSetMouseButtonCallback(window, &glfw_callback_mouse_button_);

    // Set up drawing stuff
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync is on.

    // Initialize GLAD
    // This will load the OpenGL API. We mustn't make any OpenGL calls before this!
    if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
        throw Error("gladLoadGLLoader() failed - cannot load GL API!");

    std::print("RENDERER {}\n", (char const *)glGetString(GL_RENDERER));
    std::print("VENDOR {}\n", (char const *)glGetString(GL_VENDOR));
    std::print("VERSION {}\n", (char const *)glGetString(GL_VERSION));
    std::print("SHADING_LANGUAGE_VERSION {}\n", (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Ddebug output
#if !defined(NDEBUG)
    setup_gl_debug_output();
#endif // ~ !NDEBUG

    // Global GL state
    OGL_CHECKPOINT_ALWAYS();

    // TODO: global GL setup goes here
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    OGL_CHECKPOINT_ALWAYS();

    // Get actual framebuffer size.
    // This can be different from the window size, as standard window
    // decorations (title bar, borders, ...) may be included in the window size
    // but not be part of the drawable surface area.
    int iwidth, iheight;
    glfwGetFramebufferSize(window, &iwidth, &iheight);

	// found viewport by reserching 
	// https://gamedev.stackexchange.com/questions/147522/what-is-glviewport-for-and-why-it-is-not-necessary-sometimes
	// implimenteation programmed with help from 
	// https://learn.microsoft.com/en-us/windows/win32/opengl/glviewport
    glViewport(0, 0, iwidth, iheight);

	// Load shader programs
    ShaderProgram prog( {
        { GL_VERTEX_SHADER, "assets/cw2/default.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
    } );

    ShaderProgram terrainProg({
        { GL_VERTEX_SHADER, "assets/cw2/terrain.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/terrain.frag" }
        });
    ShaderProgram particleProg({
        { GL_VERTEX_SHADER, "assets/cw2/particle.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/particle.frag" }
    });


    state.prog = &prog;
    state.terrainProg = &terrainProg;
	state.particleProg = &particleProg;

    // animation
    auto last = Clock::now();
    float angle = 0.f;

    // Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	// Load terrain mesh
    auto terrainMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
    GLuint vao = create_vao(terrainMesh); 
    std::size_t vertexCount = terrainMesh.positions.size();
	// Load terrain texture
    GLuint terrainTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");
    // Load particle texture
    GLuint particleTexture = load_texture_2d("assets/cw2/particle.png");
	// Init particle system
    ParticleSystem particleSys;
    particleSys.init(&particleProg, particleTexture);



    // load landing pad mesh
    auto padMesh = load_wavefront_obj("assets/cw2/landingpad.obj");
    GLuint padVao = create_vao(padMesh);
    std::size_t padVertexCount = padMesh.positions.size();

    Vec3f landingPadPosition1 = {30.f, -0.95f, 30.f};
    Vec3f landingPadPosition2 = {0.f, -0.95f, -5.f};

    // load rocket
    auto rocketMesh = create_rocket();
    GLuint rocketVao = create_vao(rocketMesh);
    std::size_t rocketVertexCount = rocketMesh.positions.size();

    // set rocket animation start pos (at landingpad2)
    state.animation.startPosition = landingPadPosition2 + Vec3f{0.f, 1.0f, 0.f};

    // Light colours red, green and blue to make obvious
    Vec3f lightColors[3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    // Light locations relative to the rocket center
    Vec3f lightLocations[3] = {
        { -2.0f, 2.0f, 0.0f },
        { 2.0f, 2.0f, 0.0f },
        { 0.0f, 2.0f, 2.0f }
    };

    // Performance Measurement Setup
    GLuint glQueries[5] = { 0 };
    #ifdef ENABLE_112_MEASURING_PERFORMANCE
        glGenQueries(5, glQueries);
    #endif

    // Get timestamp for the start of frame
    auto frameStartTimestamp = Clock::now();

    OGL_CHECKPOINT_ALWAYS();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
		// Measure CPU frame time
        auto currentTimeStamp = Clock::now(); // Get the current timestamp
        // Frame start time stamp will not have been changed since the start of last frame so we can compare
        float cpuFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTimeStamp - frameStartTimestamp).count() / 1000.0f;
        frameStartTimestamp = currentTimeStamp;

        // Let GLFW process events
        glfwPollEvents();

        // Check if window was resized.
        float fbwidth, fbheight;
        {
            int nwidth, nheight;
            glfwGetFramebufferSize(window, &nwidth, &nheight);

            fbwidth = float(nwidth);
            fbheight = float(nheight);

            if (0 == nwidth || 0 == nheight)
            {
                // Window minimized? Pause until it is unminimized.
                // This is a bit of a hack.
                do
                {
                    glfwWaitEvents();
                    glfwGetFramebufferSize(window, &nwidth, &nheight);
                } while (0 == nwidth || 0 == nheight);
            }

            // glViewport( 0, 0, nwidth, nheight );
        }

        // Update state
        auto const now = Clock::now();
        float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
        last = now;

        // update camera
        float speed = kMovementPerSecond_;
        if (state.camControl.fast)
            speed *= 5.0f;
        if (state.camControl.slow)
            speed *= 0.1f;

        // update x,z
        float sinphi = std::sin(state.camControl.phi);
        float cosphi = std::cos(state.camControl.phi);

        Vec3f forward = {-sinphi, 0.f, -cosphi};
        Vec3f right = {cosphi, 0.f, -sinphi};

        // update pos from flags
        if (state.camControl.moveForward)
            state.camControl.position += forward * speed * dt;
        if (state.camControl.moveBackward)
            state.camControl.position -= forward * speed * dt;
        if (state.camControl.moveRight)
            state.camControl.position += right * speed * dt;
        if (state.camControl.moveLeft)
            state.camControl.position -= right * speed * dt;
        if (state.camControl.moveUp)
            state.camControl.position.y += speed * dt;
        if (state.camControl.moveDown)
            state.camControl.position.y -= speed * dt;

        // update animation
        if (state.animation.active && !state.animation.paused)
        {
            state.animation.currentTime += dt;
        }

        Mat44f rocketModel = kIdentity44f;
        Vec3f currentRocketPos = state.animation.startPosition;

        if (!state.animation.active)
        {
            rocketModel = make_translation(state.animation.startPosition);
        }
        else
        {
            // animation algo
            float t = state.animation.currentTime;

            float t2 = t * t;
            float t3 = t2 * t;

            float vertPower = 3.0f;
            float sidePower = 0.8f;

            currentRocketPos.y += 0.5f * vertPower * t2;

            currentRocketPos.x += 0.33f * sidePower * t3;
            currentRocketPos.z += 0.33f * (sidePower * 0.5f) * t3;

            float vy = vertPower * t;
            float vx = sidePower * t2;
            float vz = (sidePower * 0.5f) * t2;

            // stops it glitching at start
            if (t < 0.01f)
            {
                vy = 1.0f;
                vx = 0.f;
                vz = 0.f;
            }

            // pitch
            float horiz_mag = std::sqrt(vx * vx + vz * vz);
            float pitchAngle = std::atan2(horiz_mag, vy);

            // yaw
            float yawAngle = std::atan2(vx, vz);

            // matrix
            Mat44f rotX = make_rotation_x(pitchAngle);
            Mat44f rotY = make_rotation_y(yawAngle);

            Mat44f rotation = rotY * rotX;

            rocketModel = make_translation(currentRocketPos) * rotation;
        }

        // Particle system updates
        Vec4f offsetLocal = { 0.f, -1.0f, 0.f, 1.f };
        Vec4f offsetWorld = rocketModel * offsetLocal;
        Vec3f emitterPos = { offsetWorld.x, offsetWorld.y, offsetWorld.z };

		// Update particles only when animation is active and not paused
        if (state.animation.active && !state.animation.paused)
        {
            particleSys.update(dt, emitterPos, true);
        }
        else if (!state.animation.active)
        {
			// Kill all particles when animation not active
            for (auto& p : particleSys.particles)
            {
                p.life = -1.0f;
            }
        }

        // === Drawing ===
        // Frame time measurements
        #ifdef ENABLE_112_MEASURING_PERFORMANCE
        // Check if we have data for the final query and if so display the data gathered
        if (glIsQuery(glQueries[4]))
        {
            GLint timeAvailable = 0;
            glGetQueryObjectiv(glQueries[4], GL_QUERY_RESULT_AVAILABLE, &timeAvailable);

            if (timeAvailable)
            {
                OGL_CHECKPOINT_DEBUG();

                GLuint64 times[5];
                glGetQueryObjectui64v(glQueries[0], GL_QUERY_RESULT, &times[0]); // Start
                glGetQueryObjectui64v(glQueries[1], GL_QUERY_RESULT, &times[1]); // After terrain
                glGetQueryObjectui64v(glQueries[2], GL_QUERY_RESULT, &times[2]); // After Landing pads
                glGetQueryObjectui64v(glQueries[3], GL_QUERY_RESULT, &times[3]); // After rocket
                glGetQueryObjectui64v(glQueries[4], GL_QUERY_RESULT, &times[4]); // End

                OGL_CHECKPOINT_DEBUG();

                // Convert nanoseconds to milliseconds
                float nmConst = 1000000.0;
                double terrainTime = (times[1] - times[0]) / nmConst;
                double landingPadsTime = (times[2] - times[1]) / nmConst;
                double rocketTime = (times[3] - times[2]) / nmConst;
                double totalTime = (times[4] - times[0]) / nmConst;

                std::print("GPU [ms] Terrain: {:.3f} | Landing Pads: {:.3f} | Rocket: {:.3f} | Total: {:.3f} --- CPU Frame: {:.3f} ms\n",
                    terrainTime, landingPadsTime, rocketTime, totalTime, cpuFrameTime);
            }
        }
        #endif

        OGL_CHECKPOINT_DEBUG();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // handle split screen
        // re runs code for when split screen is enabled
		// testing ternary operator for more efficeint code
		// https://www.w3schools.com/c/c_conditions_short_hand.php
        int numberOfScreens = state.splitScreen ? 2 : 1;

        auto submitStart = Clock::now();
        for (int i = 0; i < numberOfScreens; ++i)
        {
            glUseProgram(prog.programId());
            int viewX = 0;
            int viewY = 0;
            int viewW = (int)fbwidth;
            int viewH = (int)fbheight;

            State_::CameraType currentCamType = state.camType;

            if (state.splitScreen)
            {
                viewW = (int)fbwidth / 2;
                if (i == 0)
                {
                    viewX = 0;
                    currentCamType = state.camType;
                }
                else
                {
                    viewX = (int)fbwidth / 2;
                    currentCamType = state.camType2;
                }
            }

            glViewport(viewX, viewY, viewW, viewH);

            // projection
            Mat44f projection = make_perspective_projection(
                60.0f * (std::numbers::pi_v<float> / 180.f),
                float(viewW) / float(viewH),
                0.1f, 2000.0f);

            Vec3f camPos = state.camControl.position;
            float camPhi = state.camControl.phi;
            float camTheta = state.camControl.theta;

            if (currentCamType == State_::CameraType::FollowRocket)
            {
                // distance behind rocket
                Vec4f offsetLocal = {10.f, 10.f, 5.f, 1.f};

                Vec4f offsetWorld = rocketModel * offsetLocal;
                camPos = {offsetWorld.x, offsetWorld.y, offsetWorld.z};

                // set target
                Vec3f target = currentRocketPos;
                Vec3f dir = target - camPos;

                // yaw
                camPhi = std::atan2(-dir.x, -dir.z);

                // pitch
                float distH = std::sqrt(dir.x * dir.x + dir.z * dir.z);
                camTheta = std::atan2(dir.y, distH);
            }
            else if (currentCamType == State_::CameraType::GroundRocket)
            {
                // set fixed position
                camPos = {landingPadPosition2.x + 15.f, landingPadPosition2.y + 0.5f, landingPadPosition2.z + -5.f};

                // set target
                Vec3f target = currentRocketPos;
                Vec3f dir = target - camPos;

                camPhi = std::atan2(-dir.x, -dir.z);

                float distH = std::sqrt(dir.x * dir.x + dir.z * dir.z);
                camTheta = std::atan2(dir.y, distH);
            }

            // view matrix
            Mat44f viewTranslate = make_translation({-camPos.x, -camPos.y, -camPos.z});
            Mat44f viewRotateY = make_rotation_y(-camPhi);
            Mat44f viewRotateX = make_rotation_x(-camTheta);

            Mat44f view = viewRotateX * viewRotateY * viewTranslate;

            // model
            Mat44f model = kIdentity44f;

            // MVP
            Mat44f projCameraWorld = projection * view * model;

            // normal matrix
            Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model)));

            // draw
            // OGL_CHECKPOINT_DEBUG();

            // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            // glUseProgram( prog.programId() );

            // send matrix
            glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
            glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);

            // === Lighting ===
            // Global directional light
            Vec3f lightDir = {0.f, 1.f, -1.f};
            float lightColor[] = { 1.0f, 1.0f, 1.0f };
            float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
            lightDir.x /= len;
            lightDir.y /= len;
            lightDir.z /= len;

            glUniform3fv(3, 1, &lightDir.x);
            glUniform3fv(4, 1, lightColor);
            float ambientColor[] = {0.15f, 0.15f, 0.15f};
            glUniform3fv(5, 1, ambientColor);

            #ifdef ENABLE_112_MEASURING_PERFORMANCE
            if (i == 0) glQueryCounter(glQueries[0], GL_TIMESTAMP);
            #endif

            // === Draw terrain ===
            // Bind texture
            glUseProgram(terrainProg.programId());
            glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v); // Location 0: MVP Matrix
            glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);    // Location 1: Normal Matrix
            glUniform3fv(2, 1, &lightDir.x);                      // Location 2: Light Dir
            glUniform3fv(3, 1, lightColor);                       // Location 3: Light Diffuse
            glUniform3fv(4, 1, ambientColor);                     // Location 4: Light Ambient

            // === Local lighting for terrain ===
            // Global directional light toggle
            glUniform1i(6, state.lighting.globalDirectionalEnabled);

            // Camera position
            glUniform3fv(7, 1, &camPos.x);

            // Shiny value - set to 0 as the terrain should be rough (except maybe the water but would need to sample shiny texture)
            glUniform1f(8, 0.0f);

            GLuint lightArrayLocation = 9; // Location for shader
            int valuesPerLight = 3; // Position, colour and activity

            for (int i = 0; i < 3; ++i)
            {
                // Find the location for the light at the current index so we can edit it's values
                GLuint thisLightLocation = lightArrayLocation + (i * valuesPerLight);

                Vec4f lightPositionVec4 = { lightLocations[i].x, lightLocations[i].y, lightLocations[i].z, 1.0f };
                Vec4f worldPositionVec4 = rocketModel * lightPositionVec4;
                Vec3f worldPositionVec3 = { worldPositionVec4.x, worldPositionVec4.y, worldPositionVec4.z };

                bool active = false;
                if (i == 0) active = state.lighting.light1Enabled;
                if (i == 1) active = state.lighting.light2Enabled;
                if (i == 2) active = state.lighting.light3Enabled;

                // Send data to correct location by offsetting by 1 each time
                glUniform3fv(thisLightLocation + 0, 1, &worldPositionVec3.x);
                glUniform3fv(thisLightLocation + 1, 1, &lightColors[i].x);
                glUniform1i(thisLightLocation + 2, active);
            }

            // Bind Texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, terrainTexture);
            glUniform1i(5, 0); // Location 5: Texture Unit

            glBindVertexArray(vao);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);

            #ifdef ENABLE_112_MEASURING_PERFORMANCE
            if (i == 0) glQueryCounter(glQueries[1], GL_TIMESTAMP); // After Terrain
            #endif

            // draw landing pads
            glUseProgram(prog.programId());

            // === Local lighting for terrain ===
            // Global directional light toggle
            glUniform1i(6, state.lighting.globalDirectionalEnabled);

            // Camera position
            glUniform3fv(7, 1, &camPos.x);

            for (int i = 0; i < 3; ++i)
            {
                // Find the location for the light at the current index so we can edit it's values
                GLuint thisLightLocation = lightArrayLocation + (i * valuesPerLight);

                Vec4f lightPositionVec4 = { lightLocations[i].x, lightLocations[i].y, lightLocations[i].z, 1.0f };
                Vec4f worldPositionVec4 = rocketModel * lightPositionVec4;
                Vec3f worldPositionVec3 = { worldPositionVec4.x, worldPositionVec4.y, worldPositionVec4.z };

                bool active = false;
                if (i == 0) active = state.lighting.light1Enabled;
                if (i == 1) active = state.lighting.light2Enabled;
                if (i == 2) active = state.lighting.light3Enabled;

                // Send data to correct location by offsetting by 1 each time
                glUniform3fv(thisLightLocation + 0, 1, &worldPositionVec3.x);
                glUniform3fv(thisLightLocation + 1, 1, &lightColors[i].x);
                glUniform1i(thisLightLocation + 2, active);
            }

            glUniform1f(8, -1.0f); // Use MTL shine
            glBindVertexArray(padVao);

            model = make_translation(landingPadPosition1);

            // calculate matrices
            Mat44f mvpPad = projection * view * model;
            Mat33f normalMatPad = mat44_to_mat33(transpose(invert(model)));

            // send matrices to shader
            glUniformMatrix4fv(0, 1, GL_TRUE, mvpPad.v);       // uProjCameraWorld
            glUniformMatrix3fv(1, 1, GL_TRUE, normalMatPad.v); // uNormalMatrix
            glUniformMatrix4fv(2, 1, GL_TRUE, model.v); // uModelMatrix

            glDrawArrays(GL_TRIANGLES, 0, padVertexCount);

            // draw second pad
            model = make_translation(landingPadPosition2);

            // recalculate matrices for new position
            mvpPad = projection * view * model;
            normalMatPad = mat44_to_mat33(transpose(invert(model)));

            // send new matrices
            glUniformMatrix4fv(0, 1, GL_TRUE, mvpPad.v);
            glUniformMatrix3fv(1, 1, GL_TRUE, normalMatPad.v);
            glUniformMatrix4fv(2, 1, GL_TRUE, model.v);

            glDrawArrays(GL_TRIANGLES, 0, padVertexCount);
            #ifdef ENABLE_112_MEASURING_PERFORMANCE
            if (i == 0) glQueryCounter(glQueries[2], GL_TIMESTAMP);
            #endif

            // draw rocket
            // Shiny value - set to 100 for shiny rocket metal
            glUniform1f(8, 100.f);
            glBindVertexArray(rocketVao);
            // model = make_translation(landingPadPosition2 + Vec3f{0.f, 1.0f, 0.f});
            //  calculate matrices
            Mat44f mvpRocket = projection * view * rocketModel;
            Mat33f normalMatRocket = mat44_to_mat33(transpose(invert(rocketModel)));
            // send matrices to shader
            glUniformMatrix4fv(0, 1, GL_TRUE, mvpRocket.v);       // uProjCameraWorld
            glUniformMatrix3fv(1, 1, GL_TRUE, normalMatRocket.v); // uNormalMatrix
            glUniformMatrix4fv(2, 1, GL_TRUE, rocketModel.v); // uModelMatrix
            glDrawArrays(GL_TRIANGLES, 0, rocketVertexCount);
            #ifdef ENABLE_112_MEASURING_PERFORMANCE
            if (i == 0) glQueryCounter(glQueries[3], GL_TIMESTAMP);
            #endif

            // Render particles
            particleSys.render(projection* view);
            #ifdef ENABLE_112_MEASURING_PERFORMANCE
            if (i == 0) glQueryCounter(glQueries[4], GL_TIMESTAMP);
            #endif
        }
        auto submitEnd = Clock::now();
        
        // clean
        glBindVertexArray(0);
        glUseProgram(0);

        OGL_CHECKPOINT_DEBUG();

        // Display results
        glfwSwapBuffers(window);
    }

    // Cleanup.
    state.prog = nullptr;
    state.terrainProg = nullptr;

    // TODO: additional cleanup

    return 0;
}

catch (std::exception const &eErr)
{
    std::print(stderr, "Top-level Exception ({}):\n", typeid(eErr).name());
    std::print(stderr, "{}\n", eErr.what());
    std::print(stderr, "Bye.\n");
    return 1;
}

namespace
{
    void glfw_callback_error_(int aErrNum, char const *aErrDesc)
    {
        std::print(stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum);
    }

    void glfw_callback_key_(GLFWwindow *aWindow, int aKey, int, int aAction, int mods)
    {
        if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
        {
            glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
            return;
        }

        if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
        {
            // R-key reloads shaders.
            if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
            {
                // reset animation state
                state->animation.active = false;
                state->animation.paused = false;
                state->animation.currentTime = 0.f;
                if (state->prog)
                {
                    try
                    {
                        state->prog->reload();
                        std::print(stderr, "Shaders reloaded and recompiled.\n");
                    }
                    catch (std::exception const &eErr)
                    {
                        std::print(stderr, "Error when reloading shader:\n");
                        std::print(stderr, "{}\n", eErr.what());
                        std::print(stderr, "Keeping old shader.\n");
                    }
                }
            }

            // toggle animation
            if (GLFW_KEY_F == aKey && aAction == GLFW_PRESS)
            {
                if (!state->animation.active)
                {
                    state->animation.active = true;
                    state->animation.paused = false;
                    state->animation.currentTime = 0.f;
                }
                else
                {
                    state->animation.paused = !state->animation.paused;
                }
            }

            // Toggle split screen
            if (GLFW_KEY_V == aKey && aAction == GLFW_PRESS)
            {
                state->splitScreen = !state->splitScreen;
            }

            // toggle camera type
            if (GLFW_KEY_C == aKey && GLFW_PRESS == aAction)
            {
                using Camera = State_::CameraType;

                // Determine which camera to switch (Shift = 2nd camera, No Shift = Main camera)
                State_::CameraType &targetCam = (mods & GLFW_MOD_SHIFT) ? state->camType2 : state->camType;

                if (targetCam == Camera::Free)
                {
                    targetCam = Camera::FollowRocket;
                }
                else if (targetCam == Camera::FollowRocket)
                {
                    targetCam = Camera::GroundRocket;
                }
                else
                {
                    targetCam = Camera::Free;
                }
            }

            // Toggle Lights
            if (aKey == GLFW_KEY_1 && aAction == GLFW_PRESS) state->lighting.light1Enabled = !state->lighting.light1Enabled;
            if (aKey == GLFW_KEY_2 && aAction == GLFW_PRESS) state->lighting.light2Enabled = !state->lighting.light2Enabled;
            if (aKey == GLFW_KEY_3 && aAction == GLFW_PRESS) state->lighting.light3Enabled = !state->lighting.light3Enabled;
            if (aKey == GLFW_KEY_4 && aAction == GLFW_PRESS) state->lighting.globalDirectionalEnabled = !state->lighting.globalDirectionalEnabled ;

            bool isPressed = (aAction != GLFW_RELEASE);
            if (aKey == GLFW_KEY_W)
                state->camControl.moveForward = isPressed;
            if (aKey == GLFW_KEY_S)
                state->camControl.moveBackward = isPressed;
            if (aKey == GLFW_KEY_A)
                state->camControl.moveLeft = isPressed;
            if (aKey == GLFW_KEY_D)
                state->camControl.moveRight = isPressed;
            if (aKey == GLFW_KEY_Q)
                state->camControl.moveDown = isPressed;
            if (aKey == GLFW_KEY_E)
                state->camControl.moveUp = isPressed;
            if (aKey == GLFW_KEY_LEFT_SHIFT)
                state->camControl.fast = isPressed;
            if (aKey == GLFW_KEY_LEFT_CONTROL)
                state->camControl.slow = isPressed;
        }
    }

    void glfw_callback_mouse_button_(GLFWwindow *aWindow, int button, int action, int mods)
    {
        if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            {
                state->camControl.mouseCaptured = !state->camControl.mouseCaptured;

                if (state->camControl.mouseCaptured)
                {
                    glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    double x, y;
                    glfwGetCursorPos(aWindow, &x, &y);
                    state->camControl.lastX = float(x);
                    state->camControl.lastY = float(y);
                }
                else
                {
                    glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        }
    }

    void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
    {
        if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
        {
            if (state->camControl.mouseCaptured)
            {
                auto const dx = float(aX - state->camControl.lastX);
                auto const dy = float(aY - state->camControl.lastY);

                state->camControl.phi -= dx * kMouseSensitivity_;
                state->camControl.theta -= dy * kMouseSensitivity_;

                constexpr float kMaxtheta = std::numbers::pi_v<float> / 2.f - 0.01f;
                if (state->camControl.theta > kMaxtheta)
                    state->camControl.theta = kMaxtheta;
                if (state->camControl.theta < -kMaxtheta)
                    state->camControl.theta = -kMaxtheta;
            }

            state->camControl.lastX = float(aX);
            state->camControl.lastY = float(aY);
        }
    }
}

namespace
{
    GLFWCleanupHelper::~GLFWCleanupHelper()
    {
        glfwTerminate();
    }

    GLFWWindowDeleter::~GLFWWindowDeleter()
    {
        if (window)
            glfwDestroyWindow(window);
    }
}
