# SCHC_Node.cpp


This project is an implememntation of the SCHC standard defined in [RFC 8724](https://www.rfc-editor.org/rfc/rfc8724.html), [RFC 9011](https://www.rfc-editor.org/rfc/rfc9011.html), and [RFC 9441](https://www.rfc-editor.org/rfc/rfc9441.html). The implementation is on the end device or node side.
**SCHC_Node.cpp** works over RAKWireless 4631 hardware platform. You can learn more about the RAK4631 platform [here](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/#product-description).


<!-- vscode-markdown-toc -->
* 1. [Compatibility](#Compatibility)
* 2. [Related documents](#Relateddocuments)
* 3. [ Prerequisites](#Prerequisites)
	* 3.1. [Visual Studio Code](#VisualStudioCode)
	* 3.2. [PlatformIO](#PlatformIO)
	* 3.3. [ RAKWireless Boards](#RAKWirelessBoards)
* 4. [ Deploy](#Deploy)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->


##  1. <a name='Compatibility'></a>Compatibility
This SCHC implementation supports the ACK-on-Error mode over uplink defined in [RFC 8724](https://www.rfc-editor.org/rfc/rfc8724.html) and [RFC 9011](https://www.rfc-editor.org/rfc/rfc9011.html).

##  2. <a name='Relateddocuments'></a>Related documents
 * [RFC 8724:
SCHC: Generic Framework for Static Context Header Compression and Fragmentation](https://www.rfc-editor.org/rfc/rfc8724.html)
 * [RFC 9011:
Static Context Header Compression and Fragmentation (SCHC) over LoRaWAN](https://www.rfc-editor.org/rfc/rfc9441.html)
 * [RFC 9441:
Static Context Header Compression (SCHC) Compound Acknowledgement (ACK)](https://www.rfc-editor.org/rfc/rfc9441.html)


##  3. <a name='Prerequisites'></a> Prerequisites

To deploy ***SCHC Node*** on a RAK4631 development board you must have the following tools installed:

 * Visual Studio Code
 * PlatformIO para VSCode (VSCode extension)
 * Board Support Package for RAK4631


###  3.1. <a name='VisualStudioCode'></a>Visual Studio Code
Install Visual Studio Code which is a great and open source tool, and you can download it here:

https://code.visualstudio.com/

###  3.2. <a name='PlatformIO'></a>PlatformIO

PlatformIO is a cross-platform, cross-architecture, multiple framework, professional tool for embedded systems engineers and for software developers who write applications for embedded products.

PlatformIO is used in VSCode through an extension. To install PlatformIO in VSCode follow the steps in this [link](https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation).

###  3.3. <a name='RAKWirelessBoards'></a> RAKWireless Board Support Package

To deploy software on the RAK4631 board you need to configure PlatformIO. In the following [link](https://learn.rakwireless.com/hc/en-us/articles/26687276346775-How-To-Perform-Installation-of-Board-Support-Package-in-PlatformIO) you can find how to perform installation of Board Support Package in PlatformIO.

 
##  Setting up the deployment environment

Before deploying, you must have the tools from the previous point installed. Then you must follow the next steps to configure your deployment environment.

### Step 1: Create you workspace

To create your workspace you need to download the SCHC node source code. Open VSCode and click on ***Clone Git Repository***. In the *repository url* use the follow link:

```https://github.com/RodrigoMunozLara/SCHC_Node.cpp.git```

> **Note:** We assume you have configured Git source control in VS Code. If you haven't done so yet, click [here](https://code.visualstudio.com/docs/sourcecontrol/overview).

### Step 2: Pick a folder in PlatformIO

To develop code with PlatformIO over VSCode you must create a new project or pick a folder. SCHC node include a *platformio.ini* file that you can use it. For this, click in PlatformIO icon in VSCode and select 

### Step 3: Install the libraries

SCHC node uses some libraries to work with timers and SX126x chipset for Arduino. The libraries are installed in PlatformIO


 The deploy process is very simple. The deploy assumes that you are using PlatformIO over VSCode and you already have PlatformIO configured for a RAK4631 board.



 Before doing a deploy it is necessary that you install the following libraries on platformIO. 
 
  * sstaub/Ticker@^4.4.0
  * beegee-tokyo/SX126x-Arduino@^2.0.23
 
 To find out how to install a library in PlatformIO, follow this link


