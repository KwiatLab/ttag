/* The UIUC/NCSA license:

Copyright (c) 2014 Kwiat Quantum Information Group
All rights reserved.

Developed by:	Kwiat Quantum Information Group
				University of Illinois, Urbana-Champaign (UIUC)
				http://research.physics.illinois.edu/QI/Photonics/

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal with the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers
in the documentation and/or other materials provided with the distribution.

Neither the names of Kwiat Quantum Information Group, UIUC, nor the names of its contributors may be used to endorse
or promote products derived from this Software without specific prior written permission.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace TimeTagger
{
    public partial class savedata : Form
    {
        [DllImport("libTTag32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong tt_readarray(UIntPtr buffer, ulong startpoint, byte[] channel, ulong[] timetag, ulong datapoints);
        [DllImport("libTTag32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern double tt_resolution(UIntPtr buffer);
        [DllImport("libTTag32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong tt_maxdata(UIntPtr buffer);
        [DllImport("libTTag32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong tt_datapoints(UIntPtr buffer);


        int type = 1;
        UIntPtr buffer;
        bool iswriting = false;
        ulong pointnumber = 0;
        public savedata(int type,UIntPtr buffer)
        {
            InitializeComponent();
            this.type = type;
            this.buffer = buffer;
            switch (type) { 
                case 1:
                    writetype.Text = "Raw Binary Dump";
                    filechooser.DefaultExt = "bin";
                    break;
                case 2:
                    writetype.Text = "Raw data CSV";
                    break;
                case 3:
                    writetype.Text = "CSV";
                    break;

            }
        }

        private void mostrecent_CheckedChanged(object sender, EventArgs e)
        {
            tagnumber.Enabled = mostrecent.Checked;
        }

        private void btn_Click(object sender, EventArgs e)
        {
            if (iswriting)
            {
                //Cancel writing
                filewriter.CancelAsync();
                btn.Enabled = false;
                stat.Text = "Canceling...";
            }
            else { 
                //Start writing
                filechooser.ShowDialog();
                if (mostrecent.Checked)
                {
                    pointnumber = Convert.ToUInt64(tagnumber.Value);
                }
                else {
                    pointnumber = 0;
                }
                filewriter.RunWorkerAsync(filechooser.FileName);
                stat.Text = "Writing file...";
                btn.Text = "Cancel";
                iswriting = true;
            }
        }

        private void filewriter_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            progress.Value = e.ProgressPercentage;
        }
        private delegate void status_delegate(string s);
        public void status(string s)
        {
            if (stat.InvokeRequired)
            {
                stat.BeginInvoke(new status_delegate(status), new object[] { s });
            }
            else
            {
                stat.Text = s;
            }
        }
        private void filewriter_DoWork(object sender, DoWorkEventArgs e)
        {
            byte[] ch = new byte[1000];
            ulong[] tags = new ulong[1000];
            StreamWriter f = new StreamWriter(e.Argument.ToString());
            double resolution = tt_resolution(buffer);
            ulong dataindex = tt_datapoints(buffer);
            ulong bufsize = tt_maxdata(buffer);
            ulong startindex = (dataindex > bufsize ? dataindex-bufsize : 0);
            if (pointnumber > 0 && dataindex - startindex > pointnumber) {
                startindex = dataindex - pointnumber;
            }
            double percentageDone = 100.0/(dataindex-startindex);
            ulong pointswritten = 0;
            //Now startindex is the index to begin at, and dataindex is the end index
            //Divide the workload into chunks of 1000:
            while (dataindex - startindex > 1000) {
                ulong num = tt_readarray(buffer, startindex, ch, tags, 1000);
                if (num != 1000) { status("Getting weird results - file might be corrupt!"); }
                for (int i = 0; i < 1000; i++)
                {
                    switch (type) { 
                        case 1:
                            status("ERROR: PC LOAD LETTER");
                            break;
                        case 2:
                            f.WriteLine(ch[i].ToString() + "\t" + tags[i].ToString());
                            break;
                        case 3:
                            f.WriteLine(ch[i].ToString() + "\t" + (tags[i]*resolution).ToString());
                            break;
                    }
                }
                pointswritten += 1000;
                startindex += 1000;
                filewriter.ReportProgress(Convert.ToInt32(pointswritten * percentageDone));
                if (filewriter.CancellationPending) {
                    f.Close();
                    return;
                }
            }
            //write the final data points
            tt_readarray(buffer, startindex, ch, tags, dataindex - startindex);
            for (int i = 0; i < Convert.ToInt32(dataindex-startindex); i++)
            {
                switch (type)
                {
                    case 1:
                        status("ERROR: PC LOAD LETTER");
                        break;
                    case 2:
                        f.WriteLine(ch[i].ToString() + "\t" + tags[i].ToString());
                        break;
                    case 3:
                        f.WriteLine(ch[i].ToString() + "\t" + (tags[i] * resolution).ToString());
                        break;
                }
            }
            filewriter.ReportProgress(100);
            f.Close();
        }

        private void filewriter_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.Close();
        }
    }
}
