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
        SerialPort port;
        string[] dataArray;
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
            string data = port.ReadLine();
           
            dataArray = data.Split(':');
            if (dataArray[0] == "BMP" && dataArray.Count() > 4)
            {
                setText();
            }
           
        }
        private void setText()
        {
            if (this.barometriclbl.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(setText);
                this.Invoke(d, new object[] { });
            }
            else
            {
                if (dataArray[1] == "Offline")
                    barometriclbl.Text = "Offline";
                else
                {
                    barometriclbl.Text = "Online";
                    airpressurelbl.Text = dataArray[2];
                    altitudelbl.Text = dataArray[4];
                }
            }
        }
        private void combutton_Click(object sender, EventArgs e)
        {

            port = new SerialPort(comtxt.Text, 9600);
            port.DtrEnable = false;
            port.RtsEnable = false;
            try
            {
                port.Open();
            }
            catch
            {
                MessageBox.Show("Invalid COM port.");
            }
            port.DataReceived += new SerialDataReceivedEventHandler(portDataReceived);
        }
        private void button2_Click(object sender, EventArgs e)
        {
            port.Write("pressure:" + pressuretxt.Text);
        }


    }
}
