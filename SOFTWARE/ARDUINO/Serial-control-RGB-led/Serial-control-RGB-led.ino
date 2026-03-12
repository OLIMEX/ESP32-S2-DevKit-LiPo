//Olimex ESP32-S2-DevKit-Lipo RGB Serial Control Demo
//
//Available commands:
//help          - show this help
//off           - turn LED off
//red           - red LED
//green         - green LED
//blue          - blue LED
//rainbow       - start rainbow animation
//rgb R G B     - set custom color (0-255)

#include <Arduino.h>
#include "driver/rmt_tx.h"
#include "driver/gpio.h"

#define LED_PIN 18
#define RMT_RESOLUTION_HZ 10000000

rmt_channel_handle_t tx_chan = NULL;
rmt_encoder_handle_t encoder = NULL;

uint32_t lastAnim = 0;
uint8_t rainbow = 0;
bool rainbowMode = false;

void ws2812_init()
{
  rmt_tx_channel_config_t tx_config = {
      .gpio_num = (gpio_num_t)LED_PIN,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = RMT_RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .trans_queue_depth = 4
  };

  rmt_new_tx_channel(&tx_config, &tx_chan);

  rmt_bytes_encoder_config_t bytes_config;

  bytes_config.bit0.duration0 = 4;
  bytes_config.bit0.level0 = 1;
  bytes_config.bit0.duration1 = 8;
  bytes_config.bit0.level1 = 0;

  bytes_config.bit1.duration0 = 8;
  bytes_config.bit1.level0 = 1;
  bytes_config.bit1.duration1 = 4;
  bytes_config.bit1.level1 = 0;

  bytes_config.flags.msb_first = 1;

  rmt_new_bytes_encoder(&bytes_config, &encoder);

  rmt_enable(tx_chan);
}

void ws2812_write(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t data[3] = {g,r,b};

  rmt_transmit_config_t tx_config = {0};

  rmt_transmit(tx_chan, encoder, data, sizeof(data), &tx_config);
  rmt_tx_wait_all_done(tx_chan, portMAX_DELAY);
}

void rainbowStep()
{
  uint8_t r,g,b;
  uint8_t pos = rainbow;

  if(pos < 85) {
    r = pos * 3;
    g = 255 - pos * 3;
    b = 0;
  }
  else if(pos < 170) {
    pos -= 85;
    r = 255 - pos * 3;
    g = 0;
    b = pos * 3;
  }
  else {
    pos -= 170;
    r = 0;
    g = pos * 3;
    b = 255 - pos * 3;
  }

  ws2812_write(r,g,b);
  rainbow++;
}

void printHelp()
{
  Serial.println();
  Serial.println("Available commands:");
  Serial.println("help          - show this help");
  Serial.println("off           - turn LED off");
  Serial.println("red           - red LED");
  Serial.println("green         - green LED");
  Serial.println("blue          - blue LED");
  Serial.println("rainbow       - start rainbow animation");
  Serial.println("rgb R G B     - set custom color (0-255)");
  Serial.println();
}

void handleCommand(String cmd)
{
  cmd.trim();

  if(cmd == "help") {
    printHelp();
  }
  else if(cmd == "off") {
    rainbowMode = false;
    ws2812_write(0,0,0);
    Serial.println("LED OFF");
  }
  else if(cmd == "red") {
    rainbowMode = false;
    ws2812_write(60,0,0);
    Serial.println("Red LED ON");
  }
  else if(cmd == "green") {
    rainbowMode = false;
    ws2812_write(0,60,0);
    Serial.println("Green LED ON");
  }
  else if(cmd == "blue") {
    rainbowMode = false;
    ws2812_write(0,0,60);
    Serial.println("Blue LED ON");
  }
  else if(cmd == "rainbow") {
    rainbowMode = true;
    Serial.println("Rainbow animation started");
  }
  else if(cmd.startsWith("rgb")) {
    int r,g,b;
    sscanf(cmd.c_str(),"rgb %d %d %d",&r,&g,&b);

    rainbowMode = false;
    ws2812_write(r,g,b);

    Serial.print("Custom color set: ");
    Serial.print(r); Serial.print(" ");
    Serial.print(g); Serial.print(" ");
    Serial.println(b);
  }
  else {
    Serial.println("Unknown command. Type 'help'.");
  }
}

void setup()
{
  Serial.begin(115200);

  ws2812_init();

  Serial.println();
  Serial.println("ESP32-S2 RGB Serial Control Demo");
  printHelp();
}

void loop()
{
  if(Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }

  if(rainbowMode)
  {
    if(millis() - lastAnim > 30)
    {
      rainbowStep();
      lastAnim = millis();
    }
  }
}
