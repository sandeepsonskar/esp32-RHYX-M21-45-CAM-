#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"

// ============================================================
// WIFI SETTINGS Change to your SSID and PASS 2.4GHz
// ============================================================

const char* ssid = "SSID";
const char* password = "PASS";

// ============================================================
// CAMERA MODEL
// ============================================================

#define CAMERA_MODEL_AI_THINKER

// ============================================================
// AI THINKER ESP32-CAM PIN CONFIGURATION
// ============================================================

#if defined(CAMERA_MODEL_AI_THINKER)

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else

#error "Camera model not selected"

#endif

// ============================================================
// MOTOR PINS
// ============================================================

#define MOTOR_1_PIN_1    14
#define MOTOR_1_PIN_2    15

#define MOTOR_2_PIN_1    13
#define MOTOR_2_PIN_2    12

// ============================================================
// PUMP AND LED
// ============================================================

#define pump  2
#define led   4

// ============================================================
// HTTP SERVER
// ============================================================

httpd_handle_t camera_httpd = NULL;

// ============================================================
// CAMERA WIDTH / HEIGHT
// ============================================================

#define CAMERA_WIDTH   240
#define CAMERA_HEIGHT  240

// ============================================================
// HTML PAGE
// ============================================================

static const char PROGMEM INDEX_HTML[] = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>IoT SPRAY Robot</title>

<style>

/* ==========================================================
   PAGE
   ========================================================== */

body {

    font-family: Arial, sans-serif;

    text-align: center;

    margin: 0;

    padding: 20px;

    background: #f2f2f2;

}

/* ==========================================================
   MAIN CONTAINER
   ========================================================== */

.container {

    display: flex;

    flex-wrap: wrap;

    justify-content: center;

}

/* ==========================================================
   COLUMNS
   ========================================================== */

.column {

    flex: 1 1 45%;

    margin: 10px;

    min-width: 300px;

}

/* ==========================================================
   CAMERA BOX
   ========================================================== */

.cameraBox {

    background: white;

    padding: 15px;

    border-radius: 10px;

    box-shadow:
        0 2px 8px rgba(0,0,0,0.15);

}

/* ==========================================================
   CAMERA CANVAS
   ========================================================== */

#cameraCanvas {

    width: 240px;

    height: 240px;

    background: black;

    border: 2px solid #222;

    display: block;

    margin: auto;

}

/* ==========================================================
   BUTTON
   ========================================================== */

.button {

    width: 40%;

    background-color: rgb(14, 87, 116);

    color: white;

    border: none;

    padding: 15px;

    text-align: center;

    display: inline-block;

    font-size: 12px;

    margin: 6px 3px;

    cursor: pointer;

    border-radius: 6px;

}

/* ==========================================================
   BUTTON HOVER
   ========================================================== */

.button:hover {

    background-color: rgb(162, 176, 244);

    color: black;

}

/* ==========================================================
   CAMERA STATUS
   ========================================================== */

.status {

    margin-top: 10px;

    font-size: 14px;

    color: #444;

}

</style>

</head>


<body>


<div class="container">


    <!-- ====================================================
         CAMERA
         ==================================================== -->

    <div class="column">

        <div class="cameraBox">

            <h1>IoT SPRAY Robot</h1>

            <h2>CAMERA</h2>

            <canvas
                id="cameraCanvas"
                width="240"
                height="240">
            </canvas>

            <div
                class="status"
                id="cameraStatus">

                Connecting to camera...

            </div>

        </div>

    </div>


    <!-- ====================================================
         ROBOT CONTROL
         ==================================================== -->

    <div class="column">

        <h1>.</h1>

        <h2>CONTROL</h2>


        <!-- FORWARD -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('forward');
            "

            ontouchstart="
                toggleCheckbox('forward');
            "

            onmouseup="
                toggleCheckbox('stop');
            "

            ontouchend="
                toggleCheckbox('stop');
            "

        >

            Forward

        </button>


        <!-- BACKWARD -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('backward');
            "

            ontouchstart="
                toggleCheckbox('backward');
            "

            onmouseup="
                toggleCheckbox('stop');
            "

            ontouchend="
                toggleCheckbox('stop');
            "

        >

            Backward

        </button>


        <p></p>


        <!-- LEFT -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('left');
            "

            ontouchstart="
                toggleCheckbox('left');
            "

            onmouseup="
                toggleCheckbox('stop');
            "

            ontouchend="
                toggleCheckbox('stop');
            "

        >

            Left

        </button>


        <!-- RIGHT -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('right');
            "

            ontouchstart="
                toggleCheckbox('right');
            "

            onmouseup="
                toggleCheckbox('stop');
            "

            ontouchend="
                toggleCheckbox('stop');
            "

        >

            Right

        </button>


        <p></p>


        <!-- STOP -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('stop');
            "

            ontouchstart="
                toggleCheckbox('stop');
            "

        >

            Stop

        </button>


        <p></p>


        <!-- PUMP -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('pumpon');
            "

            ontouchstart="
                toggleCheckbox('pumpon');
            "

        >

            PUMP ON

        </button>


        <button
            class="button"

            onmousedown="
                toggleCheckbox('pumpoff');
            "

            ontouchstart="
                toggleCheckbox('pumpoff');
            "

        >

            PUMP OFF

        </button>


        <p></p>


        <!-- LED -->

        <button
            class="button"

            onmousedown="
                toggleCheckbox('ledon');
            "

            ontouchstart="
                toggleCheckbox('ledon');
            "

        >

            LIGHT ON

        </button>


        <button
            class="button"

            onmousedown="
                toggleCheckbox('ledoff');
            "

            ontouchstart="
                toggleCheckbox('ledoff');
            "

        >

            LIGHT OFF

        </button>


    </div>


</div>


<script>


// ============================================================
// ROBOT CONTROL
// ============================================================

function toggleCheckbox(command) {

    fetch(
        "/action?go=" +
        encodeURIComponent(command),
        {
            cache: "no-store"
        }
    )

    .then(function(response) {

        if (!response.ok) {

            console.log(
                "Command failed:",
                response.status
            );

        }

    })

    .catch(function(error) {

        console.log(
            "Command error:",
            error
        );

    });

}


// ============================================================
// CAMERA CANVAS
// ============================================================

const canvas =
    document.getElementById(
        "cameraCanvas"
    );


const ctx =
    canvas.getContext(
        "2d"
    );


const statusText =
    document.getElementById(
        "cameraStatus"
    );


// ============================================================
// RGB565 FRAME CONVERSION
// ============================================================
//
// RGB565:
//
// 16 bits:
//
// RRRRR GGGGGG BBBBB
//
// Red   = 5 bits
// Green = 6 bits
// Blue  = 5 bits
//
// ============================================================

function displayRGB565(data) {

    const width = canvas.width;
    const height = canvas.height;

    const totalPixels = width * height;

    const expectedBytes = totalPixels * 2;

    if (data.length < expectedBytes) {
        throw new Error(
            "RGB565 frame too small: " +
            data.length +
            " bytes"
        );
    }

    const imageData =
        ctx.createImageData(
            width,
            height
        );

    const output =
        imageData.data;

    let sourceIndex = 0;
    let destinationIndex = 0;

    for (
        let pixelIndex = 0;
        pixelIndex < totalPixels;
        pixelIndex++
    ) {

        // ====================================================
        // IMPORTANT:
        // ESP32 RGB565 framebuffer is MSB FIRST
        // ====================================================

       const highByte = data[sourceIndex++];
        const lowByte  = data[sourceIndex++];

        const rgb565 =
            (highByte << 8) |
            lowByte;


        // ====================================================
        // RED - 5 bits
        // ====================================================

        let red =
            (rgb565 >> 11) & 0x1F;

        red =
            (red << 3) |
            (red >> 2);


        // ====================================================
        // GREEN - 6 bits
        // ====================================================

        let green =
            (rgb565 >> 5) & 0x3F;

        green =
            (green << 2) |
            (green >> 4);


        // ====================================================
        // BLUE - 5 bits
        // ====================================================

        let blue =
            rgb565 & 0x1F;

        blue =
            (blue << 3) |
            (blue >> 2);


        // ====================================================
        // RGBA
        // ====================================================

        output[destinationIndex++] = red;
        output[destinationIndex++] = green;
        output[destinationIndex++] = blue;
        output[destinationIndex++] = 255;
    }


    // ========================================================
    // DISPLAY IMAGE
    // ========================================================

    ctx.putImageData(
        imageData,
        0,
        0
    );
}


// ============================================================
// GET ONE CAMERA FRAME
// ============================================================

async function updateCamera() {

    try {


        statusText.innerHTML =
            "Capturing......";


        // ----------------------------------------------------
        // Request camera frame
        // ----------------------------------------------------

        const response =
            await fetch(
                "/frame?t=" +
                Date.now(),
                {
                    method: "GET",

                    cache: "no-store",

                    headers: {
                        "Cache-Control":
                            "no-cache"
                    }
                }
            );


        // ----------------------------------------------------
        // HTTP CHECK
        // ----------------------------------------------------

        if (!response.ok) {

            throw new Error(
                "HTTP error " +
                response.status
            );

        }


        // ----------------------------------------------------
        // Read binary data
        // ----------------------------------------------------

        const buffer =
            await response.arrayBuffer();


        const data =
            new Uint8Array(buffer);


        // ----------------------------------------------------
        // Debug information
        // ----------------------------------------------------

        console.log(
            "RGB565 frame received:",
            data.length,
            "bytes"
        );


        // ----------------------------------------------------
        // Expected:
        //
        // 240 × 240 × 2
        //
        // = 115200 bytes
        // ----------------------------------------------------

        const expectedSize =
            240 *
            240 *
            2;


        if (
            data.length <
            expectedSize
        ) {

            throw new Error(
                "Invalid frame size. " +
                "Received " +
                data.length +
                " bytes, expected " +
                expectedSize
            );

        }


        // ----------------------------------------------------
        // Convert and display
        // ----------------------------------------------------

        displayRGB565(
            data
        );


        statusText.innerHTML =
            "RGB565 Camera: OK";


    }

    catch(error) {


        console.error(
            "Camera error:",
            error
        );


        statusText.innerHTML =
            "Camera connection error";


    }

}


// ============================================================
// CAMERA LOOP
// ============================================================
//
// IMPORTANT:
//
// We deliberately use a recursive timeout instead of
// setInterval().
//
// This prevents multiple camera requests from overlapping.
//
// ============================================================

async function cameraLoop() {

      while (true) {

        await updateCamera();

        // Very small delay
        await new Promise(
            resolve => setTimeout(resolve, 30)
        );
    }
}


// ============================================================
// START CAMERA
// ============================================================

cameraLoop();


</script>


</body>

</html>

)rawliteral";


// ============================================================
// INDEX HANDLER
// ============================================================

static esp_err_t index_handler(
    httpd_req_t *req
) {

    httpd_resp_set_type(
        req,
        "text/html"
    );


    return httpd_resp_send(
        req,
        (const char *)INDEX_HTML,
        strlen(INDEX_HTML)
    );

}


// ============================================================
// RGB565 FRAME HANDLER
// ============================================================
//
// IMPORTANT:
//
// No JPEG.
// No frame2jpg().
// No MJPEG.
//
// Camera framebuffer is sent directly.
//
// ============================================================

static esp_err_t frame_handler(
    httpd_req_t *req
) {


    // --------------------------------------------------------
    // Capture frame
    // --------------------------------------------------------

    camera_fb_t *fb =
        esp_camera_fb_get();


    if (!fb) {

        Serial.println(
            "ERROR: Camera capture failed"
        );


        httpd_resp_send_500(
            req
        );


        return ESP_FAIL;

    }


    // --------------------------------------------------------
    // Print frame information
    // --------------------------------------------------------

    Serial.printf(
        "Frame: %dx%d | format=%d | length=%u\n",

        fb->width,

        fb->height,

        fb->format,

        (unsigned int)fb->len

    );


    // --------------------------------------------------------
    // Verify RGB565
    // --------------------------------------------------------

    if (
        fb->format !=
        PIXFORMAT_RGB565
    ) {

        Serial.println(
            "ERROR: Camera frame is NOT RGB565"
        );


        esp_camera_fb_return(
            fb
        );


        httpd_resp_send_500(
            req
        );


        return ESP_FAIL;

    }


    // --------------------------------------------------------
    // Calculate expected frame size
    // --------------------------------------------------------

    size_t frameSize =
        fb->width *
        fb->height *
        2;


    // --------------------------------------------------------
    // Verify buffer
    // --------------------------------------------------------

    if (
        fb->len <
        frameSize
    ) {

        Serial.printf(
            "ERROR: Invalid frame buffer. "
            "Expected %u, received %u\n",

            (unsigned int)frameSize,

            (unsigned int)fb->len
        );


        esp_camera_fb_return(
            fb
        );


        httpd_resp_send_500(
            req
        );


        return ESP_FAIL;

    }


    // --------------------------------------------------------
    // HTTP CONTENT TYPE
    // --------------------------------------------------------

    httpd_resp_set_type(
        req,
        "application/octet-stream"
    );


    // --------------------------------------------------------
    // CACHE CONTROL
    // --------------------------------------------------------

    httpd_resp_set_hdr(
        req,
        "Cache-Control",
        "no-cache, no-store, must-revalidate"
    );


    httpd_resp_set_hdr(
        req,
        "Pragma",
        "no-cache"
    );


    httpd_resp_set_hdr(
        req,
        "Access-Control-Allow-Origin",
        "*"
    );


    // --------------------------------------------------------
    // Send raw RGB565
    // --------------------------------------------------------

    esp_err_t result =
        httpd_resp_send(
            req,

            (const char *)fb->buf,

            frameSize
        );


    // --------------------------------------------------------
    // Return frame buffer
    // --------------------------------------------------------

    esp_camera_fb_return(
        fb
    );


    return result;

}


// ============================================================
// ROBOT COMMAND HANDLER
// ============================================================

static esp_err_t cmd_handler(
    httpd_req_t *req
) {


    char* buf;

    size_t buf_len;

    char variable[32] = {0};


    // --------------------------------------------------------
    // Read URL query
    // --------------------------------------------------------

    buf_len =
        httpd_req_get_url_query_len(
            req
        ) + 1;


    if (buf_len <= 1) {

        httpd_resp_send_404(
            req
        );

        return ESP_FAIL;

    }


    buf =
        (char*)malloc(
            buf_len
        );


    if (!buf) {

        httpd_resp_send_500(
            req
        );

        return ESP_FAIL;

    }


    // --------------------------------------------------------
    // Get query string
    // --------------------------------------------------------

    if (
        httpd_req_get_url_query_str(
            req,
            buf,
            buf_len
        ) != ESP_OK
    ) {

        free(buf);

        httpd_resp_send_404(
            req
        );

        return ESP_FAIL;

    }


    // --------------------------------------------------------
    // Get "go" parameter
    // --------------------------------------------------------

    if (
        httpd_query_key_value(
            buf,
            "go",
            variable,
            sizeof(variable)
        ) != ESP_OK
    ) {

        free(buf);

        httpd_resp_send_404(
            req
        );

        return ESP_FAIL;

    }


    free(buf);


    int res = 0;


    // ========================================================
    // FORWARD
    // ========================================================

    if (
        !strcmp(
            variable,
            "forward"
        )
    ) {


        Serial.println(
            "Forward"
        );


        digitalWrite(
            MOTOR_1_PIN_1,
            HIGH
        );


        digitalWrite(
            MOTOR_1_PIN_2,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_1,
            HIGH
        );


        digitalWrite(
            MOTOR_2_PIN_2,
            LOW
        );

    }


    // ========================================================
    // LEFT
    // ========================================================

    else if (
        !strcmp(
            variable,
            "left"
        )
    ) {


        Serial.println(
            "Left"
        );


        digitalWrite(
            MOTOR_1_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_1_PIN_2,
            HIGH
        );


        digitalWrite(
            MOTOR_2_PIN_1,
            HIGH
        );


        digitalWrite(
            MOTOR_2_PIN_2,
            LOW
        );

    }


    // ========================================================
    // RIGHT
    // ========================================================

    else if (
        !strcmp(
            variable,
            "right"
        )
    ) {


        Serial.println(
            "Right"
        );


        digitalWrite(
            MOTOR_1_PIN_1,
            HIGH
        );


        digitalWrite(
            MOTOR_1_PIN_2,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_2,
            HIGH
        );

    }


    // ========================================================
    // BACKWARD
    // ========================================================

    else if (
        !strcmp(
            variable,
            "backward"
        )
    ) {


        Serial.println(
            "Backward"
        );


        digitalWrite(
            MOTOR_1_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_1_PIN_2,
            HIGH
        );


        digitalWrite(
            MOTOR_2_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_2,
            HIGH
        );

    }


    // ========================================================
    // STOP
    // ========================================================

    else if (
        !strcmp(
            variable,
            "stop"
        )
    ) {


        Serial.println(
            "Stop"
        );


        digitalWrite(
            MOTOR_1_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_1_PIN_2,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_1,
            LOW
        );


        digitalWrite(
            MOTOR_2_PIN_2,
            LOW
        );

    }


    // ========================================================
    // PUMP ON
    // ========================================================

    else if (
        !strcmp(
            variable,
            "pumpon"
        )
    ) {


        Serial.println(
            "Pump ON"
        );


        digitalWrite(
            pump,
            HIGH
        );

    }


    // ========================================================
    // PUMP OFF
    // ========================================================

    else if (
        !strcmp(
            variable,
            "pumpoff"
        )
    ) {


        Serial.println(
            "Pump OFF"
        );


        digitalWrite(
            pump,
            LOW
        );

    }


    // ========================================================
    // LED ON
    // ========================================================

    else if (
        !strcmp(
            variable,
            "ledon"
        )
    ) {


        Serial.println(
            "LED ON"
        );


        digitalWrite(
            led,
            HIGH
        );

    }


    // ========================================================
    // LED OFF
    // ========================================================

    else if (
        !strcmp(
            variable,
            "ledoff"
        )
    ) {


        Serial.println(
            "LED OFF"
        );


        digitalWrite(
            led,
            LOW
        );

    }


    // ========================================================
    // UNKNOWN COMMAND
    // ========================================================

    else {

        Serial.print(
            "Unknown command: "
        );

        Serial.println(
            variable
        );


        res = -1;

    }


    // ========================================================
    // ERROR
    // ========================================================

    if (res) {

        return httpd_resp_send_500(
            req
        );

    }


    // ========================================================
    // CORS
    // ========================================================

    httpd_resp_set_hdr(
        req,
        "Access-Control-Allow-Origin",
        "*"
    );


    // ========================================================
    // RESPONSE
    // ========================================================

    return httpd_resp_send(
        req,
        NULL,
        0
    );

}


// ============================================================
// START HTTP SERVER
// ============================================================
//
// ONE SERVER ONLY.
//
// Port 80 handles:
//
// /
// /action
// /frame
//
// ============================================================

void startCameraServer() {


    httpd_config_t config =
        HTTPD_DEFAULT_CONFIG();


    config.server_port =
        80;


    // --------------------------------------------------------
    // INDEX URI
    // --------------------------------------------------------

    httpd_uri_t index_uri = {

        .uri =
            "/",

        .method =
            HTTP_GET,

        .handler =
            index_handler,

        .user_ctx =
            NULL

    };


    // --------------------------------------------------------
    // ACTION URI
    // --------------------------------------------------------

    httpd_uri_t cmd_uri = {

        .uri =
            "/action",

        .method =
            HTTP_GET,

        .handler =
            cmd_handler,

        .user_ctx =
            NULL

    };


    // --------------------------------------------------------
    // RGB565 FRAME URI
    // --------------------------------------------------------

    httpd_uri_t frame_uri = {

        .uri =
            "/frame",

        .method =
            HTTP_GET,

        .handler =
            frame_handler,

        .user_ctx =
            NULL

    };


    // --------------------------------------------------------
    // START SERVER
    // --------------------------------------------------------

    esp_err_t result =
        httpd_start(
            &camera_httpd,
            &config
        );


    if (
        result == ESP_OK
    ) {


        // Register webpage

        httpd_register_uri_handler(
            camera_httpd,
            &index_uri
        );


        // Register controls

        httpd_register_uri_handler(
            camera_httpd,
            &cmd_uri
        );


        // Register RGB565 camera

        httpd_register_uri_handler(
            camera_httpd,
            &frame_uri
        );


        Serial.println();

        Serial.println(
            "================================"
        );

        Serial.println(
            "HTTP SERVER STARTED"
        );

        Serial.println(
            "Port: 80"
        );

        Serial.println(
            "/      -> Web page"
        );

        Serial.println(
            "/action -> Robot control"
        );

        Serial.println(
            "/frame  -> RGB565 camera"
        );

        Serial.println(
            "================================"
        );


    }

    else {


        Serial.printf(
            "HTTP server failed: 0x%x\n",
            result
        );

    }

}


// ============================================================
// SETUP
// ============================================================

void setup() {


    // ========================================================
    // DISABLE BROWNOUT
    // ========================================================

    WRITE_PERI_REG(
        RTC_CNTL_BROWN_OUT_REG,
        0
    );


    // ========================================================
    // MOTOR PINS
    // ========================================================

    pinMode(
        MOTOR_1_PIN_1,
        OUTPUT
    );

    pinMode(
        MOTOR_1_PIN_2,
        OUTPUT
    );

    pinMode(
        MOTOR_2_PIN_1,
        OUTPUT
    );

    pinMode(
        MOTOR_2_PIN_2,
        OUTPUT
    );


    // ========================================================
    // PUMP
    // ========================================================

    pinMode(
        pump,
        OUTPUT
    );


    // ========================================================
    // LED
    // ========================================================

    pinMode(
        led,
        OUTPUT
    );


    // ========================================================
    // INITIAL OUTPUT STATE
    // ========================================================

    digitalWrite(
        MOTOR_1_PIN_1,
        LOW
    );

    digitalWrite(
        MOTOR_1_PIN_2,
        LOW
    );

    digitalWrite(
        MOTOR_2_PIN_1,
        LOW
    );

    digitalWrite(
        MOTOR_2_PIN_2,
        LOW
    );

    digitalWrite(
        pump,
        LOW
    );

    digitalWrite(
        led,
        LOW
    );


    // ========================================================
    // SERIAL
    // ========================================================

    Serial.begin(
        115200
    );


    Serial.setDebugOutput(
        true
    );


    delay(1000);


    Serial.println();

    Serial.println(
        "=========================================="
    );

    Serial.println(
        "ESP32-CAM RGB565 ROBOT"
    );

    Serial.println(
        "RHYX-M21-45 / GC2415"
    );

    Serial.println(
        "=========================================="
    );


    // ========================================================
    // CAMERA CONFIGURATION
    // ========================================================

    camera_config_t config;


    // --------------------------------------------------------
    // LEDC
    // --------------------------------------------------------

    config.ledc_channel =
        LEDC_CHANNEL_0;

    config.ledc_timer =
        LEDC_TIMER_0;


    // --------------------------------------------------------
    // CAMERA DATA PINS
    // --------------------------------------------------------

    config.pin_d0 =
        Y2_GPIO_NUM;

    config.pin_d1 =
        Y3_GPIO_NUM;

    config.pin_d2 =
        Y4_GPIO_NUM;

    config.pin_d3 =
        Y5_GPIO_NUM;

    config.pin_d4 =
        Y6_GPIO_NUM;

    config.pin_d5 =
        Y7_GPIO_NUM;

    config.pin_d6 =
        Y8_GPIO_NUM;

    config.pin_d7 =
        Y9_GPIO_NUM;


    // --------------------------------------------------------
    // CAMERA CLOCK
    // --------------------------------------------------------

    config.pin_xclk =
        XCLK_GPIO_NUM;

    config.pin_pclk =
        PCLK_GPIO_NUM;


    // --------------------------------------------------------
    // CAMERA SYNC
    // --------------------------------------------------------

    config.pin_vsync =
        VSYNC_GPIO_NUM;

    config.pin_href =
        HREF_GPIO_NUM;


    // --------------------------------------------------------
    // SCCB
    // --------------------------------------------------------

    config.pin_sccb_sda =
        SIOD_GPIO_NUM;

    config.pin_sccb_scl =
        SIOC_GPIO_NUM;


    // --------------------------------------------------------
    // POWER
    // --------------------------------------------------------

    config.pin_pwdn =
        PWDN_GPIO_NUM;

    config.pin_reset =
        RESET_GPIO_NUM;


    // --------------------------------------------------------
    // CLOCK FREQUENCY
    // --------------------------------------------------------

    config.xclk_freq_hz =
        20000000;


    // ========================================================
    // VERY IMPORTANT
    // ========================================================
    //
    // DIRECT RGB565
    //
    // NO JPEG
    //
    // ========================================================

    config.pixel_format =
        PIXFORMAT_RGB565;


    // ========================================================
    // IMAGE SIZE
    // ========================================================

    config.frame_size =
        FRAMESIZE_240X240;
      

    // ========================================================
    // IMAGE QUALITY
    // ========================================================
    //
    // JPEG quality is irrelevant for RGB565.
    //
    // ========================================================


    // ========================================================
    // FRAME GRAB MODE
    // ========================================================

    config.grab_mode =
        CAMERA_GRAB_WHEN_EMPTY;


    // ========================================================
    // FRAME BUFFER LOCATION
    // ========================================================

    config.fb_location =
        CAMERA_FB_IN_PSRAM;


    // ========================================================
    // FRAME BUFFER COUNT
    // ========================================================

    config.fb_count =
        2;


    // ========================================================
    // CAMERA INIT
    // ========================================================

    Serial.println(
        "Initializing camera..."
    );


    esp_err_t err =
        esp_camera_init(
            &config
        );


    if (
        err != ESP_OK
    ) {


        Serial.printf(
            "Camera init FAILED: 0x%x\n",
            err
        );


        while (true) {

            delay(1000);

        }

    }


    Serial.println(
        "Camera initialized successfully"
    );


    // ========================================================
    // GET SENSOR
    // ========================================================

    sensor_t *s =
        esp_camera_sensor_get();


    if (
        s != NULL
    ) {

        Serial.println(
            "Camera sensor detected"
        );

    }


    // ========================================================
    // WIFI
    // ========================================================

    Serial.println();

    Serial.print(
        "Connecting to WiFi"
    );


    WiFi.mode(
        WIFI_STA
    );


    WiFi.begin(
        ssid,
        password
    );


    while (
        WiFi.status() !=
        WL_CONNECTED
    ) {


        delay(500);


        Serial.print(
            "."
        );

    }


    Serial.println();

    Serial.println(
        "WiFi connected"
    );


    // ========================================================
    // IP ADDRESS
    // ========================================================

    Serial.print(
        "ESP32-CAM IP: "
    );


    Serial.println(
        WiFi.localIP()
    );


    // ========================================================
    // START SERVER
    // ========================================================

    startCameraServer();


    // ========================================================
    // FINAL MESSAGE
    // ========================================================

    Serial.println();

    Serial.println(
        "=========================================="
    );

    Serial.println(
        "CAMERA READY"
    );

    Serial.print(
        "Open browser: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.println(
        "=========================================="
    );

}


// ============================================================
// LOOP
// ============================================================

void loop() {

    // Nothing required here.
    // Camera is handled by HTTP requests.

    delay(10);

}
