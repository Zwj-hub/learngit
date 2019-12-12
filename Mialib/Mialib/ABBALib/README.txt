Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.device","type":"abb.drives.iotpanel.4%401","objectId":"f8f0821f-ff74-4943-800a-7ce0e0d405f7"}
Payload
{"objectId":"f8f0821f-ff74-4943-800a-7ce0e0d405f7","type":"abb.drives.iotpanel.4@1","properties":{"serialNumberIotPanel":{"value":"Panel-5678"}},"version":2}
len:157



TX message
Message parameters
{"ability-messagetype":"platformEvent","model":"abb.ability.device","eventType":"Abb.Ability.Device.Updated"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"objectId": "f8f0821f-ff74-4943-800a-7ce0e0d405f7",
"model": "abb.ability.device",
"properties": {
	"deviceId":{ "value": "bc26panel003" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-5678" }
}
}







TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.Device.Created","model":"abb.ability.device"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel003" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-5678" }
}
}

len:216
IoTHubClient accepted the message for delivery


Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelCreated","model":"abb.ability.device","type":"abb.drives.iotpanel.4%401","objectId":"f8f0821f-ff74-4943-800a-7ce0e0d405f7"}
Payload
{"objectId":"f8f0821f-ff74-4943-800a-7ce0e0d405f7","type":"abb.drives.iotpanel.4@1","properties":{"serialNumberIotPanel":{"value":"Panel-5678"}},"version":1}
len:157




Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.configuration","type":"abb.drives.iotpanel.4%401","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"type":"abb.drives.iotpanel.config.4@16","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","version":16,"model":"abb.ability.configuration","properties":{"serialNumberIotPanel":{"value":"Panel-4321"},"firmwareVersion":{"mandatory":true,"dataType":"string"},"simIccid":{"mandatory":true,"dataType":"string"},"networkOperator":{"mandatory":true,"dataType":"string"},"driveTypes":{"driveType_1":{"description":"ACS580","mandatory":true,"dataType":"string","value":"abb.drives.acs580.4@1","firmware":"ASCD*","supported":true},"driveType_2":{"description":"ACS880 single drive or INU","mandatory":true,"dataType":"string","value":"abb.drives.acs880.inu.4@1","firmware":"AINF*","supported":true}}}}
len:693

Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.device","type":"abb.drives.iotpanel.4%401","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","type":"abb.drives.iotpanel.4@1","properties":{"serialNumberIotPanel":{"value":"Panel-4321"}},"version":17}
len:158

Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.configuration","type":"abb.drives.iotpanel.4%401","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"type":"abb.drives.iotpanel.config.4@17","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","version":17,"model":"abb.ability.configuration","properties":{"serialNumberIotPanel":{"value":"Panel-4321"},"firmwareVersion":{"mandatory":true,"dataType":"string"},"simIccid":{"mandatory":true,"dataType":"string"},"networkOperator":{"mandatory":true,"dataType":"string"},"driveTypes":{"driveType_1":{"description":"ACS580","mandatory":true,"dataType":"string","value":"abb.drives.acs580.4@1","firmware":"ASCD*","supported":true},"driveType_2":{"description":"ACS880 single drive or INU","mandatory":true,"dataType":"string","value":"abb.drives.acs880.inu.4@1","firmware":"AINF*","supported":true}}}}
len:693



1st
Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.device","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","properties":{"serialNumberIotPanel":{"value":"Panel-4321"}},"version":15}
len:125

1st sending
TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.Device.Updated","model":"abb.ability.device","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"objectId": "6fbc0f2e-93a9-4368-9b54-9c59a52b4e11",
"model": "abb.ability.device",
"properties": {
	"deviceId":{ "value": "bc26panel003" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-4321" }
},
"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-1234"
				},
			},
			"createIfMissing": true
		}
	]
}

}

len:533
IoTHubClient accepted the message for delivery


Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.configuration","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"type":"abb.drives.iotpanel.config.4@15","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","version":15,"model":"abb.ability.configuration","properties":{"serialNumberIotPanel":{"value":"Panel-4321"},"firmwareVersion":{"mandatory":true,"dataType":"string"},"simIccid":{"mandatory":true,"dataType":"string"},"networkOperator":{"mandatory":true,"dataType":"string"},"driveTypes":{"driveType_1":{"description":"ACS580","mandatory":true,"dataType":"string","value":"abb.drives.acs580.4@1","firmware":"ASCD*","supported":true},"driveType_2":{"description":"ACS880 single drive or INU","mandatory":true,"dataType":"string","value":"abb.drives.acs880.inu.4@1","firmware":"AINF*","supported":true}}}}
len:693




1st sending
TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.Device.Created","model":"abb.ability.device"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel003" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-4321" }
}
}

len:216
IoTHubClient accepted the message for delivery


1st recv
Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.device","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","properties":{"serialNumberIotPanel":{"value":"Panel-4321"}},"version":14}
len:125



TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.Device.Created","model":"abb.ability.device"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel002" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-4321" }
}
}

len:216
IoTHubClient accepted the message for delivery

Device Twin properties received: update=DEVICE_TWIN_UPDATE_COMPLETE payload={
  "desired": {
    "objectId": "6fbc0f2e-93a9-4368-9b54-9c59a52b4e11",
    "properties": {
      "serialNumberIotPanel": {
        "value": "Panel-4321"
      }
    },
    "references": {
      "00000000-0000-0000-0000-000000000000": {
        "reference": "Drive",
        "properties": {
          "serialNumberDrive": {
            "value": "Drive-1234"
          }
        }
      }
    },
    "$version": 8
  },
  "reported": {
    "$version": 1
  }
}, size=481



TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.Device.Updated","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel002" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-4321" }
},
"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-1234"
				},
			},
			"createIfMissing": true
		}
	]
}

}

len:450
IoTHubClient accepted the message for delivery


====================ABBA Lib
when iot panel has registered, recv evt from ABBA server
Message recieved from IoTHub
RX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"Abb.Ability.InformationModel.ObjectModelUpdated","model":"abb.ability.device","objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11"}
Payload
{"objectId":"6fbc0f2e-93a9-4368-9b54-9c59a52b4e11","properties":{"serialNumberIotPanel":{"value":"Panel-4321"}},"version":6}
len:124


================ Device created ==================================================
2018-11-27 device twins callback
{
  "desired": {
    "objectId": "96de39b8-733f-4061-937f-f76152b3c6da",
    "properties": {
      "serialNumberIotPanel": {
        "value": "Panel-12345678901234567890"
      }
    },
    "references": {
      "e686a929-d332-4f66-b9f8-5a5d8207530f": {
        "reference": "Drive",
        "properties": {
          "serialNumberDrive": {
            "value": "Drive-SRMIS-ACS880-12345678901234567890"
          }
        }
      }
    },
    "$version": 2
  },
  "reported": {
    "$version": 1
  }
}

2018 - 10 - 30 12:44 : 48.262 + 01 : 00[Debug] iot - hub - client : sending message with 744 bytes as payload :

ability - messagetype : platformEvent
eventType : Abb.Ability.Device.Created
	objectId : 0000005b - 0000 - 0001 - 0001 - 000000000000

	

{
	"objectId": "0000005b-0000-0001-0001-000000000000",
		"type" : "abb.drives.neta.3@1",
		"model" : "abb.ability.device",
		"properties" : {
		"deviceId": {
			"value": "devasimmste1810300001"
		},
			"hubName" : {
				"value": "AbiGtwyIth5snkEuwDev.azure-devices.net"
			},
				"serialNumber" : {
					"value": "devasimmste1810300001"
				}
	},
		"references": {
		"Drive": [
		{
			"objectId": "0000005b-0000-0001-0001-000000000001",
				"type" : "abb.drives.acs880.inu.3@1",
				"model" : "abb.ability.device",
				"properties" : {
				"serialNumber": {
					"value": "n1devasimmste1810300001"
				}
			},
				"createIfMissing" : true
		}
		]
	}
}

2018 - 10 - 30 12:45 : 53.690 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelCreated message with type platformEvent

ability - messagetype : platformEvent
eventType : Abb.Ability.InformationModel.ObjectModelCreated
	objectId : 0000005b - 0000 - 0001 - 0001 - 000000000000
	model : abb.ability.configuration

{
  "type": "abb.drives.neta.config.3@1",
  "objectId" : "0000005b-0000-0001-0001-000000000000",
  "model" : "abb.ability.configuration",
  "properties" : {
	"serialNumber": {
	  "value": "devasimmste1810300001"
	},
	"driveTypes" : {
	  "driveType_1": {
		"value": "abb.drives.acs580.2@1"
	  },
	  "driveType_2" : {
		"value": "abb.drives.acs880.inu.3@1"
	  },
	  "driveType_3" : {
		"value": "abb.drives.acs880.isu.2@1"
	  },
	  "driveType_4" : {
		"value": ""
	  },
	  "driveType_5" : {
		"value": "abb.drives.acs800.inu.2@1"
	  },
	  "driveType_6" : {
		"value": "abb.drives.acs800.inu.anxr.2@1"
	  },
	  "driveType_7" : {
		"value": "abb.drives.acs800.inu.amxr.2@1"
	  },
	  "driveType_8" : {
		"value": "abb.drives.acs800.inu.ajxc.2@1"
	  },
	  "driveType_9" : {
		"value": "abb.drives.acs800.inu.aqw.2@1"
	  },
	  "driveType_10" : {
		"value": "abb.drives.acs800.isu.il.2@1"
	  },
	  "driveType_11" : {
		"value": "abb.drives.acs800.isu.iwxr.2@1"
	  },
	  "driveType_12" : {
		"value": "abb.drives.acs800.isu.idx.2@1"
	  },
	  "driveType_13" : {
		"value": "abb.drives.acs800.isu.ix.2@1"
	  },
	  "driveType_14" : {
		"value": "abb.drives.acs800.inu.ah.2@1"
	  },
	  "driveType_15" : {
		"value": "abb.drives.dcs800.2@1"
	  },
	  "driveType_16" : {
		"value": "abb.drives.acs1000.msoi.2@1"
	  },
	  "driveType_17" : {
		"value": "abb.drives.acs1000.msoh.2@1"
	  },
	  "driveType_18" : {
		"value": "abb.drives.acs2000.inu.2@1"
	  },
	  "driveType_19" : {
		"value": "abb.drives.acs2000.isu.2@1"
	  },
	  "driveType_20" : {
		"value": "abb.drives.acs5000.2@1"
	  },
	  "driveType_21" : {
		"value": "abb.drives.acs6000.2@1"
	  },
	  "driveType_22" : {
		"value": "abb.drives.acs580mv.2@1"
	  }
	}
  },
  "version": 1
}

2018 - 10 - 30 12:45 : 58.824 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelCreated message with type platformEvent

ability - messagetype : platformEvent
eventType : Abb.Ability.InformationModel.ObjectModelCreated
	  objectId : 0000005b - 0000 - 0001 - 0001 - 000000000001
	  model : abb.ability.configuration

  {
	"type": "abb.drives.acs880.inu.config.3@1",
	"objectId" : "0000005b-0000-0001-0001-000000000001",
	"model" : "abb.ability.configuration",
	"properties" : {
	  "serialNumber": {
		"value": "n1devasimmste1810300001"
	  },
	  "driveType" : {
		"value": "acs880"
	  },
	  "cooling" : {
		"value": "air"
	  },
	  "driveCategory" : {
		"value": "LV"
	  },
	  "monitoring" : {
		"value": "basic"
	  },
	  "registration" : {
		"value": "unchecked"
	  },
	  "schedules" : {
		"basic": {
		  "cycle_1": {
			"value": [
			  "01_01",
			  "01_10",
			  "01_15",
			  "06_11"
			]
		  },
		  "cycle_2": {
			"value": [
			  "01_07",
			  "01_11",
			  "01_31",
			  "05_111",
			  "06_16"
			]
		  },
		  "cycle_3": {
			"value": [
			  "01_35",
			  "01_36",
			  "01_37",
			  "05_41",
			  "05_42",
			  "05_121",
			  "30_11",
			  "30_12",
			  "30_13",
			  "30_14",
			  "30_17",
			  "30_26",
			  "30_27",
			  "96_01"
			]
		  },
		  "cycle_4": {
			"value": [
			  "05_01",
			  "05_02",
			  "45_01",
			  "45_02",
			  "45_03",
			  "99_03",
			  "99_04",
			  "99_06",
			  "99_07",
			  "99_08",
			  "99_09",
			  "99_10",
			  "99_12"
			]
		  }
		},
		"advanced": {
		  "cycle_1": {
			"value": [
			  "01_01",
			  "01_10",
			  "01_15",
			  "05_11",
			  "05_13",
			  "06_11"
			]
		  },
		  "cycle_2": {
			"value": [
			  "01_07",
			  "01_11",
			  "01_31",
			  "05_111",
			  "06_16"
			]
		  },
		  "cycle_3": {
			"value": [
			  "01_35",
			  "01_36",
			  "01_37",
			  "05_41",
			  "05_42",
			  "05_121",
			  "30_11",
			  "30_12",
			  "30_13",
			  "30_14",
			  "30_17",
			  "30_26",
			  "30_27",
			  "96_01"
			]
		  },
		  "cycle_4": {
			"value": [
			  "05_01",
			  "05_02",
			  "45_01",
			  "45_02",
			  "45_03",
			  "99_03",
			  "99_04",
			  "99_06",
			  "99_07",
			  "99_08",
			  "99_09",
			  "99_10",
			  "99_12"
			]
		  }
		},
		"full": {
		  "cycle_1": {
			"value": [
			  "01_01",
			  "01_10",
			  "01_15",
			  "05_11",
			  "05_13",
			  "06_11"
			]
		  },
		  "cycle_2": {
			"value": [
			  "01_02",
			  "01_06",
			  "01_07",
			  "01_11",
			  "01_13",
			  "01_14",
			  "01_24",
			  "01_31",
			  "05_12",
			  "05_14",
			  "05_15",
			  "05_16",
			  "05_17",
			  "05_111",
			  "06_01",
			  "06_02",
			  "06_03",
			  "06_04",
			  "06_05",
			  "06_16",
			  "06_17",
			  "06_18",
			  "06_19",
			  "06_21",
			  "06_25",
			  "10_01",
			  "10_21",
			  "11_01",
			  "12_11",
			  "12_21",
			  "30_01",
			  "30_02"
			]
		  },
		  "cycle_3": {
			"value": [
			  "01_35",
			  "01_36",
			  "01_37",
			  "05_30",
			  "05_31",
			  "05_41",
			  "05_42",
			  "05_121",
			  "30_11",
			  "30_12",
			  "30_13",
			  "30_14",
			  "30_17",
			  "30_26",
			  "30_27",
			  "96_01"
			]
		  },
		  "cycle_4": {
			"value": [
			  "05_01",
			  "05_02",
			  "05_04",
			  "45_01",
			  "45_02",
			  "45_03",
			  "99_03",
			  "99_04",
			  "99_06",
			  "99_07",
			  "99_08",
			  "99_09",
			  "99_10",
			  "99_12"
			]
		  }
		}
	  },
	  "constants": {
		"constant_1": {
		  "value": 300
		},
		"constant_2" : {
		  "value": -300
		}
	  },
	  "algorithms": {
		"algorithm_1": {
		  "value": "InuChargingCounter"
		},
		"algorithm_2" : {
		  "value": "InuTorque"
		},
		"algorithm_3" : {
		  "value": "InuSpeed"
		},
		"algorithm_4" : {
		  "value": "InuPower"
		},
		"algorithm_5" : {
		  "value": "InuNmTorque"
		},
		"algorithm_6" : {
		  "value": "InuMVDrivesStress"
		},
		"algorithm_7" : {
		  "value": "InuEnergyConsumption"
		},
		"algorithm_8" : {
		  "value": "InuAvailability"
		},
		"algorithm_9" : {
		  "value": "InuSignalSummary"
		},
		"algorithm_10" : {
		  "value": "InuScatterTorqueSpeed"
		},
		"algorithm_11" : {
		  "value": "InuScatterPowerSpeed"
		},
		"algorithm_12" : {
		  "value": "InuSaveFaultResetTime"
		},
		"algorithm_13" : {
		  "value": "InuCavitySpeed"
		},
		"algorithm_14" : {
		  "value": "InuCavityTorque"
		},
		"algorithm_15" : {
		  "value": "InuTemperatureIndex"
		}
	  },
	  "trends": {
		"trend_1": {
		  "value": "Power"
		},
		"trend_2" : {
		  "value": "Torque"
		},
		"trend_3" : {
		  "value": "Speed"
		},
		"trend_4" : {
		  "value": "Temperature"
		},
		"trend_5" : {
		  "value": "Current"
		},
		"trend_6" : {
		  "value": "EnergyConsumption"
		},
		"trend_7" : {
		  "value": "Pumpcavitationspeed"
		},
		"trend_8" : {
		  "value": "Pumpcavitationtorque"
		},
		"trend_9" : {
		  "value": "extambtemperature"
		},
		"trend_10" : {
		  "value": "extambpressure"
		},
		"trend_11" : {
		  "value": "extambhumidity"
		}
	  },
	  "scatters": {
		"scatter_1": {
		  "value": "torque,speed"
		},
		"scatter_2" : {
		  "value": "power,speed"
		},
		"scatter_3" : {
		  "value": "igbt,power"
		}
	  },
	  "histograms": {
		"histogram_1": {
		  "value": "Speed"
		},
		"histogram_2" : {
		  "value": "power"
		},
		"histogram_3" : {
		  "value": "temperature"
		},
		"histogram_4" : {
		  "value": "torque"
		}
	  },
	  "serviceCounters": {
		"serviceCounter_1": {
		  "value": "fanagingindicator"
		}
	  },
	  "thermalLimits" : {
		"yellow": {
		  "threshold": {
			"value": 50
		  },
		  "mean" : {
			"value": 32.5
		  },
		  "stdDev" : {
			"value": 4
		  }
		},
		"red": {
		  "threshold": {
			"value": 50
		  },
		  "mean" : {
			"value": 50
		  },
		  "stdDev" : {
			"value": 4
		  }
		}
	  }
	},
	"version": 1
  }

	  2018 - 10 - 30 12:46 : 02.920 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelCreated message with type platformEvent

		ability - messagetype : platformEvent
		eventType : Abb.Ability.InformationModel.ObjectModelCreated
		objectId : 0000005b - 0000 - 0001 - 0001 - 000000000000
		model : abb.ability.device

	{
	  "objectId": "0000005b-0000-0001-0001-000000000000",
	  "properties" : {
		"serialNumber": {
		  "value": "devasimmste1810300001"
		}
	  },
	  "version" : 1
	}

		2018 - 10 - 30 12:46 : 22.362 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelCreated message with type platformEvent

		  ability - messagetype : platformEvent
		  eventType : Abb.Ability.InformationModel.ObjectModelCreated
		  objectId : 0000005b - 0000 - 0001 - 0001 - 000000000001
		  model : abb.ability.device

	  {
		"objectId": "0000005b-0000-0001-0001-000000000001",
		"properties" : {
		  "serialNumber": {
			"value": "n1devasimmste1810300001"
		  }
		},
		"version" : 1
	  }


========================== SYNC MODEL ==============================

2018 - 10 - 30 14:54 : 21.538 + 01 : 00[Information] Got device twin for devasimmste1810300001 :
		{
			"deviceId": null,
				"etag" : null,
				"version" : null,
				"properties" : {
				"desired": {
					"objectId": "0000005b-0000-0001-0001-000000000000",
						"properties" : {
						"serialNumber": {
							"value": "devasimmste1810300001"
						}
					},
						"references" : {
						"0000005b-0000-0001-0001-000000000001": {
							"reference": "Drive",
								"properties" : {
								"serialNumber": {
									"value": "n1devasimmste1810300001"
								}
							}
						}
					},
						"$version": 3
				},
					"reported": {
						"$version": 1
					}
			}
		}

		2018 - 10 - 30 14:54 : 29.624 + 01 : 00[Debug] iot - hub - client : sending message with 125 bytes as payload :

		ability - messagetype : platformEvent
			eventType : Abb.Ability.Device.SyncModel
			model : abb.ability.device
			objectId : 0000005b - 0000 - 0001 - 0001 - 000000000000

		{
			"references": [],
				"contained" : false,
				"recursive" : false,
				"related" : [
					"abb.ability.configuration"
				]
		}

		2018 - 10 - 30 14:54 : 29.845 + 01 : 00[Debug] iot - hub - client : sending message with 125 bytes as payload :

		ability - messagetype : platformEvent
			eventType : Abb.Ability.Device.SyncModel
			model : abb.ability.device
			objectId : 0000005b - 0000 - 0001 - 0001 - 000000000001

		{
			"references": [],
				"contained" : false,
				"recursive" : false,
				"related" : [
					"abb.ability.configuration"
				]
		}

		2018 - 10 - 30 14:54 : 31.221 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelUpdated message with type platformEvent

			ability - messagetype : platformEvent
			eventType : Abb.Ability.InformationModel.ObjectModelUpdated
			objectId : 0000005b - 0000 - 0001 - 0001 - 000000000001
			model : abb.ability.configuration

		{
		  "type": "abb.drives.acs880.inu.config.3@1",
		  "id" : "518302f4-2573-4a62-92d6-5cc8db3d18f5",
		  "objectId" : "0000005b-0000-0001-0001-000000000001",
		  "model" : "abb.ability.configuration",
		  "properties" : {
			"serialNumber": {
			  "value": "n1devasimmste1810300001"
			},
			"driveType" : {
			  "value": "acs880"
			},
			"cooling" : {
			  "value": "air"
			},
			"driveCategory" : {
			  "value": "LV"
			},
			"monitoring" : {
			  "value": "basic"
			},
			"registration" : {
			  "value": "unchecked"
			},
			"schedules" : {
			  "basic": {
				"cycle_1": {
				  "value": [
					"01_01",
					"01_10",
					"01_15",
					"06_11"
				  ]
				},
				"cycle_2": {
				  "value": [
					"01_07",
					"01_11",
					"01_31",
					"05_111",
					"06_16"
				  ]
				},
				"cycle_3": {
				  "value": [
					"01_35",
					"01_36",
					"01_37",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				  ]
				},
				"cycle_4": {
				  "value": [
					"05_01",
					"05_02",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				  ]
				}
			  },
			  "advanced": {
				"cycle_1": {
				  "value": [
					"01_01",
					"01_10",
					"01_15",
					"05_11",
					"05_13",
					"06_11"
				  ]
				},
				"cycle_2": {
				  "value": [
					"01_07",
					"01_11",
					"01_31",
					"05_111",
					"06_16"
				  ]
				},
				"cycle_3": {
				  "value": [
					"01_35",
					"01_36",
					"01_37",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				  ]
				},
				"cycle_4": {
				  "value": [
					"05_01",
					"05_02",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				  ]
				}
			  },
			  "full": {
				"cycle_1": {
				  "value": [
					"01_01",
					"01_10",
					"01_15",
					"05_11",
					"05_13",
					"06_11"
				  ]
				},
				"cycle_2": {
				  "value": [
					"01_02",
					"01_06",
					"01_07",
					"01_11",
					"01_13",
					"01_14",
					"01_24",
					"01_31",
					"05_12",
					"05_14",
					"05_15",
					"05_16",
					"05_17",
					"05_111",
					"06_01",
					"06_02",
					"06_03",
					"06_04",
					"06_05",
					"06_16",
					"06_17",
					"06_18",
					"06_19",
					"06_21",
					"06_25",
					"10_01",
					"10_21",
					"11_01",
					"12_11",
					"12_21",
					"30_01",
					"30_02"
				  ]
				},
				"cycle_3": {
				  "value": [
					"01_35",
					"01_36",
					"01_37",
					"05_30",
					"05_31",
					"05_41",
					"05_42",
					"05_121",
					"30_11",
					"30_12",
					"30_13",
					"30_14",
					"30_17",
					"30_26",
					"30_27",
					"96_01"
				  ]
				},
				"cycle_4": {
				  "value": [
					"05_01",
					"05_02",
					"05_04",
					"45_01",
					"45_02",
					"45_03",
					"99_03",
					"99_04",
					"99_06",
					"99_07",
					"99_08",
					"99_09",
					"99_10",
					"99_12"
				  ]
				}
			  }
			},
			"constants": {
			  "constant_1": {
				"value": 300
			  },
			  "constant_2" : {
				"value": -300
			  }
			},
			"algorithms": {
			  "algorithm_1": {
				"value": "InuChargingCounter"
			  },
			  "algorithm_2" : {
				"value": "InuTorque"
			  },
			  "algorithm_3" : {
				"value": "InuSpeed"
			  },
			  "algorithm_4" : {
				"value": "InuPower"
			  },
			  "algorithm_5" : {
				"value": "InuNmTorque"
			  },
			  "algorithm_6" : {
				"value": "InuMVDrivesStress"
			  },
			  "algorithm_7" : {
				"value": "InuEnergyConsumption"
			  },
			  "algorithm_8" : {
				"value": "InuAvailability"
			  },
			  "algorithm_9" : {
				"value": "InuSignalSummary"
			  },
			  "algorithm_10" : {
				"value": "InuScatterTorqueSpeed"
			  },
			  "algorithm_11" : {
				"value": "InuScatterPowerSpeed"
			  },
			  "algorithm_12" : {
				"value": "InuSaveFaultResetTime"
			  },
			  "algorithm_13" : {
				"value": "InuCavitySpeed"
			  },
			  "algorithm_14" : {
				"value": "InuCavityTorque"
			  },
			  "algorithm_15" : {
				"value": "InuTemperatureIndex"
			  }
			},
			"trends": {
			  "trend_1": {
				"value": "Power"
			  },
			  "trend_2" : {
				"value": "Torque"
			  },
			  "trend_3" : {
				"value": "Speed"
			  },
			  "trend_4" : {
				"value": "Temperature"
			  },
			  "trend_5" : {
				"value": "Current"
			  },
			  "trend_6" : {
				"value": "EnergyConsumption"
			  },
			  "trend_7" : {
				"value": "Pumpcavitationspeed"
			  },
			  "trend_8" : {
				"value": "Pumpcavitationtorque"
			  },
			  "trend_9" : {
				"value": "extambtemperature"
			  },
			  "trend_10" : {
				"value": "extambpressure"
			  },
			  "trend_11" : {
				"value": "extambhumidity"
			  }
			},
			"scatters": {
			  "scatter_1": {
				"value": "torque,speed"
			  },
			  "scatter_2" : {
				"value": "power,speed"
			  },
			  "scatter_3" : {
				"value": "igbt,power"
			  }
			},
			"histograms": {
			  "histogram_1": {
				"value": "Speed"
			  },
			  "histogram_2" : {
				"value": "power"
			  },
			  "histogram_3" : {
				"value": "temperature"
			  },
			  "histogram_4" : {
				"value": "torque"
			  }
			},
			"serviceCounters": {
			  "serviceCounter_1": {
				"value": "fanagingindicator"
			  }
			},
			"thermalLimits" : {
			  "yellow": {
				"threshold": {
				  "value": 50
				},
				"mean" : {
				  "value": 32.5
				},
				"stdDev" : {
				  "value": 4
				}
			  },
			  "red": {
				"threshold": {
				  "value": 50
				},
				"mean" : {
				  "value": 50
				},
				"stdDev" : {
				  "value": 4
				}
			  }
			}
		  },
		  "version": 1
		}


			2018 - 10 - 30 14:54 : 32.335 + 01 : 00[Information] deva neta = devasimmste1810300001 : received Abb.Ability.InformationModel.ObjectModelUpdated message with type platformEvent

			  ability - messagetype : platformEvent
			  eventType : Abb.Ability.InformationModel.ObjectModelUpdated
			  objectId : 0000005b - 0000 - 0001 - 0001 - 000000000000
			  model : abb.ability.configuration

		  {
			"type": "abb.drives.neta.config.3@1",
			"id" : "bc660721-1291-4ab5-a8bc-653582254e1c",
			"objectId" : "0000005b-0000-0001-0001-000000000000",
			"model" : "abb.ability.configuration",
			"properties" : {
			  "serialNumber": {
				"value": "devasimmste1810300001"
			  },
			  "driveTypes" : {
				"driveType_1": {
				  "value": "abb.drives.acs580.2@1"
				},
				"driveType_2" : {
				  "value": "abb.drives.acs880.inu.3@1"
				},
				"driveType_3" : {
				  "value": "abb.drives.acs880.isu.2@1"
				},
				"driveType_4" : {
				  "value": ""
				},
				"driveType_5" : {
				  "value": "abb.drives.acs800.inu.2@1"
				},
				"driveType_6" : {
				  "value": "abb.drives.acs800.inu.anxr.2@1"
				},
				"driveType_7" : {
				  "value": "abb.drives.acs800.inu.amxr.2@1"
				},
				"driveType_8" : {
				  "value": "abb.drives.acs800.inu.ajxc.2@1"
				},
				"driveType_9" : {
				  "value": "abb.drives.acs800.inu.aqw.2@1"
				},
				"driveType_10" : {
				  "value": "abb.drives.acs800.isu.il.2@1"
				},
				"driveType_11" : {
				  "value": "abb.drives.acs800.isu.iwxr.2@1"
				},
				"driveType_12" : {
				  "value": "abb.drives.acs800.isu.idx.2@1"
				},
				"driveType_13" : {
				  "value": "abb.drives.acs800.isu.ix.2@1"
				},
				"driveType_14" : {
				  "value": "abb.drives.acs800.inu.ah.2@1"
				},
				"driveType_15" : {
				  "value": "abb.drives.dcs800.2@1"
				},
				"driveType_16" : {
				  "value": "abb.drives.acs1000.msoi.2@1"
				},
				"driveType_17" : {
				  "value": "abb.drives.acs1000.msoh.2@1"
				},
				"driveType_18" : {
				  "value": "abb.drives.acs2000.inu.2@1"
				},
				"driveType_19" : {
				  "value": "abb.drives.acs2000.isu.2@1"
				},
				"driveType_20" : {
				  "value": "abb.drives.acs5000.2@1"
				},
				"driveType_21" : {
				  "value": "abb.drives.acs6000.2@1"
				},
				"driveType_22" : {
				  "value": "abb.drives.acs580mv.2@1"
				}
			  }
			},
			"version": 1
		  }

		  
=============================================================================================================================		  
1st msg sending
TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.Created"}
Payload
{
"model": "abb.ability.device",
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel000" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-12345678901234567890" }
},"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-SRMIS-ACS880-12345678901234567890"
				},
			},
			"createIfMissing": true
		}
	]
}

}

len:525
IoTHubClient accepted the message for delivery


1st msg recv
Device Twin properties received: update=DEVICE_TWIN_UPDATE_COMPLETE payload={
  "desired": {
    "objectId": "96de39b8-733f-4061-937f-f76152b3c6da",
    "properties": {
      "serialNumberIotPanel": {
        "value": "Panel-12345678901234567890"
      }
    },
    "references": {
      "e686a929-d332-4f66-b9f8-5a5d8207530f": {
        "reference": "Drive",
        "properties": {
          "serialNumberDrive": {
            "value": "Drive-SRMIS-ACS880-12345678901234567890"
          }
        }
      }
    },
    "$version": 2
  },
  "reported": {
    "$version": 1
  }
}
2nd msg sending
TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.Updated","objectId":"96de39b8-733f-4061-937f-f76152b3c6da"}
Payload
{
"model": "abb.ability.device",
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel000" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-12345678901234567890" }
},"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-SRMIS-ACS880-12345678901234567890"
				},
				"objectId" : {
					"value": "e686a929-d332-4f66-b9f8-5a5d8207530f"
				},
			},
			"createIfMissing": true
		}
	]
}

}

len:604
IoTHubClient accepted the message for delivery



1st msg recv

Device Twin properties received: update=DEVICE_TWIN_UPDATE_COMPLETE payload={
  "desired": {
    "objectId": "96de39b8-733f-4061-937f-f76152b3c6da",
    "properties": {
      "serialNumberIotPanel": {
        "value": "Panel-12345678901234567890"
      }
    },
    "references": {
      "e686a929-d332-4f66-b9f8-5a5d8207530f": {
        "reference": "Drive",
        "properties": {
          "serialNumberDrive": {
            "value": "Drive-SRMIS-ACS880-12345678901234567890"
          }
        }
      }
    },
    "$version": 2
  },
  "reported": {
    "$version": 1
  }
}C, size=526



2nd msg sending
"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.Updated","objectId":"96de39b8-733f-4061-937f-f76152b3c6da"}
Payload
{
"model": "abb.ability.device",
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel000" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-12345678901234567890" }
},"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-SRMIS-ACS880-12345678901234567890"
				},
				"objectId" : {
					"value": "e686a929-d332-4f66-b9f8-5a5d8207530f"
				},
			},
			"createIfMissing": true
		}
	]
}

}

3rd sending
Message parameters
{"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.SyncModel","objectId":"96de39b8-733f-4061-937f-f76152b3c6da","model":"abb.ability.device"}
Payload
{"recursive":false,"contained":false,"references":[null],"related":["abb.ability.configuration"]}
len:98











TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.Created"}
Payload
{
"model": "abb.ability.device",
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel000" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-12345678901234567890" }
}
}

len:263
IoTHubClient accepted the message for delivery


1st recv
Device Twin properties received: update=DEVICE_TWIN_UPDATE_COMPLETE payload={
  "desired": {
    "objectId": "96de39b8-733f-4061-937f-f76152b3c6da",
    "properties": {
      "serialNumberIotPanel": {
        "value": "Panel-12345678901234567890"
      }
    },
    "references": {
      "e686a929-d332-4f66-b9f8-5a5d8207530f": {
        "reference": "Drive",
        "properties": {
          "serialNumberDrive": {
            "value": "Drive-SRMIS-ACS880-12345678901234567890"
          }
        }
      }
    },
    "$version": 2
  },
  "reported": {
    "$version": 1
  }
}

2nd sending
TX message
Message parameters
{"ability-messagetype":"platformEvent","eventType":"ABB.Ability.Device.Updated","objectId":"96de39b8-733f-4061-937f-f76152b3c6da"}
Payload
{
"model": "abb.ability.device",
"type": "abb.drives.iotpanel.4@1",
"properties": {
	"deviceId":{ "value": "bc26panel000" },
	"hubName":{ "value": "drives-rcm-dev-we-ih.azure-devices.net" },
	"serialNumberIotPanel": { "value": "Panel-12345678901234567890" }
},
"references": {
		"Drive": [
		{
			"model": "abb.ability.device",
			"type" : "abb.drives.acs880.inu.4@1",
			"properties" : {
				"serialNumberDrive" : {
					"value": "Drive-SRMIS-ACS880-12345678901234567890"
				},
				"objectId" : {
					"value": "e686a929-d332-4f66-b9f8-5a5d8207530f"
				},
			},
			"createIfMissing": true
		}
	]
}

}

len:605
IoTHubClient accepted the message for delivery


TX message
Message parameters
{"ability-messagetype":"timeSeries"}
Payload
[
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_01",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "rpm"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_01",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "rpm"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_01",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "rpm"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_06",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "Hz"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_06",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "Hz"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_06",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "Hz"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_07",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "A"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_07",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "A"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_07",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "A"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_10",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_10",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_10",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_11",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "V"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_11",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "V"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_11",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "V"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_15",
			"timestamp" : "2018-12-07T11:47:33.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_15",
			"timestamp" : "2018-12-07T11:46:43.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_15",
			"timestamp" : "2018-12-07T11:45:51.0Z",
			"value" : 0.000000,
			"unit" : "%"
		},
		{
			"objectId": "f7f3fa40-b6f4-479f-b0c4-77911859818c",
			"model" : "abb.ability.device",
			"variable" : "01_31",
			"timestamp" : "2018-12-07T11:47:07.0Z",
			"value" : 40.000000,
			"unit" : "癈"
		}]

len:3928
IoTHubClient accepted the message for delivery