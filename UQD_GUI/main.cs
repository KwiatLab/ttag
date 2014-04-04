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
using System.Runtime.InteropServices;
using System.Threading;
using TimeTag;

namespace TimeTagger
{
    public partial class main : Form
    {
        //Functions from libTTag. For 32 bit, rename libTTag32.dll
        public const string ttlib = "libTTag32.dll";
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tt_getNextFree();
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern UIntPtr tt_create(int mapnumber, UInt64 datapoints);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tt_close(UIntPtr buffer);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tt_setresolution(UIntPtr buffer, double resolution);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tt_setchannels(UIntPtr buffer, int channels);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tt_running(UIntPtr buffer);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tt_addrunner(UIntPtr buffer);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tt_remrunner(UIntPtr buffer);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tt_addarray_offset(UIntPtr buffer, byte[] channels, long[] tags, ulong datapoints, int add, int div);
        [DllImport(ttlib, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tt_validateBuffer(UIntPtr buffer);


        //Buffer details
        ulong bufsize;
        int ttnum;
        int buffernum;

        //The inversion Mask
        int inversionMask = 0;

        //Pointer to the buffer object
        UIntPtr buffer = System.UIntPtr.Zero;

        //The time tagger interface
        TTInterface ttInterface = null;

        public main(int ttnum, ulong bufsize)
        {
            InitializeComponent();
            this.bufsize = bufsize;
            this.ttnum = ttnum;
        }

        private void addrunner_Click(object sender, EventArgs e)
        {
            //flashbtn.Enabled = false;
            if (this.buffer != System.UIntPtr.Zero)
                tt_addrunner(this.buffer);
        }

        private void remrunner_Click(object sender, EventArgs e)
        {
            if (this.buffer != System.UIntPtr.Zero)
                tt_remrunner(this.buffer);
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
                stat.AppendText(">> " + s + "\n");
                stat.ScrollToCaret();
            }
        }



        public bool init()
        {
            //First load the interface
            if (ttInterface == null)
            {
                //Attempt opening the time-tagger libraries
                status("Opening Timetagger Libraries...");
                try
                {
                    ttInterface = new TTInterface();
                    status("OK");
                }
                catch (Exception ex)
                {
                    ttInterface = null;
                    status("Opening ttInterface.dll failed:\n\"" + ex.Message + "\"");
                    flashbtn.Enabled = false;
                    addrunner.Enabled = false;
                    remrunner.Enabled = false;
                    return false;
                }
            }
            if (!ttInterface.IsOpen())
            {
                try
                {
                    status("Connecting to Timetagger...");
                    ttInterface.Open(ttnum);
                    status("FPGA Version:" + ttInterface.GetFpgaVersion() + " Resolution:" + ttInterface.GetResolution() + " Errors:" + ttInterface.ReadErrorFlags());
                    ttInterface.Use10MHz(false);
                    setthreshold(2.0);
                    status("OK");
                }
                catch (Exception ex)
                {
                    status("Error: " + ex.Message);
                    flashbtn.Enabled = false;
                    addrunner.Enabled = false;
                    remrunner.Enabled = false;
                    return false;
                }
            }

            if (buffer == System.UIntPtr.Zero)
            {
                try
                {
                    status("Creating Buffer...");
                    buffernum = tt_getNextFree();
                    buffer = tt_create(buffernum, bufsize);
                    if (buffer == System.UIntPtr.Zero)
                    {
                        status("Error: Buffer creation failed.\nThe size given might be too large.");
                        flashbtn.Enabled = false;
                        addrunner.Enabled = false;
                        remrunner.Enabled = false;
                        return false;
                    }
                    //Set the number of channels and the time tagger's resolution
                    tt_setchannels(buffer, 16);
                    tt_setresolution(buffer, ttInterface.GetResolution());

                    status("Buffer: " + buffernum + " Size: " + bufsize);
                }
                catch (Exception ex)
                {
                    status("Error: " + ex.Message);
                    flashbtn.Enabled = false;
                    addrunner.Enabled = false;
                    remrunner.Enabled = false;
                    return false;
                }
            }
            if (!runnertimer.Enabled)
            {
                runnertimer.Start();
            }
            if (!tagger.IsBusy)
            {
                tagger.RunWorkerAsync();
            }
            ledcontrol.Enabled = true;
            optionsmenu.Enabled = true;
            refon.Enabled = true;
            savemenu.Enabled = true;
            this.Text = "TT" + ttnum + " Buffer " + buffernum;
            status("Ready.");
            return true;
        }
        public void close()
        {
            if (runnertimer.Enabled)
            {
                runnertimer.Stop();
            }
            if (tagger.IsBusy)
            {
                tagger.CancelAsync();
            }
            if (ttInterface != null)
            {
                if (ttInterface.IsOpen())
                {
                    ttInterface.Close();
                }
            }
            if (buffer != System.UIntPtr.Zero)
            {
                tt_close(buffer);
            }
        }

        private void main_Load(object sender, EventArgs e)
        {
            init();
        }



        private void main_FormClosing(object sender, FormClosingEventArgs e)
        {
            close();
        }

        private void runnertimer_Tick(object sender, EventArgs e)
        {
            int r = tt_running(buffer);
            if (r != 0)
            {
                refon.Enabled = false;
                optionsmenu.Enabled = false;
                ledcontrol.Enabled = false;
                savemenu.Enabled = false;
            }
            else
            {
                refon.Enabled = true; optionsmenu.Enabled = true; ledcontrol.Enabled = true;
                savemenu.Enabled = true;
            }
            runners.Text = Convert.ToString(r);
        }

        private void tagger_DoWork(object sender, DoWorkEventArgs e)
        {
            byte[] channels;
            long[] times;
            int num;
            int errs;
            try
            {
                while (true)
                {
                    while (tt_running(buffer) == 0)
                    {
                        Thread.Sleep(300);
                        if (tagger.CancellationPending)
                        {
                            return;
                        }
                    }
                    status("Running...");
                    ttInterface.StartTimetags();
                    while (tt_running(buffer) != 0)
                    {
                        errs = ttInterface.ReadErrorFlags();
                        num = ttInterface.ReadTags(out channels, out times);

                        tt_addarray_offset(buffer, channels, times, Convert.ToUInt64(num), -1, 1);

                        if (errs != 0) status("Time Tagger: " + ttInterface.GetErrorText(errs));
                        if (tagger.CancellationPending)
                        {
                            ttInterface.StopTimetags();
                            return;
                        }

                    }
                    ttInterface.StopTimetags();
                    status("Stopped");
                }
            }
            catch (Exception ex)
            {
                status("# " + ex.Message);
            }
        }

        private void lEDsOFFToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttInterface.SetLedBrightness(0);
            status("LED brightness set to 0%");
        }

        private void lEDs50ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttInterface.SetLedBrightness(50);
            status("LED brightness set to 50%");
        }

        private void lEDsMAXToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ttInterface.SetLedBrightness(100);
            status("LED brightness set to 100%");
        }

        private void flashbtn_Click(object sender, EventArgs e)
        {
            status("Calibrating...");
            flashbtn.Enabled = false;
            ttInterface.Calibrate();
            status("Calibration Finished.");
            flashbtn.Enabled = true;
        }

        private void refon_CheckedChanged(object sender, EventArgs e)
        {
            ttInterface.Use10MHz(refon.Checked);
            if (refon.Checked) status("Using external 10MHz clock.");
            else status("Using internal clock.");
        }
        private void setthreshold(double v)
        {

            for (int i = 1; i <= 16; i++)
            {
                ttInterface.SetInputThreshold(i, v);
            }
            status("Set all channel input threshold at " + v + "V");
        }

        private void tn2v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = false;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(-2.0);
        }

        private void tn15v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = false;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(-1.5);
        }

        private void tn1v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = false;
            tn05v.Enabled = true;
            setthreshold(-1.0);
        }

        private void tn05v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = false;
            setthreshold(-0.5);
        }

        private void t05v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = false;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(0.5);
        }

        private void t1v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = true;
            t1v.Enabled = false;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(1.0);
        }

        private void t15v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = true;
            t15v.Enabled = false;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(1.5);
        }

        private void t2v_Click(object sender, EventArgs e)
        {
            t2v.Enabled = false;
            t15v.Enabled = true;
            t1v.Enabled = true;
            t05v.Enabled = true;
            tn2v.Enabled = true;
            tn15v.Enabled = true;
            tn1v.Enabled = true;
            tn05v.Enabled = true;
            setthreshold(2.0);
        }

        private void dumpToBinaryFileToolStripMenuItem_Click(object sender, EventArgs e)
        {

            status("Binary file dump...");
            Form f = new savedata(1,this.buffer);
            f.ShowDialog(this);
            status("Finished file write.");
        }

        private void saveRawTagsToCSVToolStripMenuItem_Click(object sender, EventArgs e)
        {
            status("Saving raw data to CSV...");
            Form f = new savedata(2, this.buffer);
            f.ShowDialog(this);
            status("Finished file write.");
        }

        private void saveToCSVToolStripMenuItem_Click(object sender, EventArgs e)
        {
            status("Saving data to CSV...");
            Form f = new savedata(3, this.buffer);
            f.ShowDialog(this);
            status("Finished file write.");
        }

        private void validateBufferToolStripMenuItem_Click(object sender, EventArgs e)
        {
            status("Running Buffer Validation");
            int i = tt_validateBuffer(buffer);
            if (i == 0)
            {
                status("BUFFER VALIDATION FAILED! ANALYSIS FUNCTIONS MAY CRASH/GIVE BAD RESULTS");
            }
            else {
                status("Buffer validation successful. Everything is fine.");
            }
        }

        private void risingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            status("Set all channels to Rising Edge.");
            this.inversionMask = 0;
            ttInterface.SetInversionMask(this.inversionMask);
        }

        private void fallingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            status("Set all channels to falling Edge");

            this.inversionMask = 2147483647;
            ttInterface.SetInversionMask(this.inversionMask);
        }

        private void sETCHANNELToolStripMenuItem_Click(object sender, EventArgs e)
        {
            double threshold = 0;
            int channel = 0;
            if (double.TryParse(perchannel_threshold.Text, out threshold) && int.TryParse(perchannel_cnumber.Text, out channel) && (perchannel_edge.Text == "+" || perchannel_edge.Text == "-"))
            {
                //Set the inversion Mask
                if (channel > 0 && channel < 17 && threshold >= -2.0 && threshold <= 2.0)
                {
                    if ((this.inversionMask & (1 << (channel - 1))) != 0)
                    {
                        if (perchannel_edge.Text == "+")
                        {
                            this.inversionMask ^= 1 << (channel - 1);
                        }
                    }
                    else
                    {
                        if (perchannel_edge.Text == "-")
                        {
                            this.inversionMask ^= 1 << (channel - 1);
                        }
                    }
                    ttInterface.SetInversionMask(this.inversionMask);
                    ttInterface.SetInputThreshold(channel, threshold);
                    status("Set Channel " + channel + " to " + threshold + "V (" + perchannel_edge.Text + ")");
                }
                else
                {
                    status("Error: One of the values was out of bounds!");
                }


            }
            else
            {
                status("Error: One of the text boxes had invalid format.");
            }
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            double threshold = 0;
            if (double.TryParse(threshold_textbox.Text, out threshold))
            {
                if (threshold >= -2.0 && threshold <= 2.0)
                {
                    t2v.Enabled = true;
                    t15v.Enabled = true;
                    t1v.Enabled = true;
                    t05v.Enabled = true;
                    tn2v.Enabled = true;
                    tn15v.Enabled = true;
                    tn1v.Enabled = true;
                    tn05v.Enabled = true;
                    setthreshold(threshold);
                }
                else {
                    status("Error: Threshold wasn't within (-2.0,2.0)");
                }
            }
            else {
                
                 status("Error: Text box has invalid format.");
                
            }
        }
    }
}
