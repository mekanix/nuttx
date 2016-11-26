#include "Pin.h"

#include "lpc43_pinconfig.h"
#include "lpc43_gpio.h"

#include <cctype>
#include <algorithm>

Pin::Pin()
{
    this->inverting = false;
    this->valid = false;
}

Pin::Pin(const char *s)
{
    this->inverting = false;
    this->valid = false;
    from_string(s);
}


std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), std::toupper);
    return str;
}

// look up table to convert GPIO port/pin into a PINCONF
static const uint32_t port_pin_lut[NUM_GPIO_PORTS][NUM_GPIO_PINS] = {
    {PINCONF_GPIO0p0, PINCONF_GPIO0p1, PINCONF_GPIO0p2,  PINCONF_GPIO0p3,  PINCONF_GPIO0p4,  PINCONF_GPIO0p5,  PINCONF_GPIO0p6, PINCONF_GPIO0p7,
     PINCONF_GPIO0p8, PINCONF_GPIO0p9, PINCONF_GPIO0p10, PINCONF_GPIO0p11, PINCONF_GPIO0p12, PINCONF_GPIO0p13, PINCONF_GPIO0p14, PINCONF_GPIO0p15,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //port 0

    {PINCONF_GPIO1p0,  PINCONF_GPIO1p1,  PINCONF_GPIO1p2,  PINCONF_GPIO1p3,  PINCONF_GPIO1p4,  PINCONF_GPIO1p5,  PINCONF_GPIO1p6,  PINCONF_GPIO1p7,
     PINCONF_GPIO1p8,  PINCONF_GPIO1p9,  PINCONF_GPIO1p10, PINCONF_GPIO1p11, PINCONF_GPIO1p12, PINCONF_GPIO1p13, PINCONF_GPIO1p14, PINCONF_GPIO1p15,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //port 1

    {PINCONF_GPIO2p0,  PINCONF_GPIO2p1,  PINCONF_GPIO2p2,  PINCONF_GPIO2p3,  PINCONF_GPIO2p4,  PINCONF_GPIO2p5,  PINCONF_GPIO2p6,  PINCONF_GPIO2p7,
     PINCONF_GPIO2p8,  PINCONF_GPIO2p9,  PINCONF_GPIO2p10, PINCONF_GPIO2p11, PINCONF_GPIO2p12, PINCONF_GPIO2p13, PINCONF_GPIO2p14, PINCONF_GPIO2p15,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //port 2

    {PINCONF_GPIO3p0,  PINCONF_GPIO3p1,  PINCONF_GPIO3p2,  PINCONF_GPIO3p3,  PINCONF_GPIO3p4,  PINCONF_GPIO3p5,  PINCONF_GPIO3p6,  PINCONF_GPIO3p7,
     PINCONF_GPIO3p8,  PINCONF_GPIO3p9,  PINCONF_GPIO3p10, PINCONF_GPIO3p11, PINCONF_GPIO3p12, PINCONF_GPIO3p13, PINCONF_GPIO3p14, PINCONF_GPIO3p15,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //port 3

    {PINCONF_GPIO4p0,  PINCONF_GPIO4p1,  PINCONF_GPIO4p2,  PINCONF_GPIO4p3,  PINCONF_GPIO4p4,  PINCONF_GPIO4p5,  PINCONF_GPIO4p6,  PINCONF_GPIO4p7,
     PINCONF_GPIO4p8,  PINCONF_GPIO4p9,  PINCONF_GPIO4p10, PINCONF_GPIO4p11, PINCONF_GPIO4p12, PINCONF_GPIO4p13, PINCONF_GPIO4p14, PINCONF_GPIO4p15,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //port 4

    {PINCONF_GPIO5p0,  PINCONF_GPIO5p1,  PINCONF_GPIO5p2,  PINCONF_GPIO5p3,  PINCONF_GPIO5p4,  PINCONF_GPIO5p5,  PINCONF_GPIO5p6,  PINCONF_GPIO5p7,
     PINCONF_GPIO5p8,  PINCONF_GPIO5p9,  PINCONF_GPIO5p10, PINCONF_GPIO5p11, PINCONF_GPIO5p12, PINCONF_GPIO5p13, PINCONF_GPIO5p14, PINCONF_GPIO5p15,
     PINCONF_GPIO5p16, PINCONF_GPIO5p17, PINCONF_GPIO5p18, PINCONF_GPIO5p19, PINCONF_GPIO5p20, PINCONF_GPIO5p21, PINCONF_GPIO5p22, PINCONF_GPIO5p23,
     PINCONF_GPIO5p24, PINCONF_GPIO5p25, PINCONF_GPIO5p26,
     0,0,0,0,0}, //port 5

     {PINCONF_GPIO6p0, PINCONF_GPIO6p1,  PINCONF_GPIO6p2,  PINCONF_GPIO6p3,  PINCONF_GPIO6p4,  PINCONF_GPIO6p5,  PINCONF_GPIO6p6,  PINCONF_GPIO6p7,
     PINCONF_GPIO6p8,  PINCONF_GPIO6p9,  PINCONF_GPIO6p10, PINCONF_GPIO6p11, PINCONF_GPIO6p12, PINCONF_GPIO6p13, PINCONF_GPIO6p14, PINCONF_GPIO6p15,
     PINCONF_GPIO6p16, PINCONF_GPIO6p17, PINCONF_GPIO6p18, PINCONF_GPIO6p19, PINCONF_GPIO6p20, PINCONF_GPIO6p21, PINCONF_GPIO6p22, PINCONF_GPIO6p23,
     PINCONF_GPIO6p24, PINCONF_GPIO6p25, PINCONF_GPIO6p26, PINCONF_GPIO6p27, PINCONF_GPIO6p28, PINCONF_GPIO6p29, PINCONF_GPIO6p30,
     0}, //port 6

     {PINCONF_GPIO5p0, PINCONF_GPIO5p1,  PINCONF_GPIO5p2,  PINCONF_GPIO5p3,  PINCONF_GPIO5p4,  PINCONF_GPIO5p5,  PINCONF_GPIO5p6,  PINCONF_GPIO5p7,
     PINCONF_GPIO5p8,  PINCONF_GPIO5p9,  PINCONF_GPIO5p10, PINCONF_GPIO5p11, PINCONF_GPIO5p12, PINCONF_GPIO5p13, PINCONF_GPIO5p14, PINCONF_GPIO5p15,
     PINCONF_GPIO5p16, PINCONF_GPIO5p17, PINCONF_GPIO5p18, PINCONF_GPIO5p19, PINCONF_GPIO5p20, PINCONF_GPIO5p21, PINCONF_GPIO5p22, PINCONF_GPIO5p23,
     PINCONF_GPIO5p24, PINCONF_GPIO5p25,
     0,0,0,0,0,0} //port 7
};

// given the physical port and pin (P2.7) finds the GPIO port and pin (GPIO0[7])
static bool lookup_pin(uint16_t port, uint16_t pin, uint16_t& gpioport, uint16_t& gpiopin)
{
    for (int i = 0; i < NUM_GPIO_PORTS; ++i) {
        for (int j = 0; j < NUM_GPIO_PINS; ++j) {
            uint32_t v= port_pin_lut[i][j];
            if(v == 0) continue;
            if( ((v&PINCONF_PINS_MASK)>>PINCONF_PINS_SHIFT) == port && ((v&PINCONF_PIN_MASK)>>PINCONF_PIN_SHIFT) == pin ) {
                gpioport= i;
                gpiopin= j;
                return true;
            }
        }
    }

    return false;
}

// Make a new pin object from a string
// Pins are defined for the LPC43xx as GPIO names GPIOp[n] or gpiop_n where p is the GPIO port and n is the pin (NOT pin names eg P1_6 these are not used)
Pin* Pin::from_string(std::string value)
{
    valid= false;
    inverting= false;

    uint16_t port= 0;
    uint16_t pin= 0;
    size_t pos= 0;
    if(toUpper(value.substr(0, 4)) == "GPIO") {
        // grab first integer as GPIO port.
        port = strtol(value.substr(4).c_str(), nullptr, 10);
        pos= value.find_first_of("[_", 4);
        if(pos == std::string::npos) return nullptr;

        // grab pin number
        pin = strtol(value.substr(pos+1).c_str(), nullptr, 10);

    }else if(toUpper(value.substr(0, 1)) == "P") {
        uint16_t x= strtol(value.substr(1).c_str(), nullptr, 10);
        pos= value.find_first_of("._", 1);
        if(pos == std::string::npos) return nullptr;
        uint16_t y= strtol(value.substr(pos+1).c_str(), nullptr, 10);

        // Pin name convert to GPIO
        if(!lookup_pin(x, y, port, pin)) return nullptr;

    } else if(value.substr(0, 3) == "ADC") {
        //initialize adc only vars
        // adc_only = false;
        // adc_channel = 0xff;

        // //verify if it is an analog only pin
        // std::size_t found = value.find("adc");
        // if (found != std::string::npos) adc_only = true;

        // if(adc_only) {
        //     std::string channelStr = value.substr (4);
        //     cs = channelStr.c_str();
        //     adc_channel = strtol(cs, &cn, 10);

        //     // PinName adc_pins[16] = {adc0_0, adc0_1, adc0_2, adc0_3, adc0_4, adc0_5, adc0_6, adc0_7, adc1_0, adc1_1, adc1_2, adc1_3, adc1_4, adc1_5, adc1_6, adc1_7};
        //     // this->pinName = adc_pins[adc_channel];

        //     return this;
        // }

    }else{
        return nullptr;
    }

    if(port >= NUM_GPIO_PORTS || pin >= NUM_GPIO_PINS) return nullptr;

    // convert port and pin to a GPIO and setup as a GPIO
    uint32_t gpio= port_pin_lut[port][pin];
    if(gpio == 0) return nullptr; // not a valid pin

    // now check for modifiers:-
    // ! = invert pin
    // o = set pin to open drain
    // ^ = set pin to pull up
    // v = set pin to pull down
    // - = set pin to no pull up or down
    for(char c : value.substr(pos+1)) {
        switch(c) {
            case '!':
                this->inverting = true;
                break;
            case 'o':
                gpio |= PINCONF_FLOAT;
                break;
            case '^':
                gpio |= PINCONF_PULLUP;
                break;
            case 'v':
                gpio |= PINCONF_PULLDOWN;
                break;
            case '-':
                break;
        }
    }
    lpc43_pin_config(gpio); //configures pin for GPIO
    gpiocfg= (port << GPIO_PORT_SHIFT) | (pin << GPIO_PIN_SHIFT);

    this->valid = true;
    return this;
}

Pin* Pin::as_output()
{
    lpc43_gpio_config(gpiocfg|GPIO_MODE_OUTPUT);
    return this;
}

Pin* Pin::as_input()
{
    lpc43_gpio_config(gpiocfg|GPIO_MODE_INPUT);
    return this;
}

#if 0
// If available on this pin, return mbed hardware pwm class for this pin
mbed::PwmOut* Pin::hardware_pwm()
{
    return nullptr;
}

mbed::InterruptIn* Pin::interrupt_pin()
{
    if(!this->valid) return nullptr;
    /*
        // set as input
        as_input();

        if (port_number == 0 || port_number == 2) {
            PinName pinname = port_pin((PortName)port_number, pin);
            return new mbed::InterruptIn(pinname);

        }else{
            this->valid= false;
            return nullptr;
        }
    */
    return nullptr;
}
#endif
