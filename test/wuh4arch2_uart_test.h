#include "esphome.h"
#include <string>

class WUH4ARCH2_UART : public Component,
                       public UARTDevice
{
public:
  TextSensor *last_line = new TextSensor();
  TextSensor *last_error = new TextSensor();
  TextSensor *hdmi_source = new TextSensor();
  TextSensor *audio_input = new TextSensor();
  TextSensor *switch_mode = new TextSensor();
  TextSensor *edid = new TextSensor();
  TextSensor *hdcp_status = new TextSensor();
  TextSensor *firmware_version = new TextSensor();

  WUH4ARCH2_UART(UARTComponent *parent) : UARTDevice(parent) {}

  void setup() override
  {
  }

  void remove_prefix(char *str, const char *prefix)
  {
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);

    if (strncmp(str, prefix, prefix_len) == 0)
    {                                                           // Check if the string starts with the prefix
      memmove(str, str + prefix_len, str_len - prefix_len + 1); // Move the rest of the string to the beginning
    }
  }

  int readline(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;

    if (readch > 0)
    {
      switch (readch)
      {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0; // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len - 1)
        {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  void loop() override
  {
    const int max_line_length = 120;
    static char buffer[max_line_length];
    static char line[max_line_length];

    while (available())
    {
      if (readline(read(), buffer, max_line_length) > 0)
      {
        std::strcpy(line, buffer);

        remove_prefix(line, "<<");

        last_line->publish_state(line);

        if (std::strncmp(line, "HDMI", 4) == 0) // HDMI1,HDMI2,HDMI3,HDMI4
        {
          if (sizeof(line) > 4)
          {
            hdmi_source->publish_state(line); // Return 1,2,3,4
          }
        }
        else if (strcmp(line, "AUDExternal") == 0)
        {
          audio_input->publish_state("ARC");
        }
        else if (strcmp(line, "AUDInternal") == 0)
        {
          audio_input->publish_state("HDMI");
        }
        else if (strcmp(line, "AUTO Switch") == 0)
        {
          switch_mode->publish_state("Auto");
        }
        else if (strcmp(line, "MANUAL Switch") == 0)
        {
          switch_mode->publish_state("Manual");
        }
        else if (std::strncmp(line, "HDCP", 4) == 0)
        {
          hdcp_status->publish_state(line);
        }
        else if (std::strncmp(line, "EDID", 4) == 0)
        {
          edid->publish_state(line);
        }
        else if (std::strncmp(line, " VER", 4) == 0) // During a reboot this comes in with out the space. Need to look at removing leading spaces.
        {
          firmware_version->publish_state(line);
        }
        else if (strcmp(line, " --------") == 0)
        {
          // System Info Results
        }
        else if (strcmp(line, " WUH4ARC-H2") == 0)
        {
          // Model Number
        }
        else
        {
          last_error->publish_state("|" + std::string(line) + "|"); //<<-- going to need to add logic to this. Need to capture what command is sent and check that the respnse matches. If not thne show it as an error.
        }
      }
    }
  }
};