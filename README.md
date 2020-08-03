# Arduino OnShape API Client

An API client that allows Arduino Nano to interact with OnShape's API

## Setup/Installation

### API Key

To start, you'll want to create an API Key/Secret Key pair [here](https://dev-portal.onshape.com/keys)

### Installing Libraries

This client is made up of four libraries:

- TimeLib: Built-in library used to get dates for [request signature](https://dev-portal.onshape.com/keys)
- SHA256: Creates the HMAC digest for authorization
- [Base64](https://github.com/Densaugeo/base64_arduino): Encodes the HMAC in base 64
- ArduinoOnShapeLib: The actual client interface

TimeLib and [Base64](https://github.com/Densaugeo/base64_arduino) can be installed through the Arduino IDE's Library Manager. SHA256 and the OnShape client must be installed from this repository. To do so, zip the contents of each folder into a zip. Then, in the Arduino IDE, you can install each of the three libraries through `Sketch>Include Library>Add .ZIP Library`.

## Usage

This API client can make the same API requests that are availible in OnShape's API Explorer. You can find example code through the IDE in `File>Examples>ArduinoOnShapeLib`.