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
        delegate void SetTextCallback();
        public RocketGroundControl()
        {
            InitializeComponent(); 
        }

        private void Form1_Load(object sender, EventArgs e)
        {


        }
        private void portDataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            /*
             * data templates
             BMP sensor  BMP:STATUS:PRESSURE:TEMPERATURE:ALTITUDE

             */
            string data = xbeePort.ReadLine();

            dataRecievedArray = data.Split(':'); // Split up each piece of data into an array
            if (dataRecievedArray[0] == "BMP" && dataRecievedArray.Count() > 4) // Checks if we recieved BMP sensor info format BMP:ONLINE:PRESSURE:TEMPERATURE:ALTITUDE
            {
                setText();
            }
           
        }
        private void setText()
        {
            if (this.barometriclbl.InvokeRequired) // Must invoke UI thread to change UI elements since portDataRecieved is on a separate thread
            {
                SetTextCallback d = new SetTextCallback(setText);
                this.Invoke(d, new object[] { });
            }
            else
            {
                if (dataRecievedArray[1] == "Offline")
                {
                    barometriclbl.Text = "Offline";
                    barometriclbl.ForeColor = Color.Red;
                }
                else
                {
                    barometriclbl.Text = "Online";
                    airpressurelbl.Text = dataRecievedArray[2];
                    altitudelbl.Text = dataRecievedArray[4];
                    barometriclbl.ForeColor = Color.Lime;
                }
            }
        }
        private void combutton_Click(object sender, EventArgs e)
        {
            if (!connected) // Check if connection exists
            {
                xbeePort = new SerialPort(comtxt.Text, 9600);
                xbeePort.DtrEnable = false;
                xbeePort.RtsEnable = false;
                try
                {
                    xbeePort.Open();
                    connected = true;
                    combutton.Text = "Disconnect";
                    comtxt.Enabled = false;
                    parachuteCheckBox.Enabled = true;
                    parachuteTextBox.Enabled = true;
                    deployButton.Enabled = true;
                    pressureSend.Enabled = true;
                    pressureTextBox.Enabled = true;
                }
                catch
                {
                    MessageBox.Show("Invalid COM xbeePort.");
                }
                xbeePort.DataReceived += new SerialDataReceivedEventHandler(portDataReceived);
            }
            else
            {
                try
                {
                    xbeePort.DataReceived -= new SerialDataReceivedEventHandler(portDataReceived);
                    xbeePort.Close();
                    combutton.Text = "Connect";
                    connected = false;
                    comtxt.Enabled = true;
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
