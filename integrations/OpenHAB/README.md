# Setup

## HP's
Modify and flash arduino code.  Specifically pay close attention to .h file.  Make sure you get at least the following:
- ssid
- password
- mqtt_server
- client_id (must be unique on mqtt network)
- heatpump_topics (there are 6)

## OpenHAB

### Requirements:
- Transformations: Map
- Bindings: mqtt1

### Config:
This assumes basic understanding of OH configuration, bindings, files and concepts.  Also that you have a functioning OH instance already running.  See https://www.openhab.org/docs/ for OH specific documents.

- Add required binding and transform.
- Configure MQTT broker. https://www.openhab.org/addons/bindings/mqtt1/
- Copy *.map files to ohdirectory/transform
- Use the provided HP.items as examples to create your own .items file.  You will need to adjust the item names for each HP you have to something unique that makes sense to you as well as the mqtt topics to match the ones used in your arduino code earlier.
- Use the provided HP.sitemap as examples on how to integrate your new items into your sitemap.
- Don't forget to add your new items to any persistence config that meets your needs.

I use Fahrenheit as I am US based.  Some tweaks will be needed to convert to C.  I think maybe just to the arduino code.
