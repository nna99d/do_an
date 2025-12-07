#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_camera.h"
#include <WiFiClientSecure.h>
#include "mbedtls/md.h"

// WiFi & MQTT 
const char* ssid = "wifi của chùa";
const char* password = "hinhnhula9so9";

const char* mqtt_server = "1ce52fa3e7184ba5bd3f13aa4b86e418.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "esp32cam";
const char* mqtt_pass = "Abc123@x";

const char* TOPIC_META = "esp32cam/image/meta";
const char* TOPIC_CHUNK = "esp32cam/image/chunk";
const char* TOPIC_CMD   = "esp32cam/cmd";

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

// ESP32-CAM AI Thinker Pinout
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

// Camera init
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 18;
    config.fb_count = 1;
  }
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    ESP.restart();
  }
}


inline uint8_t rotl8(uint8_t v, unsigned r){ r&=7; return (uint8_t)((v<<r)|(v>>(8-r))); }

char toDNA(uint8_t b2){ switch(b2&3){case 0:return 'A';case 1:return 'C';case 2:return 'G';default:return 'T';}}
uint8_t fromDNA(char base){ switch(base){case 'A':return 0;case 'C':return 1;case 'G':return 2;case 'T':return 3;default:return 0;} }

uint8_t dnaTransform(uint8_t b, uint8_t k){
  char dna[4];
  for(int i=0;i<4;i++) dna[i]=toDNA((b>>(6-2*i))&3);
  switch(k&3){
    case 0: // complement
      for(int i=0;i<4;i++){ if(dna[i]=='A') dna[i]='T'; else if(dna[i]=='T') dna[i]='A'; else if(dna[i]=='C') dna[i]='G'; else if(dna[i]=='G') dna[i]='C'; }
      break;
    case 1: // reverse
      for(int i=0;i<2;i++){ char t=dna[i]; dna[i]=dna[3-i]; dna[3-i]=t; }
      break;
    case 2: // swap pairs
      for(int i=0;i<4;i+=2){ char t=dna[i]; dna[i]=dna[i+1]; dna[i+1]=t; }
      break;
    case 3: break; // no change
  }
  uint8_t out=0;
  for(int i=0;i<4;i++) out|=(fromDNA(dna[i])<<(6-2*i));
  return out;
}

void sha256(const uint8_t* input, size_t len, uint8_t out[32]) {
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t* info=mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, input, len);
  mbedtls_md_finish(&ctx, out);
  mbedtls_md_free(&ctx);
}

// MQTT callback 
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i=0;i<length;i++) msg += (char)payload[i];
  if (msg=="capture") {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) { Serial.println("Capture fail"); return; }
    size_t imgLen = fb->len;

    // random 16B key
    uint8_t key[16];
    for (int i=0;i<16;i++) key[i]=(uint8_t)(esp_random()&0xFF);

    // SHA-256(key)
    uint8_t hash[32]; sha256(key,16,hash);

    // send META
    String meta="{\"len\":"+String(imgLen)+",\"key\":\"";
    for(int i=0;i<16;i++){ char buf[3]; sprintf(buf,"%02x",key[i]); meta+=buf; }
    meta+="\",\"hash\":\"";
    for(int i=0;i<32;i++){ char buf[3]; sprintf(buf,"%02x",hash[i]); meta+=buf; }
    meta+="\"}";
    client.publish(TOPIC_META, meta.c_str(), false);

    // send image in chunks
    const size_t CHUNK=1024;
    uint8_t buf[CHUNK];
    size_t seq=0;
    for(size_t i=0;i<imgLen;i+=CHUNK){
      size_t clen=(imgLen-i<CHUNK)?imgLen-i:CHUNK;
      for(size_t j=0;j<clen;j++){
        uint8_t p=fb->buf[i+j];
        uint8_t k=key[(i+j)%16];
        uint8_t d=dnaTransform(p,k);
        buf[j]=rotl8(d,k&7)^k;
      }
      client.publish(TOPIC_CHUNK, buf, clen, false);
      seq++;
    }
    esp_camera_fb_return(fb);
    Serial.printf("Published %u chunks (%d bytes)\n",(unsigned)seq,(int)imgLen);
  }
}

void reconnect() {
  while (!client.connected()) {
    String cid="ESP32CAM-"+String((uint32_t)esp_random(),HEX);
    if (client.connect(cid.c_str(),mqtt_user,mqtt_pass)) {
      client.subscribe(TOPIC_CMD);
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  initCamera();
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(200);
  wifiClient.setInsecure();
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
