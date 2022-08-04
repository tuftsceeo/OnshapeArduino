# Arduino Onshape API Client

An API client that allows Arduino Nano to interact with OnShape's API

## Setup/Installation

### API Key

To start, you'll want to create an API Key/Secret Key pair [here](https://dev-portal.onshape.com/keys)

### Installing Libraries

This client is made up of the following libraries:

- [Base64_Codec](https://www.arduino.cc/reference/en/libraries/base64_codec/): Encodes the authentication key in base 64
- OnShape: The actual client interface

[Base64_Codec] can be installed through the Arduino IDE's Library Manager.OnShape must be installed from this repository. To do so, zip the contents of each folder. Then, in the Arduino IDE, you can install these two libraries through `Sketch>Include Library>Add .ZIP Library`.

## Usage

This API client can make the same API requests that are available in OnShape's API Explorer. You can find example code through the IDE in `File>Examples>OnShape` or in this repo under `OnShape/examples'.
