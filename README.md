# CALL CENTER VDE

![](https://www.vde.com.mx/media/wysiwyg/contacto/store-image.png)


## Introducción
Este sistema fue creado con la intención de mejorar la experiencia al recibir o realizar llamadas a clientes de la empresa por parte del equipo de ventas conectados a sus respectivas extensiones. La funcionalidad del sistema consta de monitorear las llamadas de clientes y una vez se detecte el número del cliente emisor se desplegará una ventana en el navegador con la información de dicho cliente, esto en el equipo que esté conectado a la extensión por parte del departamento de ventas. 

### Tecnologías utilizadas
- Call monitor 3 (Panasonic)
- Python 3.7.2
- Visual Studio 2013

### Requerimientos
#### Call Monitor
- Open Source Tool CallMonitor3
- Visual Studio 2013
- vc_mbcsmfc

#### Python
> easygui==0.98.2
> PyQt5==5.15.6
> rich==11.2.0

### Sistema
Para el correcto funcionamiento del sistema deben estar funcionando los 3 siguientes elementos: Monitoreo de llamadas (Call Monitor), envió de llamadas a las extensiones (Servidor) y el receptor de llamadas para despliegue de información (Cliente o extensión).

Para iniciar el Call Monitor tenemos que ejecutar la aplicación “CallMonitor3.exe” que se encuentra en la carpeta Debug del Call Monitor, también se encuentra en el escritorio como acceso directo.

Para iniciar el Servidor se tiene que abrir la aplicación “server.exe” que se encuentra en la carpeta Debug del Call Monitor, al igual que el Call Monitor, el servidor también se encuentra en el escritorio como acceso directo. Para modificar las URL de llamadas entrantes y salientes, se editan los archivos de texto que se encuentran en la carpeta config.

Para iniciar el cliente se debe tener la carpeta “Cliente” en alguno de los equipos del departamento de ventas, conectados a la red de la empresa para su correcto funcionamiento. Se debe abrir la aplicación“client.exe”. Esta carpeta debe contar con dos directorios importantes: config y src. Para modificar la dirección IP en la que se encuentra el servidor se debe editar el archivo de texto que se encuentra en la carpeta config.


## Funcionamiento
Se realiza un monitoreo de las llamadas entrantes y se realizan las respectivas validaciones para números telefónicos de nuestros clientes, se cuentan con 3 principales programas para el funcionamiento del sistema, el detector de llamadas (Call Monitor Panasonic), el servidor y el cliente (ventas).

### Call Monitor Panasonic
Call Monitor 3 es una herramienta open source proporcionada por Panasonic para la detección de llamadas por medio del conmutador Panasonic, se utilizó está herramienta para generar archivos que reportan las llamadas entrantes de los respectivos clientes. Las llamadas son almacenadas en la carpeta ExtText, las cuales son manejadas por el servidor.exe para el envió de información a los equipos del departamento de ventas, este cambio fue agregado en el método SetInfoData del archivo CallData.cpp

Se obtiene tanto el estado de la llamada como el nombre de la línea, la llamada será registrada si la llamada entró en alguna de las extensiones y si el estado de la llamada es diferente a ONHOLD y DISCONNECTED.

```cpp
string status = pTree->GetItemText(hTree);
HTREEITEM prevCTree = pTree->GetPrevVisibleItem(hTree);
string topItem = pTree->GetItemText(prevCTree);
string lineName = topItem.substr(0, 4);

if (is_number(lineName) && status.substr(0, 6) != "ONHOLD" && status.substr(0, 10) != "DISCONNECT"){
	CString callerID = pCInfo->GetCallerID();
	CString calledID = pCInfo->GetCalledID();
	CString redirectionID = pCInfo->GetRedirectionID();
	CString textFile = "null";

	if (callerID.GetLength() > 8){
		textFile = "entrante:" + callerID;
	}
	if (calledID.GetLength() > 8){
		textFile = "saliente:" + calledID;
	}
	if (redirectionID.GetLength() > 8){
		textFile = "entrante:" + redirectionID;
	}

	if (textFile != "null"){
		ofstream myfile;
		string file_name = "ExtText/" + lineName + ".txt";
		myfile.open(file_name.c_str(), ios::out);
		myfile << textFile;
		myfile.close();
	}
}
```

Una vez realizadas las validaciones, se obtiene como información el emisor, receptor y el número de redireccionamiento, la información que contendrá el archivo será el número de más de 8 caracteres y el tipo de llamada, este archivo será guardado con el nombre de la extensión dentro de la carpeta ExtText


### Servidor
El servidor es un script creado en Python para monitorear los archivos generados por el Call Monitor y enviarlos a los equipos conectados del departamento de ventas, por medio de hilos se obtienen las extensiones (clientes) que estén conectados, en caso de encontrar una llamada entrante por parte del Call Monitor, el servidor mandará como información el número de teléfono del cliente y URL correspondiente que será desplegada en el equipo del departamento de ventas.

### Cliente
El cliente es un script creado en Python para conectarse al servidor, este recibirá la información que es enviada por el servidor a las respectivas extensiones conectadas. El procedimiento para conectarse es simple, se ejecuta el programa y se ingresa la extensión, cuando se recibe una llamada el programa despliega en el navegador la información de la llamada que detectó el servidor. 