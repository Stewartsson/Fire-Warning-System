/*
  Animated Fire & Distance Radar (ESP8266 VERSION)
  For John - Features: Servo sweep, IR Flame, Ultrasonic, Animated AJAX HUD.
*/

#include <Servo.h>          
#include <ESP8266WiFi.h>      
#include <ESP8266WebServer.h> 

// --- Offline Wireless Dashboard Configuration ---
const char* ssid = "ResQBot_Radar";
const char* password = "rescueadmin"; 
ESP8266WebServer server(80); 

// --- ESP8266 SAFE PIN DEFINITIONS ---
const int SERVO_PIN = D1;        
const int FLAME_SENSOR_PIN = D2; 
const int TRIG_PIN = D5;         
const int ECHO_PIN = D6;         

// --- Global Variables ---
Servo myServo;
int servoPos = 0;           
int servoDirection = 1;     
const int servoDelay = 20;  
unsigned long lastServoMoveTime = 0;

bool fireDetected = false;
int fireAngle = 0; 

int currentDistance = 0;
unsigned long lastSonarReadTime = 0;
const unsigned long sonarInterval = 100; // Read distance every 100ms

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n--- Starting ANIMATED Fire & Distance Radar ---");

  // Initialize Pins
  pinMode(FLAME_SENSOR_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize Servo 
  myServo.attach(SERVO_PIN); 

  // Initialize Local Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("1. Connect Chrome to Wi-Fi: ");
  Serial.println(ssid);
  Serial.print("2. Open Chrome and go to: http://");
  Serial.println(IP);

  // Link the web server endpoints
  server.on("/", handleRoot);      // Sends the HTML page
  server.on("/data", handleData);  // Sends the live data to the animation
  server.begin(); 
  Serial.println("Animated Tactical HUD Online!");
}

void loop() {
  server.handleClient(); 

  // 1. Read Distance
  if (millis() - lastSonarReadTime >= sonarInterval) {
    lastSonarReadTime = millis();
    readDistance();
  }

  // 2. Check for fire
  checkFire();

  // 3. Sweep servo only if safe
  if (!fireDetected) {
    sweepServo();
  } 
}

void readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  
  if (duration == 0) {
    currentDistance = 999; 
  } else {
    currentDistance = duration * 0.034 / 2;
  }
}

void sweepServo() {
  if (millis() - lastServoMoveTime >= servoDelay) {
    lastServoMoveTime = millis();
    servoPos += servoDirection;
    myServo.write(servoPos);

    if (servoPos >= 180) {
      servoDirection = -1; 
    } else if (servoPos <= 0) {
      servoDirection = 1;  
    }
  }
}

void checkFire() {
  int flameStatus = digitalRead(FLAME_SENSOR_PIN);
  
  // Active-LOW logic
  if (flameStatus == LOW) { 
    if (!fireDetected) {
      fireDetected = true;
      fireAngle = servoPos; // Lock in the exact angle
    }
  } else {
    if (fireDetected) {
      fireDetected = false;
    }
  }
}

// --- AJAX Data Stream ---
void handleData() {
  // This packages the robot's brain data into a tiny format for the animation to read
  String json = "{";
  json += "\"fire\":" + String(fireDetected ? "1" : "0") + ",";
  json += "\"angle\":" + String(fireDetected ? fireAngle : servoPos) + ",";
  json += "\"dist\":" + String(currentDistance);
  json += "}";
  server.send(200, "application/json", json);
}

// --- Animated HTML/CSS/JS Generation ---
void handleRoot() {
  // Using a raw string literal to cleanly inject all the complex HTML/CSS
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { background-color: #050505; color: #00ff00; font-family: 'Courier New', monospace; margin: 0; padding: 15px; text-align: center; }
    .hud { max-width: 500px; margin: auto; border: 2px solid #333; padding: 20px; background: #0a0a0a; box-shadow: 0 0 20px rgba(0,255,0,0.15); border-radius: 10px; }
    h1 { color: #ffc107; letter-spacing: 2px; text-shadow: 0 0 10px rgba(255,193,7,0.5); }
    
    /* --- RADAR GRAPHICS --- */
    .radar-container { 
      position: relative; width: 180px; height: 180px; margin: 30px auto; 
      border-radius: 50%; border: 3px solid #00ff00; overflow: hidden; 
      background: radial-gradient(circle, rgba(0,255,0,0.15) 0%, transparent 60%); 
      box-shadow: 0 0 25px rgba(0,255,0,0.3); transition: all 0.3s; 
    }
    .radar-container::before { content: ''; position: absolute; top: 50%; left: 0; width: 100%; height: 1px; background: rgba(0,255,0,0.3); }
    .radar-container::after { content: ''; position: absolute; top: 0; left: 50%; width: 1px; height: 100%; background: rgba(0,255,0,0.3); }
    
    /* Spinning green sweep */
    .sweep { 
      position: absolute; top: 0; left: 50%; width: 50%; height: 50%; 
      background: conic-gradient(from 0deg, transparent 70%, rgba(0,255,0,0.9) 100%); 
      transform-origin: 0% 100%; animation: scan 2s linear infinite; 
    }
    
    /* Locked red laser line */
    .target-line { 
      display: none; position: absolute; top: 50%; left: 50%; width: 50%; height: 3px; 
      background: #ff0000; transform-origin: 0% 0%; box-shadow: 0 0 10px #ff0000; 
    }
    
    @keyframes scan { 100% { transform: rotate(360deg); } }
    
    /* --- FIRE DANGER OVERLAYS --- */
    .radar-container.fire-mode { 
      border-color: #ff0000; background: radial-gradient(circle, rgba(255,0,0,0.3) 0%, transparent 60%); 
      box-shadow: 0 0 40px rgba(255,0,0,0.6); animation: strobe 0.5s infinite; 
    }
    .radar-container.fire-mode .sweep { display: none; } /* Hide spinning effect */
    .radar-container.fire-mode .target-line { display: block; } /* Show locked laser */
    .radar-container.fire-mode::before, .radar-container.fire-mode::after { background: rgba(255,0,0,0.3); }

    /* Panels */
    .panel { background: #111; border: 1px solid #333; margin: 15px 0; padding: 15px; border-radius: 5px; }
    .val { font-size: 2.5em; font-weight: bold; margin: 10px 0; text-shadow: 0 0 10px currentColor; }
    .status { font-size: 1.5em; font-weight: bold; }
    
    #status-panel { border-left: 5px solid #00ff00; }
    #status-panel.danger { border-color: #ff0000; background: #3a0000; }
    
    @keyframes strobe { 0%, 100% { opacity: 1; } 50% { opacity: 0.6; } }
  </style>
</head>
<body>
  <div class="hud">
    <h1>TACTICAL RADAR</h1>
    
    <!-- Radar Graphic -->
    <div id="radar" class="radar-container">
      <div class="sweep"></div>
      <div id="target" class="target-line"></div>
    </div>
    
    <!-- Status Data -->
    <div id="status-panel" class="panel">
      <div id="status-text" class="status" style="color:#00ff00;">RADAR SWEEPING</div>
      <div id="angle-text" style="color:#888; margin-top:5px;">ANGLE: 0&deg;</div>
    </div>
    
    <!-- Distance Data -->
    <div class="panel">
      <div style="color:#888; font-size:0.9em; letter-spacing:2px;">DISTANCE TO TARGET</div>
      <div id="dist-text" class="val" style="color:#03a9f4;">-- cm</div>
    </div>
  </div>

  <script>
    // This script asks the robot for data in the background every half second
    function updateHUD() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          const radar = document.getElementById('radar');
          const target = document.getElementById('target');
          const statusPanel = document.getElementById('status-panel');
          const statusText = document.getElementById('status-text');
          const angleText = document.getElementById('angle-text');
          const distText = document.getElementById('dist-text');
          
          // Update Distance
          if (data.dist == 999) {
            distText.innerHTML = "OUT OF RANGE";
            distText.style.color = "#ff5555";
          } else {
            distText.innerHTML = data.dist + " cm";
            distText.style.color = "#03a9f4";
          }
          
          // Update Fire Status & Animations
          if (data.fire === 1) {
            radar.classList.add('fire-mode');
            statusPanel.classList.add('danger');
            statusText.innerHTML = "!!! FIRE DETECTED !!!";
            statusText.style.color = "#ff0000";
            angleText.innerHTML = "LOCKED AT: " + data.angle + "&deg;";
            
            // Map the robot's physical angle to the graphic laser line on screen
            let visualAngle = -data.angle; 
            target.style.transform = "rotate(" + visualAngle + "deg)";
          } else {
            radar.classList.remove('fire-mode');
            statusPanel.classList.remove('danger');
            statusText.innerHTML = "RADAR SWEEPING";
            statusText.style.color = "#00ff00";
            angleText.innerHTML = "CURRENT ANGLE: " + data.angle + "&deg;";
          }
        });
    }
    
    // Run the animation loop constantly
    setInterval(updateHUD, 500);
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}
