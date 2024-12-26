<!-- vscode-markdown-toc -->
* 1. [Compatibility](#Compatibility)
* 2. [Related documents](#Relateddocuments)
* 3. [Installation](#Installation)
	* 3.1. [Deploying to RAKwireless 4631 platform.](#DeployingtoRAKwireless4631platform.)
	* 3.2. [ Deploying to Omnet platform.](#DeployingtoOmnetplatform.)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->


# SCHC_Node.cpp


This project is an implememntation of the SCHC standard defined in [RFC 8724](https://www.rfc-editor.org/rfc/rfc8724.html), [RFC 9011](https://www.rfc-editor.org/rfc/rfc9011.html), and [RFC 9441](https://www.rfc-editor.org/rfc/rfc9441.html). The implementation is on the end device or node side.
**SCHC_Node.cpp** works over RAKWireless 4631 hardware platform. You can learn more about the RAK4631 platform [here](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/#product-description).

##  1. <a name='Compatibility'></a>Compatibility
This SCHC implementation supports the ACK-on-Error mode over uplink defined in [RFC 8724](https://www.rfc-editor.org/rfc/rfc8724.html) and [RFC 9011](https://www.rfc-editor.org/rfc/rfc9011.html).

##  2. <a name='Relateddocuments'></a>Related documents
 * [RFC 8724:
SCHC: Generic Framework for Static Context Header Compression and Fragmentation](https://www.rfc-editor.org/rfc/rfc8724.html)
 * [RFC 9011:
Static Context Header Compression and Fragmentation (SCHC) over LoRaWAN](https://www.rfc-editor.org/rfc/rfc9441.html)
 * [RFC 9441:
Static Context Header Compression (SCHC) Compound Acknowledgement (ACK)](https://www.rfc-editor.org/rfc/rfc9441.html)


##  3. <a name='Installation'></a>Installation


The ***test*** directory contains different ***main*** files. Each main file is designed for a different environment. The supported environments are:

 * __RAKWireless 4631 hardware platform__: You can learn more about the RAK4631 platform [here](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/#product-description).
 * __Omnet++ 6.1__:  

Each main file must be deployed according to the platform for which it was designed. Each deployment consists of copying the main file to the main directory of this workspace.

###  3.1. <a name='DeployingtoRAKwireless4631platform.'></a>Deploying to RAKwireless 4631 platform. 

Copy the ***platform.ini***, ***library.json***, and ***main.cpp*** files from ***test/rak4631*** directory to root workspace


