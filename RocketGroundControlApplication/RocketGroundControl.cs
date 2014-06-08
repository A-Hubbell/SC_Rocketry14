/*
      Title: Ground Rocket Software
    Creator: Harley McPhee
Description: The purpose of this software is to connect to an xbee via serial port, the xbee should recieve information
from an arduino hooked up to an xbee and multiple sensors.
The sensors data this program should expect are
Pressure
Altitude
GPS cordinates
Velocity
Acceleration
The program will also allow the xbee to transmit information to the arduino such as a signal to manually deploy the parachute
and also to manually change the sea level air pressure value
*/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace WindowsFormsApplication1
{
    public partial class RocketGroundControl : Form
    {
        public SerialPort xbeePort; 
        public string[] dataRecievedArray;
        public bool connected = false;
        public double maxAltitude = 0;
        delegate void SetTextCallback();
        public RocketGroundControl()
        {
            InitializeComponent(); 
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();

            Console.WriteLine("The following serial ports were found:");

            // Display each port name to the console. 
            foreach (string port in ports)
            {
                comPortCB.Items.Add(port);
            }

        }
        private void portDataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            /*
             * data templates
             BMP SENSOR  BMP:STATUS:PRESSURE:TEMPERATURE:ALTITUDE
             DOF SENSOR  DOF:STATUS:?:?:?:?
             GPS SENSOR  GPS:STATUS:?:?:?
             */
            try
            {
                string data = xbeePort.ReadLine();

                dataRecievedArray = data.Split(':'); // Split up each piece of data into an array
                if (dataRecievedArray[0] == "BMP" ) // Checks if we recieved BMP sensor info format BMP:ONLINE:PRESSURE:TEMPERATURE:ALTITUDE
                {
                    setTextBMP();
                }
                else if (dataRecievedArray[0] == "GPS" )
                {
                    setTextGPS();

                }
                else if (dataRecievedArray[0] == "DOF" )
                {
                    setTextDOF();
                }
                else if (dataRecievedArray[0] == "RocketStatus")
                {
                    setRocketStatus();
                }
            }
            catch (Exception el)
            {
               // MessageBox.Show(el.InnerException.ToString());
            }
        }
        private void setRocketStatus()
        {
            if (this.accelerometerlbl.InvokeRequired) // Must invoke UI thread to change UI elements since portDataRecieved is on a separate thread
            {
                SetTextCallback d = new SetTextCallback(setRocketStatus);
                this.Invoke(d, new object[] { });
            }
            else
            {
                rocketstatuslbl.Text = dataRecievedArray[1];
            }
        }
        private void setTextDOF()
        {
            if (this.accelerometerlbl.InvokeRequired) // Must invoke UI thread to change UI elements since portDataRecieved is on a separate thread
            {
                SetTextCallback d = new SetTextCallback(setTextDOF);
                this.Invoke(d, new object[] { });
            }
            else
            {
                try
                {
                    if (dataRecievedArray[1] == "OFFLINE\r")
                    {
                        accelerometerlbl.Text = "Offline";
                        accelerometerlbl.ForeColor = Color.Red;
                    }
                    else
                    {
                        accelerometerlbl.Text = "Online";
                        accelerationXlbl.Text = dataRecievedArray[2];
                        accelerationYlbl.Text = dataRecievedArray[3];
                        accelerationZlbl.Text = dataRecievedArray[4];

                        velocityXlbl.Text = dataRecievedArray[5];
                        velocityYlbl.Text = dataRecievedArray[6];
                        velocityZlbl.Text = dataRecievedArray[7];

                        accelerometerlbl.ForeColor = Color.Lime;
                    }
                }
                catch { }
            }
        }
        private void setTextGPS()
        {
           try
            {
            if (this.gpslbl.InvokeRequired) // Must invoke UI thread to change UI elements since portDataRecieved is on a separate thread
            {
                SetTextCallback d = new SetTextCallback(setTextGPS);
                this.Invoke(d, new object[] { });
            }
            else
            {

                    if (dataRecievedArray[1] == "OFFLINE\r")
                    {
                        gpslbl.Text = "Offline";
                        gpslbl.ForeColor = Color.Red;
                    }
                    else
                    {
                        gpslbl.Text = "Online";
                        gpsLatlbl.Text = dataRecievedArray[5];
                        gpsLonglbl.Text = dataRecievedArray[6];
                        gpslbl.ForeColor = Color.Lime;
                    }
                }
                
            }catch { }

        }
        private void setTextBMP()
        {
            try
            {
                if (this.barometriclbl.InvokeRequired) // Must invoke UI thread to change UI elements since portDataRecieved is on a separate thread
                {
                    SetTextCallback d = new SetTextCallback(setTextBMP);
                    this.Invoke(d, new object[] { });
                }
                else
                {

                    if (dataRecievedArray[1] == "OFFLINE\r")
                    {
                        barometriclbl.Text = "Offline";
                        barometriclbl.ForeColor = Color.Red;
                    }
                    else
                    {
               
                        barometriclbl.Text = "Online";
                        airpressurelbl.Text = dataRecievedArray[2];
                        altitudelbl.Text = dataRecievedArray[4];
                            if (Convert.ToDouble(dataRecievedArray[4].Trim()) > maxAltitude)
                            {
                                maxAltitude = Convert.ToDouble(dataRecievedArray[4].Trim());
                                maxAltitudelbl.Text = maxAltitude.ToString();
                            }
                        }
                   
                        barometriclbl.ForeColor = Color.Lime;

                    } 
                }
                catch {}
        }
        private void combutton_Click(object sender, EventArgs e)
        {
            if (!connected) // Check if connection exists
            {
                try
                {
                    string selected = (string)comPortCB.SelectedItem;
                    xbeePort = new SerialPort(selected, 9600);
                    xbeePort.DtrEnable = false;
                    xbeePort.RtsEnable = false;
   
                    xbeePort.Open();
                    connected = true;
                    combutton.Text = "Disconnect";
                    comPortCB.Enabled = false;
                    parachuteCheckBox.Enabled = true;
                    parachuteTextBox.Enabled = true;
                    deployButton.Enabled = true;
                    pressureSend.Enabled = true;
                    pressureTextBox.Enabled = true;
                    xbeePort.DataReceived += new SerialDataReceivedEventHandler(portDataReceived);
                }
                catch
                {
                    MessageBox.Show("Invalid COM xbeePort.");
                }
                
            }
            else
            {
                try
                {
                    xbeePort.DataReceived -= new SerialDataReceivedEventHandler(portDataReceived);
                    xbeePort.Close();
                    combutton.Text = "Connect";
                    connected = false;
                    comPortCB.Enabled = true;
                    parachuteCheckBox.Enabled = false;
                    parachuteTextBox.Enabled = false;
                    deployButton.Enabled = false;
                    pressureSend.Enabled = false;
                    pressureTextBox.Enabled = false;
                }
                catch (Exception el)
                {
                    MessageBox.Show(el.InnerException.ToString());
                }
            }
        }
        private void button2_Click(object sender, EventArgs e)
        {
            xbeePort.Write("pressure:" + pressureTextBox.Text);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (parachuteCheckBox.Checked && parachuteTextBox.Text == "manual")
            {
                xbeePort.Write("LaunchPar");
            }
        }


    }
}
