# B&R Bridge

The B&R Bridge is an [NVIDIA Omniverse](https://www.nvidia.com/en-us/omniverse/) extension for communicating with [B&R PLCs](https://www.br-automation.com/) using websockets and the [OMJSON](https://github.com/loupeteam/OMJSON) library.

# Installation

### Install from registry

This is the preferred method. Open up the extensions manager by navigating to `Window / Extensions`. The extension is available as a "Third Party" extension. Search for `B&R Bridge`, and click the slider to Enable it. Once enabled, the extension will be available as an option in the top menu banner of the Omniverse app.

### Install from source

You can also install from source instead. In order to do so, follow these steps:
- Clone the repo [here](https://github.com/loupeteam/Omniverse_BnR_Bridge_Extension).
- In your Omniverse app, open the extensions manager by navigating to `Window / Extensions`.
- Open the general extension settings, and add a new entry into the `Extension Search Paths` table. This should be the local path to the root of the repo that was just cloned. 
- Back in the extensions manager, search for `B&R BRIDGE`, and enable it. 
- Once enabled, the extension will show up as an option in the top menu banner. 

# Configuration

You can open the extension by clicking on `B&R Bridge / Open Bridge Settings` from the top menu. The following configuration options are available:

- Enable Client: Enable or disable the client from reading or writing data to the PLC.
- Refresh Rate: The rate at which the client will read data from the PLC in milliseconds.
- PLC IP and Port: IP and Port of the PLC to connect to.
- Settings commands: These commands are used to load and save the extension settings as permanent parameters. The Save button backs up the current parameters, and the Load button restores them from the last saved values. 

# Usage

Once the extension is enabled, the B&R Bridge will attempt to connect to the PLC.

### Monitoring Extension Status

The status of the extension can be viewed in the `Status` field. Here are the possible messages and their meaning:
- `Disabled`: the enable checkbox is unchecked, and no communication is attempted. 
- `Attempting to connect...`: the client is trying to connect to the PLC. Staying in this state for more than a few seconds indicates that there is a problem with the connection. 
- `Connected`: the client has successfully established a connection with the PLC. 
- `Error writing data to the PLC: [...]`: an error occurred while performing a variable write. 
- `Error reading data from the PLC: [...]`: an error occurred while performing a variable read.

### Monitoring Variable Values

Once variable reads are occurring, the `Monitor` pane will show a JSON string with the names and values of the variables being read. This is helpful for troubleshooting. 

### Performing read/write operations

The variables on the PLC that should be read or written are specified in a custom user extension or app that uses the API available from the `loupe.simulation.br_bridge` module.

```python
from loupe.simulation.br_bridge import BRBridge
      
# Instantiate the bridge
br_bridge = BRBridge.Manager()

# This function gets called once on init, and should be used to subscribe to cyclic reads.
def on_plc_init( event ):
    # Create a list of variable names to be read cyclically, and add to Manager
    variables = [   'MAIN:custom_struct.var1', 
                    'MAIN:custom_struct.var_array[0]', 
                    'MAIN:custom_struct.var_array[1]']

    br_bridge.add_cyclic_read_variables(variables)

# This function is called every time the bridge receives new data
def on_message( event ):
    # Read the event data, which includes values for the PLC variables requested
    data = event.payload['data']['MAIN']['custom_struct']['var_array']

# In the app's cyclic logic (for example on_physics_step(), etc), writes can be performed as follows:
def cyclic():
    # Write the value `1` to PLC variable 'MAIN.custom_struct.var1'
    br_bridge.write_variable('MAIN:custom_struct.var1', 1)

# Register lifecycle subscriptions
br_bridge.register_init_callback(on_plc_init)
br_bridge.register_data_callback(on_message)

```