# Class Diagramm of irrigation system

The following diagrams show the activity diagrams of the irrigation system. The irrigation system is controlled by an irrigation controller. The irrigation controller has a list of irrigation zones. Each irrigation zone has a valve, a software button, a hardware button, and a timer. The irrigation controller reads the buttons, processes the logic, and writes the outputs.

The initial activity is the setup of the irrigation controller. The irrigation controller defines the irrigation zones, the buttons, and the timer.

Setup method:

```plantuml
start
:createValves;
:createHardwareButtons;
:createSoftwareButtons;
:createIrrigationZones;
:addValvesToIrrigationZones;
:addHardwareButtonsToIrrigationZones;
:addSoftwareButtonsToIrrigationZones;
:createIrrigationController;
:addZones;
end
```

In the loop, the irrigation controller reads the buttons, processes the logic, and writes the outputs.

Main loop:

```plantuml
start
:readButtonStates;
:synchronizeButtonsStates;
:processLogic;
:writeOutputs;
end
```

In the read buttons activity, the irrigation controller reads the states of software and hardware buttons. As hardware push-buttons do not use switches but only buttons, it is necessary to recognize the signal edge at this point. This is to be solved with the help of interrupts.

Read buttons method:

```plantuml
start
:readSoftwareButtons;
:readHardwareButtons;
end
```

In the process logic method, the irrigation controller checks if the software or hardware button are pressed. If any button is pressed, in this case is to check the signal edge of both buttons, it must be from low to high, the irrigation controller starts or stops the irrigation zone. Also the irrigation controller starts the timer of this irrigation zone.
If the timer is running, the irrigation controller stops the irrigation zone after the timer is finished. If any button is pressed, the irrigation controller stops the timer and the irrigation zone immediately.

Process logic method:

```plantuml
start
:checkSoftwareButton;
:checkHardwareButton;
:checkTimer;
:checkRunning;
:checkPressed;
:checkTimerRunning;
:checkTimerFinished;
:checkPressed;
end
```
