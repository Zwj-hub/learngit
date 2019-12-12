#-------------------------------------------------
#
# Project created by QtCreator 2019-06-13T15:07:10
#
#-------------------------------------------------

QT       +=  qml quick quick-private network opengl sql script scripttools svg xml xmlpatterns  multimedia  testlib dbus core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = Mialib
TEMPLATE = lib
CONFIG +=staticlib
sudoDEFINES += MIALIB_LIBRARY

INCLUDEPATH+= \
             src-azure/azure-iot-sdk-c/c-utility/inc \
             src-azure/azure-iot-sdk-c/c-utility/deps/umock-c/inc \
             src-azure/azure-iot-sdk-c/c-utility/deps/azure-macro-utils-c/inc \
             src-azure/azure-iot-sdk-c/iothub_client/inc \
             src-azure/azure-iot-sdk-c/umqtt/inc \
             src-azure/azure-iot-sdk-c/provisioning_client/inc \
             src-azure/azure-iot-sdk-c/c-utility/pal/generic \
             src-azure/azure-iot-sdk-c/c-utility/deps/umock-c/inc \
             src-azure/azure-iot-sdk-c/c-utility/deps/azure-macro-utils-c/inc \
             src-azure/azure-iot-sdk-c/provisioning_client/adapters \
             azure_iot_port/inc \
             OS_Abstraction \
             HAL \
             at_parser \
             cpmPlusEmbeddedSDK/SDK/src/MIA_Driver \
             cpmPlusEmbeddedSDK/SDK/src/MIA_Websocket \
             ABBALib \
             CloudLib \
             interface \
             wolfssl \
             wolfssl/wolfssl \
             wolfssl_port \
             cpm_port    \




# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to              how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ABBALib/ABBA_Configuration.c \
    ABBALib/ABBA_EventMsg.c \
    ABBALib/ABBA_Messages.c \
    ABBALib/ABBA_PanelInfo.c \
    ABBALib/ABBA_Register.c \
    ABBALib/ABBA_UnitTest.c \
    CloudLib/CL_ABBAbilityDeviceRegistration.c \
    CloudLib/CL_ABBAbilitySerialiser.c \
    CloudLib/CL_ABBAbilityTelemetry.c \
    CloudLib/CL_DebugSerialiser.c \
    CloudLib/CL_Device.c \
    CloudLib/CL_Manager.c \
    CloudLib/CL_Meas.c \
    CloudLib/CL_MeasBuff.c \
    CloudLib/CL_Parameter.c \
    CloudLib/CL_ParameterList.c \
    CloudLib/CL_Serialiser.c \
    CloudLib/CL_String.c \
    CloudLib/CL_UnitTest.c \
    at_parser/at_handler.c \
    at_parser/base64.c \
    at_parser/modem_driver.c \
    at_parser/quactel_bc26_driver.c \
    azure-iot-sdk-c/c-utility/azure_base32.c \
    azure-iot-sdk-c/c-utility/buffer.c \
    azure-iot-sdk-c/c-utility/connection_string_parser.c \
    azure-iot-sdk-c/c-utility/consolelogger.c \
    azure-iot-sdk-c/c-utility/constbuffer.c \
    azure-iot-sdk-c/c-utility/constmap.c \
    azure-iot-sdk-c/c-utility/crt_abstractions.c \
    azure-iot-sdk-c/c-utility/doublylinkedlist.c \
    azure-iot-sdk-c/c-utility/hmacsha256.c \
    azure-iot-sdk-c/c-utility/map.c \
    azure-iot-sdk-c/c-utility/optionhandler.c \
    azure-iot-sdk-c/c-utility/sastoken.c \
    azure-iot-sdk-c/c-utility/sha1.c \
    azure-iot-sdk-c/c-utility/sha224.c \
    azure-iot-sdk-c/c-utility/sha384-512.c \
    azure-iot-sdk-c/c-utility/singlylinkedlist.c \
    azure-iot-sdk-c/c-utility/string_tokenizer.c \
    azure-iot-sdk-c/c-utility/strings.c \
    azure-iot-sdk-c/c-utility/tlsio_options.c \
    azure-iot-sdk-c/c-utility/urlencode.c \
    azure-iot-sdk-c/c-utility/usha.c \
    azure-iot-sdk-c/c-utility/vector.c \
    azure-iot-sdk-c/c-utility/xio.c \
    azure-iot-sdk-c/c-utility/xlogging.c \
    azure-iot-sdk-c/iothub_client/iothub_client_authorization.c \
    azure-iot-sdk-c/iothub_client/iothub_client_core_ll.c \
    azure-iot-sdk-c/iothub_client/iothub_client_ll.c \
    azure-iot-sdk-c/iothub_client/iothub_client_retry_control.c \
    azure-iot-sdk-c/iothub_client/iothub_device_client_ll.c \
    azure-iot-sdk-c/iothub_client/iothub_message.c \
    azure-iot-sdk-c/iothub_client/iothub_transport_ll_private.c \
    azure-iot-sdk-c/iothub_client/iothubtransport_mqtt_common.c \
    azure-iot-sdk-c/iothub_client/iothubtransportmqtt.c \
    azure-iot-sdk-c/provisioning_client/hsm_client_data.c \
    azure-iot-sdk-c/provisioning_client/iothub_auth_client.c \
    azure-iot-sdk-c/provisioning_client/iothub_security_factory.c \
    azure-iot-sdk-c/provisioning_client/prov_auth_client.c \
    azure-iot-sdk-c/provisioning_client/prov_device_ll_client.c \
    azure-iot-sdk-c/provisioning_client/prov_security_factory.c \
    azure-iot-sdk-c/provisioning_client/prov_transport_mqtt_client.c \
    azure-iot-sdk-c/provisioning_client/prov_transport_mqtt_common.c \
    azure-iot-sdk-c/umqtt/mqtt_client.c \
    azure-iot-sdk-c/umqtt/mqtt_codec.c \
    azure-iot-sdk-c/umqtt/mqtt_message.c \
    azure_iot_port/src/DPSConncetion.c \
    azure_iot_port/src/agenttime_AzureIoTPanel.c \
    azure_iot_port/src/azure_base64.c \
    azure_iot_port/src/azure_hmac.c \
    azure_iot_port/src/hsm_panel.c \
    azure_iot_port/src/lock_AzureIoTPanel.c \
    azure_iot_port/src/parson.c \
    azure_iot_port/src/platform_AzureIoTPanel.c \
    azure_iot_port/src/threadapi_AzureIoTPanel.c \
    azure_iot_port/src/tickcounter_AzureIoTPanel.c \
    azure_iot_port/src/tlsio_AzureIoTPanel.c \
    cpm/Mia_Base.cpp \
    cpm/Mia_Device.cpp \
    cpm/Mia_Driver.cpp \
    cpm/Mia_DriverBase.cpp \
    cpm/Mia_Equipment.cpp \
    cpm/Mia_Exception.cpp \
    cpm/WS_Error.c \
    cpm/WS_Util.c \
    cpm/WS_Websocket.c \
    cpm_port/Mia_Base_Osal.cpp \
    cpm_port/WS_Socket_Osal.c \
    interface/MIA_Certificate.c \
    interface/azureIoTLib.cpp \
    interface/miaLib.cpp \
    wolfssl/aes.c \
    wolfssl/arc4.c \
    wolfssl/asn.c \
    wolfssl/coding.c \
    wolfssl/curve25519.c \
    wolfssl/des3.c \
    wolfssl/dh.c \
    wolfssl/dsa.c \
    wolfssl/ecc.c \
    wolfssl/ecc_fp.c \
    wolfssl/error.c \
    wolfssl/fe_operations.c \
    wolfssl/hash.c \
    wolfssl/hmac.c \
    wolfssl/integer.c \
    wolfssl/internal.c \
    wolfssl/keys.c \
    wolfssl/logging.c \
    wolfssl/md4.c \
    wolfssl/md5.c \
    wolfssl/memory.c \
    wolfssl/pkcs12.c \
    wolfssl/pwdbased.c \
    wolfssl/rabbit.c \
    wolfssl/random.c \
    wolfssl/rsa.c \
    wolfssl/sha.c \
    wolfssl/sha256.c \
    wolfssl/signature.c \
    wolfssl/ssl.c \
    wolfssl/tls.c \
    wolfssl/wc_encrypt.c \
    wolfssl/wc_port.c \
    wolfssl/wolfio.c \
    wolfssl/wolfmath.c \
    wolfssl/wolfssl/wolfcrypt/src/misc.c






HEADERS += \
    ABBALib/ABBALib.h \
    define.h \
    interface/build_configuration.h \
    interface/build_configuration_os.h \
    interface/miaLib.h \
    wolfssl_port/user_settings.h






unix {
    target.path = /usr/lib
    INSTALLS += target
}



