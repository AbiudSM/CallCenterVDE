import socket
import threading
import time, sys, os
import webbrowser
import easygui
from PyQt5.QtWidgets import QApplication, QSystemTrayIcon, QMenu
from PyQt5.QtGui import QIcon

# * GLOBAL VARIABLES
# get extension
try:
    with open('config/extension.txt','r') as extension_file:
        EXT = extension_file.read()
except:
    EXT = easygui.integerbox("Ingrese su extensión",title='CALL CENTER',lowerbound=999,upperbound=10000)
    if not EXT:
        sys.exit(0)
    else:
        EXT = str(EXT)

        # save extension file
        with open('config/extension.txt', 'w') as extension_file:
            extension_file.write(EXT)

# Get Server IP
try:
    with open('config/ip_server.txt') as server_file:
        SERVER = server_file.read() 
except Exception as e:
    easygui.msgbox(f"[SERVER] {e} \nNo se encontró la ip del servidor config/ip_server.txt", ok_button="Cerrar", title='Call Center')
    sys.exit(0)

HEADER = 64
PORT = 5050
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"
ADDR = (SERVER, PORT)

# ? previous_recieve validates repetitive and unnecessary information
previous_recieve = ''

# Connection to the server
try:
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)
except Exception as e:
    easygui.msgbox(f"[LOGOUT] {e} \nCerrando sesión...", ok_button="Cerrar", title='Call Center')
    sys.exit(0)


# * Functions
# send the extension as message to the server
def send(msg):
    while True:
        try:
            message = msg.encode(FORMAT)
            msg_length = len(message)
            send_length = str(msg_length).encode(FORMAT)
            send_length += b' ' * (HEADER - len(send_length))
            client.send(send_length)
            client.send(message)
            
            client.settimeout(1)
            try:
                recive = client.recv(2048).decode(FORMAT)
            except Exception as e:
                recive = False

            global previous_recieve
            if recive and recive != previous_recieve:
                previous_recieve = recive
                open_browser(recive)
            
            if msg == DISCONNECT_MESSAGE:
                break

            time.sleep(1)
        except Exception as e:
            easygui.msgbox(f"[LOGOUT] {e} \nSesión cerrada", ok_button="Cerrar", title='Call Center')
            os._exit(0)


def open_browser(recieve: str):
    try:
        webbrowser.open(recieve)
    except Exception as e:
        easygui.msgbox(f"[ERROR] No fue posible abrir el navegador con recive = {recieve} \n Error: {e}...", ok_button="Cerrar", title='Call Center')


# ? reset previous_recieve every 60 seconds
def timeout_msg():
    global previous_recieve
    while True:
        previous_recieve = ''
        time.sleep(60)


def logout(e):
    easygui.msgbox(f"[LOGOUT] {e} \nSesión cerrada", ok_button="Cerrar", title='Call Center')
    send(DISCONNECT_MESSAGE)
    sys.exit(0)


# Function to change extension
def change_extension():
    new_ext = easygui.integerbox("Ingrese su extensión",title='CALL CENTER',lowerbound=999,upperbound=10000)
    if new_ext:
        new_ext = str(new_ext)

        with open('config/extension.txt', 'w') as extension_file:
            extension_file.write(new_ext)

        os.execv(sys.executable, sys.argv)


# Tray icon in the Clipboard Master symbol
def menu(ext):
    app = QApplication(sys.argv)

    trayIcon = QSystemTrayIcon(QIcon('src/cc_icon.png'),parent=app)

    trayIcon.setToolTip(f'Call Center: {ext}')
    trayIcon.show()

    menu = QMenu()

    changeExtAction = menu.addAction(QIcon('src/configuration.ico'),f'Cambiar extensión ({ext})')
    changeExtAction.triggered.connect(change_extension)

    exitAction = menu.addAction(QIcon('src/close.png'),'Cerrar')
    exitAction.triggered.connect(app.quit)

    trayIcon.setContextMenu(menu)

    sys.exit(app.exec_())


# * Main run
try:
    thread_timeout = threading.Thread(target=timeout_msg, daemon=True)
    thread_timeout.start()

    thread_send = threading.Thread(target=send, args=(EXT,), daemon=True)
    thread_send.start()

    menu(EXT)
        
except Exception as e:
    logout(e)
except KeyboardInterrupt:
    logout('KeyboardInterrupt')