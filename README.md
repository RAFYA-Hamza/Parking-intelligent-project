# 👋🎲 Smart-Parking-project
Our project consists of a connected car park that manages spaces in a flexible and intelligent way. They offer the user the possibility of keeping his car in a secure place on the one hand, and being able to reserve a remote location for his vehicle on the other hand. All this through a graphical interface to be designed so that the user can access it by computer or telephone.

Our system will allow users to access the car park through the graphical interface or according to one of the three modes of use
following:
- **Subscriber mode:** where the reservation duration for each day along a month is known. The method of payment is either monthly or quarterly.
- **Reservation mode:** where the reservation duration per day is known. The mode of payment is priced per hour, with reduction.
- **Visitor mode:** where the reservation duration is unknown during the day. The method of payment is priced per hour, without reduction.

The diagram below explains the interconnection network between the different devices of the system through a server, which represents the data processing center. Thus the various data issued and sent to the devices pass through the server for the execution of the associated actions.

![Image1](https://github.com/RAFYA-Hamza/Parking-intelligent-project/assets/101463808/c33cad86-7529-48c6-b8be-a2e59bb1f033)


# Main functions :
# 📈 PF1: IHM Device.
**IHM Device** is the device that includes a human-machine interface (HMI) designed to manage the different functionalities of a smart parking management system, with the use of the Node Red programming environment as a data processing center from the various other devices, the latter including the interface are connected to the Mosquitto server.\\

The functionalities offered by this device include the creation and authentication of temporary accounts in the system database, the real-time display of the status of parking spaces, the reservation of parking spaces, as well as the calculation and payment dwell times in the car park.


# 🛠️ PF2: In Control Device.
**In Control Device** comes to automate the input process. We therefore fill this need by reserving in advance with an application that manages and gives the available places. In the event that a vehicle is detected using an ultrasonic sensor at the entrance to the parking lot. The user must enter the valid entry code. If the code is valid, the server sends the validation. Otherwise the barrier remains closed. The number of places available is always displayed at the entrance to the car park.

# 🛠️ PF3: Out Control Device.
**Out Control Device** is the device that used to check if a vehicle is allowed to leave the parking lot. It can be used to check if a vehicle has paid the parking ticket or has a valid parking permit.

This device can be configured in different ways, but it is generally composed of a QR code or RFID badge reader, an LCD screen to display instructions and information, and a barrier mechanism to control access to the car park.

# 📝 PF4: Occupancy Management Device.
**Occupancy Management Device** is a device that can be used in smart parking. This device is responsible for detecting the availability of places in the car park, and communicating the status of the place to the server in order to manage the car park and facilitate the task of finding an available place for customers.

This device is made up of various pieces of equipment, in particular ultrasonic sensors which allow the detection of the status of each space in the intelligent car park, LEDs which display the status of the space (occupied or available), and a Buzzer which is activated when the state of the places changes.

