# Class Diagramm of irrigation system

```plantuml
class Valve{
    +open()
    +close()
    +isOpen() : boolean
    -name: string
    -GPIOChannel: int
}

class HardwareButton{
    +isPressed() : boolean
    +isReleased() : boolean
    -name: string
    -GPIOChannel: int
}

class SoftwareButton{
    +isPressed() : boolean
    +isReleased() : boolean
    -name: string
    -technology: string
    -address: string
}

class IrrigationZone{
    +start()
    +stop()
    +isRunning() : boolean
    -name: string
    -valve: Valve
}

class Timer{
    +start()
    +stop()
    +isRunning() : boolean
    -time: int
}

class IrrigationController{
    +addZone(zone: IrrigationZone, button: SoftwareButton, button: HardwareButton, timer: Timer)
    +removeZone(zone: IrrigationZone)
    +setup()
    +readButtons()
    +processLogic()
    +writeOutputs()
    -zones: List<IrrigationZone>
}
```
