using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace WindowsFormsApplication1
{
    public partial class RocketGroundControl : Form
    {
        SerialPort port;
        public RocketGroundControl()
        {
            InitializeComponent();
            
        }

        private void Form1_Load(object sender, EventArgs e)
        {


        }
        private void portDataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            string data = port.ReadExisting();
        }

        private void combutton_Click(object sender, EventArgs e)
        {
            port = new SerialPort(comtxt.Text, 9600);
            port.DtrEnable = false;
            port.RtsEnable = false;
            port.Open();
            port.DataReceived += new SerialDataReceivedEventHandler(portDataReceived);
        }

    }
}
